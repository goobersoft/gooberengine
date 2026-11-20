/* The built-in sound editor (F4).
 *
 *   +------+----------------------------------------------+
 *   | sfx  | speed / length / loop          [play]        |
 *   | list | pitch of each of the 32 steps (draw it)      |
 *   | 0-63 | volume of each step                          |
 *   |      | wave of each step, effect of each step       |
 *   +------+----------------+-----------------------------+
 *   | wave | the wave being | the 16 waves + noise,        |
 *   |      | drawn          | the effect pens              |
 *
 * Typing works like a little keyboard: the bottom letter row Z S X D C V
 * G B H N J M plays and enters the notes of one octave (Q W E R T Y U are
 * the white notes of the next); 0-7 set the step's volume. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_X 2
#define LIST_Y 13
static int EX = 66;           /* steps area (moves left beside the file tree) */
static bool narrow;           /* beside the file tree: no sfx list (PgUp / PgDn), no wave labels */
#define SW 10                 /* pixels per step */
#define HEAD_Y 12
#define PITCH_Y 24            /* pitch graph: 64 px, pitch 0 at the bottom */
#define PITCH_H 64
#define VOL_Y 92              /* volume graph: 8 levels of 4 px */
#define VOL_H 32
#define WAVE_ROW_Y 127
#define FX_ROW_Y 138
#define LOOP_Y 150
static int WE_X = 66;         /* wave editor: 32 samples x 4 px, 16 levels x 4 px */
#define WE_Y 160
static int THUMB_X = 206;

static void layout(void) {
    narrow = UI_W < 66 + SFX_STEPS * 10;
    EX = narrow ? 8 : 66;
    WE_X = narrow ? 2 : 66;
    THUMB_X = narrow ? 136 : 206;
}
#define THUMB_Y 160
#define FXPEN_Y 216
#define LIST_ROW 9
#define MAX_UNDO 64
#define PREVIEW_VOICE (NVOICES - 1)

typedef struct { uint8_t waves[NWAVES][WAVE_LEN]; Sfx sfx[NSFX]; } Bank;

static const char *const fx_names[NFX] = {"none", "slide", "vibrato", "drop", "fade in", "fade out", "arpeggio"};

static struct {
    bool ready;
    int cur;                  /* sfx being edited */
    int cursor;               /* step */
    int octave;               /* 0-4: the Z row plays octave+2 */
    int pen_wave, pen_fx, pen_vol;
    int drag;                 /* 0 none, 1 pitch, 2 volume, 3 wave row, 4 fx row, 5 wave editor */
    int drag_btn, last_step, last_val;
    Bank undo[MAX_UNDO], redo[MAX_UNDO];
    int nundo, nredo;
    Sfx clip;
    bool has_clip;
    char status[128];
    int status_color;
    long status_until;
} S;

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(S.status, sizeof S.status, fmt, ap);
    va_end(ap);
    S.status_color = color;
    S.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[sound] %s\n", S.status);
}

static void init(void) {
    if (S.ready) return;
    S.ready = true;
    S.octave = 2;
    S.pen_vol = 5;
}

static Sfx *cur(void) { return &cur_snd->sfx[S.cur]; }
static SfxStep *step(int k) { return &cur()->steps[k]; }

/* ---------------------------------------------------------------- undo */

static void push(Bank *stack, int *n) {
    if (*n == MAX_UNDO) { memmove(stack, stack + 1, sizeof(Bank) * (MAX_UNDO - 1)); (*n)--; }
    memcpy(stack[*n].waves, cur_snd->waves, sizeof cur_snd->waves);
    memcpy(stack[*n].sfx, cur_snd->sfx, sizeof cur_snd->sfx);
    (*n)++;
}

static void before_change(void) {
    push(S.undo, &S.nundo);
    S.nredo = 0;
    doc_touch(RES_SFX);
}

static void undo_redo(bool redo) {
    Bank *from = redo ? S.redo : S.undo, *to = redo ? S.undo : S.redo;
    int *nfrom = redo ? &S.nredo : &S.nundo, *nto = redo ? &S.nundo : &S.nredo;
    if (!*nfrom) { status(COL_GREY, "nothing to %s", redo ? "redo" : "undo"); return; }
    push(to, nto);
    --*nfrom;
    audio_lock();
    memcpy(cur_snd->waves, from[*nfrom].waves, sizeof cur_snd->waves);
    memcpy(cur_snd->sfx, from[*nfrom].sfx, sizeof cur_snd->sfx);
    audio_unlock();
    doc_touch(RES_SFX);
}

/* ---------------------------------------------------------------- playing */

static bool playing_cur(void) { return synth_voice_sfx(0) == S.cur; }

static void play_toggle(void) {
    audio_lock();
    if (playing_cur()) synth_stop(0);
    else synth_sfx(S.cur, 0);
    audio_unlock();
}

static void preview(int k) {
    SfxStep *st = step(k);
    if (!st->vol) return;
    audio_lock();
    synth_note(PREVIEW_VOICE, st->pitch, st->wave, st->vol, TICK_HZ / 6);
    audio_unlock();
}

static void stop_all(void) {
    audio_lock();
    synth_stop(-1);
    audio_unlock();
}

/* ---------------------------------------------------------------- keys */

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void enter_note(int semitone) {
    before_change();
    SfxStep *st = step(S.cursor);
    st->pitch = (uint8_t)clampi(S.octave * 12 + semitone, 0, NPITCH - 1);
    st->wave = (uint8_t)S.pen_wave;
    if (!st->vol) st->vol = (uint8_t)S.pen_vol;
    preview(S.cursor);
    if (S.cursor < SFX_STEPS - 1) S.cursor++;
    if (S.cursor >= cur()->length && cur()->length < SFX_STEPS) cur()->length = (uint8_t)(S.cursor + 1);
}

static void key(const KeyEvent *ev) {
    int sc = ev->sc;
    bool ctrl = ev->mod & MOD_CTRL, shift = ev->mod & MOD_SHIFT;
    Sfx *x = cur();
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: {
                char err[256];
                Doc *d = doc_active(RES_SFX);
                if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved sfx/%s.txt", d->name);
                else status(COL_RED, "%s", err);
                return;
            }
            case SDL_SCANCODE_R: stop_all(); ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: undo_redo(shift); return;
            case SDL_SCANCODE_Y: undo_redo(true); return;
            case SDL_SCANCODE_C: S.clip = *x; S.has_clip = true; status(COL_GREY, "copied sfx %d", S.cur); return;
            case SDL_SCANCODE_V:
                if (!S.has_clip) return;
                before_change();
                *x = S.clip;
                return;
            case SDL_SCANCODE_L: before_change(); x->length = (uint8_t)(S.cursor + 1); return;
            case SDL_SCANCODE_B: before_change(); x->loop_start = (uint8_t)S.cursor; if (x->loop_end <= x->loop_start) x->loop_end = 0; return;
            case SDL_SCANCODE_E:
                before_change();
                x->loop_end = x->loop_end == S.cursor + 1 ? 0 : (uint8_t)(S.cursor + 1);   /* again: no loop */
                if (x->loop_end && x->loop_start >= x->loop_end) x->loop_start = 0;
                return;
            default: return;
        }
    }
    /* the note keys */
    static const struct { int sc, semi; } notes[] = {
        {SDL_SCANCODE_Z, 0}, {SDL_SCANCODE_S, 1}, {SDL_SCANCODE_X, 2}, {SDL_SCANCODE_D, 3}, {SDL_SCANCODE_C, 4},
        {SDL_SCANCODE_V, 5}, {SDL_SCANCODE_G, 6}, {SDL_SCANCODE_B, 7}, {SDL_SCANCODE_H, 8}, {SDL_SCANCODE_N, 9},
        {SDL_SCANCODE_J, 10}, {SDL_SCANCODE_M, 11}, {SDL_SCANCODE_Q, 12}, {SDL_SCANCODE_W, 14}, {SDL_SCANCODE_E, 16},
        {SDL_SCANCODE_R, 17}, {SDL_SCANCODE_T, 19}, {SDL_SCANCODE_Y, 21}, {SDL_SCANCODE_U, 23},
    };
    for (size_t i = 0; i < sizeof notes / sizeof notes[0]; i++)
        if (sc == notes[i].sc) { enter_note(notes[i].semi); return; }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_7) {
        before_change();
        step(S.cursor)->vol = (uint8_t)(sc - SDL_SCANCODE_1 + 1);
        S.pen_vol = step(S.cursor)->vol;
        preview(S.cursor);
        return;
    }
    SfxStep *st = step(S.cursor);
    switch (sc) {
        case SDL_SCANCODE_0: before_change(); st->vol = 0; return;
        case SDL_SCANCODE_ESCAPE:
            if (synth_voice_sfx(0) != -1) { stop_all(); return; }
            ui_switch(MODE_SHELL);
            return;
        case SDL_SCANCODE_SPACE: play_toggle(); return;
        case SDL_SCANCODE_LEFT: S.cursor = clampi(S.cursor - (shift ? 8 : 1), 0, SFX_STEPS - 1); return;
        case SDL_SCANCODE_RIGHT: S.cursor = clampi(S.cursor + (shift ? 8 : 1), 0, SFX_STEPS - 1); return;
        case SDL_SCANCODE_HOME: S.cursor = 0; return;
        case SDL_SCANCODE_END: S.cursor = x->length - 1; return;
        case SDL_SCANCODE_UP: case SDL_SCANCODE_DOWN: {
            int d = (sc == SDL_SCANCODE_UP ? 1 : -1) * (shift ? 12 : 1);
            before_change();
            st->pitch = (uint8_t)clampi(st->pitch + d, 0, NPITCH - 1);
            preview(S.cursor);
            return;
        }
        case SDL_SCANCODE_DELETE: before_change(); st->vol = 0; if (S.cursor < SFX_STEPS - 1) S.cursor++; return;
        case SDL_SCANCODE_BACKSPACE:
            if (S.cursor > 0) S.cursor--;
            before_change();
            step(S.cursor)->vol = 0;
            return;
        case SDL_SCANCODE_TAB:
            before_change();
            st->fx = (uint8_t)((st->fx + (shift ? NFX - 1 : 1)) % NFX);
            S.pen_fx = st->fx;
            return;
        case SDL_SCANCODE_COMMA: case SDL_SCANCODE_PERIOD:
            before_change();
            S.pen_wave = (S.pen_wave + (sc == SDL_SCANCODE_COMMA ? WAVE_NOISE : 1)) % (WAVE_NOISE + 1);
            st->wave = (uint8_t)S.pen_wave;
            preview(S.cursor);
            return;
        case SDL_SCANCODE_LEFTBRACKET: S.octave = clampi(S.octave - 1, 0, 4); return;
        case SDL_SCANCODE_RIGHTBRACKET: S.octave = clampi(S.octave + 1, 0, 4); return;
        case SDL_SCANCODE_MINUS: before_change(); x->speed = (uint8_t)clampi(x->speed - 1, 1, 64); return;
        case SDL_SCANCODE_EQUALS: before_change(); x->speed = (uint8_t)clampi(x->speed + 1, 1, 64); return;
        case SDL_SCANCODE_PAGEUP: S.cur = (S.cur + NSFX - 1) % NSFX; return;
        case SDL_SCANCODE_PAGEDOWN: S.cur = (S.cur + 1) % NSFX; return;
        default: return;
    }
}

/* ---------------------------------------------------------------- mouse */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

static int mouse_step(void) { return clampi((in.mouse_x - EX) / SW, 0, SFX_STEPS - 1); }

/* sets the value at one step for the area being dragged */
static void apply(int area, int k, int val, int btn) {
    SfxStep *st = step(k);
    switch (area) {
        case 1:
            if (btn == 2) { st->vol = 0; break; }
            st->pitch = (uint8_t)clampi(val, 0, NPITCH - 1);
            if (!st->vol) { st->vol = (uint8_t)S.pen_vol; st->wave = (uint8_t)S.pen_wave; }
            break;
        case 2: st->vol = (uint8_t)clampi(btn == 2 ? 0 : val, 0, 7); break;
        case 3: if (btn == 2) S.pen_wave = st->wave; else st->wave = (uint8_t)S.pen_wave; break;
        case 4: if (btn == 2) S.pen_fx = st->fx; else st->fx = (uint8_t)S.pen_fx; break;
        case 5: cur_snd->waves[S.pen_wave][k] = (uint8_t)clampi(val, 0, 15); break;
    }
}

static int area_value(int area) {
    if (area == 1) return PITCH_Y + PITCH_H - 1 - in.mouse_y;
    if (area == 2) return (VOL_Y + VOL_H - 1 - in.mouse_y) / 4 + 1;
    if (area == 5) return 15 - (in.mouse_y - WE_Y) / 4;
    return 0;
}

static int area_step(int area) {
    return area == 5 ? clampi((in.mouse_x - WE_X) / 4, 0, WAVE_LEN - 1) : mouse_step();
}

static void drag_to(void) {
    int k = area_step(S.drag), v = area_value(S.drag);
    int k0 = S.last_step, v0 = S.last_val;
    int n = abs(k - k0);
    for (int i = 0; i <= n; i++) {          /* fill the steps between, so fast strokes leave no gaps */
        int kk = n ? k0 + (k - k0) * i / n : k;
        int vv = n ? v0 + (v - v0) * i / n : v;
        apply(S.drag, kk, vv, S.drag_btn);
    }
    if (S.drag == 1 && k != S.last_step && S.drag_btn == 1) preview(k);
    S.last_step = k;
    S.last_val = v;
}

static void mouse(void) {
    if (S.drag) {
        if (in.mouse[S.drag_btn] && !in.mouse_hit[S.drag_btn]) { drag_to(); return; }
        S.drag = 0;
    }
    int btn = in.mouse_hit[1] ? 1 : in.mouse_hit[2] ? 2 : 0;
    if (!btn) return;
    /* sfx list */
    if (!narrow && inside(LIST_X, LIST_Y, 56, 16 * LIST_ROW)) {
        int c = (in.mouse_x - LIST_X) / 14, r = (in.mouse_y - LIST_Y) / LIST_ROW;
        if (c < 4 && r < 16) S.cur = r * 4 + c;
        return;
    }
    /* header buttons: speed - +, length - +, play */
    const struct { int x, w, what; } head[] = {{EX + 36, 8, 1}, {EX + 62, 8, 2}, {EX + 120, 8, 3}, {EX + 146, 8, 4}, {EX + 286, 34, 5}};
    for (size_t i = 0; i < sizeof head / sizeof head[0]; i++)
        if (inside(head[i].x, HEAD_Y, head[i].w, 10)) {
            Sfx *x = cur();
            if (head[i].what == 5) { play_toggle(); return; }
            before_change();
            if (head[i].what == 1) x->speed = (uint8_t)clampi(x->speed - 1, 1, 64);
            if (head[i].what == 2) x->speed = (uint8_t)clampi(x->speed + 1, 1, 64);
            if (head[i].what == 3) x->length = (uint8_t)clampi(x->length - 1, 1, SFX_STEPS);
            if (head[i].what == 4) x->length = (uint8_t)clampi(x->length + 1, 1, SFX_STEPS);
            return;
        }
    /* the step rows and the wave editor */
    int area = 0;
    if (inside(EX, PITCH_Y, SFX_STEPS * SW, PITCH_H)) area = 1;
    else if (inside(EX, VOL_Y, SFX_STEPS * SW, VOL_H)) area = 2;
    else if (inside(EX, WAVE_ROW_Y, SFX_STEPS * SW, 10)) area = 3;
    else if (inside(EX, FX_ROW_Y, SFX_STEPS * SW, 10)) area = 4;
    else if (inside(WE_X, WE_Y, WAVE_LEN * 4, 64) && S.pen_wave < NWAVES) area = 5;
    if (area) {
        if (area != 5) S.cursor = mouse_step();
        if (!(area >= 3 && btn == 2)) before_change();     /* right-click on wave/fx rows only picks */
        S.drag = area;
        S.drag_btn = btn;
        S.last_step = area_step(area);
        S.last_val = area_value(area);
        apply(area, S.last_step, S.last_val, btn);
        if (area == 1 && btn == 1) preview(S.last_step);
        return;
    }
    /* wave thumbnails and noise, effect pens */
    for (int w = 0; w <= WAVE_NOISE; w++) {
        int tx = THUMB_X + (w % 8) * 24, ty = THUMB_Y + (w / 8) * 18;
        if (w == WAVE_NOISE) { tx = THUMB_X; ty = THUMB_Y + 36; }
        if (inside(tx, ty, w == WAVE_NOISE ? 46 : 23, 17)) { S.pen_wave = w; return; }
    }
    for (int f = 0; f < NFX; f++)
        if (inside(THUMB_X + f * 27, FXPEN_Y, 26, 11)) { S.pen_fx = f; return; }
}

/* ---------------------------------------------------------------- drawing */

static void thumb(int w, int x, int y, bool on) {
    draw_rect(x, y, 23, 17, on ? RGB6(1, 2, 3) : RGB6(0, 0, 1), true);
    for (int i = 0; i < WAVE_LEN; i += 2) {
        int v = cur_snd->waves[w][i];
        draw_pset(x + 3 + i / 2 + i / 4, y + 15 - v, on ? COL_BLACK : COL_YELLOW);
    }
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    Sfx *x = cur();
    int play_step = playing_cur() ? synth_voice_step(0) : -1;

    /* sfx list */
    for (int n = 0; n < NSFX && !narrow; n++) {
        int lx = LIST_X + (n % 4) * 14, ly = LIST_Y + (n / 4) * LIST_ROW;
        bool used = false;
        for (int k = 0; k < SFX_STEPS && !used; k++) used = cur_snd->sfx[n].steps[k].vol > 0;
        if (n == S.cur) draw_rect(lx - 1, ly, 14, LIST_ROW + 1, RGB6(1, 2, 3), true);
        char num[4];
        snprintf(num, sizeof num, "%02d", n);
        int c = n == S.cur ? COL_BLACK : synth_voice_sfx(0) == n ? COL_YELLOW : used ? COL_WHITE : RGB6(1, 1, 2);
        draw_text(num, lx, ly, c);
    }

    /* header */
    char buf[96];
    snprintf(buf, sizeof buf, "speed - %-2d +  len - %-2d +   loop %d-%d", x->speed, x->length, x->loop_start, x->loop_end);
    draw_text(buf, EX, HEAD_Y, RGB6(2, 3, 3));
    snprintf(buf, sizeof buf, "oct %d", S.octave + 2);
    draw_text(buf, EX + 248, HEAD_Y, RGB6(2, 2, 3));
    draw_rect(EX + 286, HEAD_Y, 34, 10, playing_cur() ? COL_YELLOW : RGB6(1, 2, 3), true);
    draw_text(playing_cur() ? "stop" : "play", EX + 291, HEAD_Y, COL_BLACK);

    /* backgrounds: cursor column, steps past the end */
    draw_rect(EX, PITCH_Y, SFX_STEPS * SW, PITCH_H, RGB6(0, 0, 1), true);
    draw_rect(EX, VOL_Y, SFX_STEPS * SW, VOL_H, RGB6(0, 0, 1), true);
    for (int oct = 12; oct < NPITCH; oct += 12) draw_line(EX, PITCH_Y + PITCH_H - 1 - oct, EX + SFX_STEPS * SW - 1, PITCH_Y + PITCH_H - 1 - oct, RGB6(0, 1, 1));
    draw_rect(EX + S.cursor * SW, PITCH_Y, SW, FX_ROW_Y + 10 - PITCH_Y, RGB6(0, 1, 2), true);

    for (int k = 0; k < SFX_STEPS; k++) {
        const SfxStep *st = &x->steps[k];
        int sx = EX + k * SW;
        bool live = k < x->length;
        int py = PITCH_Y + PITCH_H - 1 - st->pitch;
        if (st->vol) {
            int c = !live ? RGB6(1, 1, 1) : k == play_step ? COL_YELLOW : RGB6(1 + st->vol / 3, 2 + st->vol / 4, 3);
            draw_rect(sx + 1, py, SW - 2, PITCH_Y + PITCH_H - py, c, true);
            draw_rect(sx + 1, py, SW - 2, 2, live ? COL_WHITE : COL_GREY, true);
            int vh = st->vol * 4;
            draw_rect(sx + 1, VOL_Y + VOL_H - vh, SW - 2, vh, live ? RGB6(3, 1, 2) : RGB6(1, 1, 1), true);
        } else draw_pset(sx + SW / 2, py, RGB6(1, 1, 2));
        char wc = st->wave >= NWAVES ? 'n' : "0123456789abcdef"[st->wave];
        draw_char(wc, sx + 2, WAVE_ROW_Y, live ? COL_CYAN : RGB6(0, 1, 1));
        draw_char(fx_chars[st->fx % NFX], sx + 2, FX_ROW_Y, live ? COL_ORANGE : RGB6(1, 1, 0));
    }
    if (x->length < SFX_STEPS) draw_line(EX + x->length * SW, PITCH_Y, EX + x->length * SW, FX_ROW_Y + 9, COL_RED);
    if (x->loop_end > x->loop_start) {
        int lx0 = EX + x->loop_start * SW, lx1 = EX + x->loop_end * SW - 1;
        draw_line(lx0, LOOP_Y, lx1, LOOP_Y, COL_GREEN);
        draw_line(lx0, LOOP_Y - 3, lx0, LOOP_Y, COL_GREEN);
        draw_line(lx1, LOOP_Y - 3, lx1, LOOP_Y, COL_GREEN);
    }
    draw_text("w", EX - 8, WAVE_ROW_Y, RGB6(1, 2, 2));
    draw_text("e", EX - 8, FX_ROW_Y, RGB6(2, 2, 1));

    /* wave editor */
    if (!narrow) draw_text("wave", 2, WE_Y + 4, RGB6(2, 3, 3));
    if (S.pen_wave < NWAVES) {
        snprintf(buf, sizeof buf, "%x", S.pen_wave);
        if (!narrow) draw_text(buf, 2, WE_Y + 16, COL_WHITE);
        if (!narrow) draw_text("draw it", 2, WE_Y + 28, RGB6(1, 1, 2));
        draw_rect(WE_X, WE_Y, WAVE_LEN * 4, 64, RGB6(0, 0, 1), true);
        draw_line(WE_X, WE_Y + 32, WE_X + WAVE_LEN * 4 - 1, WE_Y + 32, RGB6(0, 1, 1));
        for (int i = 0; i < WAVE_LEN; i++) {
            int v = cur_snd->waves[S.pen_wave][i];
            draw_rect(WE_X + i * 4, WE_Y + (15 - v) * 4, 3, 3, COL_YELLOW, true);
        }
    } else {
        if (!narrow) draw_text("noise", 2, WE_Y + 16, COL_WHITE);
        for (int i = 0; i < 400; i++) draw_pset(WE_X + rand() % (WAVE_LEN * 4), WE_Y + rand() % 64, RGB6(1, 1, 1));
    }
    for (int w = 0; w < NWAVES; w++) thumb(w, THUMB_X + (w % 8) * 24, THUMB_Y + (w / 8) * 18, w == S.pen_wave);
    draw_rect(THUMB_X, THUMB_Y + 36, 46, 17, S.pen_wave == WAVE_NOISE ? RGB6(1, 2, 3) : RGB6(0, 0, 1), true);
    draw_text("noise", THUMB_X + 8, THUMB_Y + 40, S.pen_wave == WAVE_NOISE ? COL_BLACK : COL_YELLOW);
    for (int f = 0; f < NFX; f++) {
        int bx = THUMB_X + f * 27;
        draw_rect(bx, FXPEN_Y, 26, 11, f == S.pen_fx ? COL_ORANGE : RGB6(0, 0, 1), true);
        draw_char(fx_chars[f], bx + 10, FXPEN_Y + 1, f == S.pen_fx ? COL_BLACK : COL_ORANGE);
    }
    snprintf(buf, sizeof buf, "pen: %s, vol %d", fx_names[S.pen_fx], S.pen_vol);
    draw_text(buf, THUMB_X + 52, THUMB_Y + 40, RGB6(2, 2, 1));

    /* bars */
    ui_draw_tabs();
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, UI_W, FONT_H, RGB6(0, 1, 2), true);
    int hk = inside(EX, PITCH_Y, SFX_STEPS * SW, FX_ROW_Y + 10 - PITCH_Y) ? mouse_step() : S.cursor;
    const SfxStep *hs = &x->steps[hk];
    char name[4];
    pitch_name(hs->pitch, name);
    if (hs->vol) snprintf(buf, sizeof buf, "sfx %d step %d: %s wave %c vol %d %s", S.cur, hk, name,
                          hs->wave >= NWAVES ? 'n' : "0123456789abcdef"[hs->wave], hs->vol, fx_names[hs->fx % NFX]);
    else snprintf(buf, sizeof buf, "sfx %d step %d: rest", S.cur, hk);
    int bx = draw_text(buf, 3, by, RGB6(2, 3, 3));
    if (S.status[0] && frame_count < S.status_until) draw_text(S.status, bx + 12, by, S.status_color);
    snprintf(buf, sizeof buf, "%s%s", doc_active(RES_SFX)->name, doc_active(RES_SFX)->dirty ? "*" : "");
    draw_text(buf, UI_W - 3 - text_width(buf), by, RGB6(2, 3, 3));
}

/* the tree opened another sound bank: its undo history doesn't apply */
void sound_doc_changed(void) { S.nundo = S.nredo = 0; }

void sound_frame(void) {
    init();
    layout();
    if (!cart.loaded) { ui_switch(MODE_SHELL); return; }
    for (int i = 0; i < in.nevents && ui_mode == MODE_SOUND; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        if (ui_global_key(ev)) { stop_all(); continue; }
        key(ev);
    }
    if (ui_mode != MODE_SOUND) { stop_all(); return; }
    mouse();
    draw();
}
