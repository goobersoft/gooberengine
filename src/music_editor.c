/* The built-in music editor (F5): a tracker.
 *
 *   speed - 8 +   order: [00][01][00][>0][ .][ .] ...     the song
 *   row | track 1 | track 2 | ... | track 8                one pattern
 *    00 | C-4 05. | ...     |
 *
 * Each cell is note, wave, volume, effect. The cursor sits on one of
 * those four fields: on the note, the Z..M / Q..U keys play and enter
 * notes (1 = note off); on the others, type the value (hex digit or n for
 * the wave, 0-7 for the volume, an effect character). Track t plays on
 * voice t, so a game's sound effects can borrow a track's voice. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SONG_Y 11
#define ORDER_X 118
#define ORDER_W 18
#define ORDER_SHOWN ((UI_W - ORDER_X) / ORDER_W)
#define TSHOWN ((UI_W - GX) / TW < NTRACKS ? (UI_W - GX) / TW : NTRACKS)   /* tracks on screen */
#define HEAD_Y 23
#define GRID_Y 33
#define ROWS_SHOWN 19
#define GX 20                 /* first track's x */
#define TW 46                 /* track width: "C-4 05." + gap */
#define MAX_UNDO 32
#define PREVIEW_VOICE (NVOICES - 1)

enum { F_NOTE, F_WAVE, F_VOL, F_FX };

typedef struct { Pattern patterns[NPATTERNS]; OrderEntry order[ORDER_LEN]; uint8_t speed; } SongSnap;

static struct {
    bool ready;
    int pat;                  /* pattern shown */
    int opos;                 /* order entry selected */
    int oscroll;
    int row, track, field;
    int top;
    int octave, pen_wave, pen_vol;
    SongSnap undo[MAX_UNDO], redo[MAX_UNDO];
    int tleft;                /* first track shown (they don't all fit beside the file tree) */
    int nundo, nredo;
    Pattern clip;
    bool has_clip;
    char status[128];
    int status_color;
    long status_until;
} U;

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(U.status, sizeof U.status, fmt, ap);
    va_end(ap);
    U.status_color = color;
    U.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[music] %s\n", U.status);
}

static void init(void) {
    if (U.ready) return;
    U.ready = true;
    U.octave = 2;
    U.pen_vol = 5;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
static Cell *cell(void) { return &cur_song->patterns[U.pat].cells[U.row][U.track]; }

/* ---------------------------------------------------------------- undo */

static void push(SongSnap *stack, int *n) {
    if (*n == MAX_UNDO) { memmove(stack, stack + 1, sizeof(SongSnap) * (MAX_UNDO - 1)); (*n)--; }
    memcpy(stack[*n].patterns, cur_song->patterns, sizeof cur_song->patterns);
    memcpy(stack[*n].order, cur_song->order, sizeof cur_song->order);
    stack[*n].speed = cur_song->song_speed;
    (*n)++;
}

static void before_change(void) {
    push(U.undo, &U.nundo);
    U.nredo = 0;
    doc_touch(RES_MUSIC);
}

static void undo_redo(bool redo) {
    SongSnap *from = redo ? U.redo : U.undo, *to = redo ? U.undo : U.redo;
    int *nfrom = redo ? &U.nredo : &U.nundo, *nto = redo ? &U.nundo : &U.nredo;
    if (!*nfrom) { status(COL_GREY, "nothing to %s", redo ? "redo" : "undo"); return; }
    push(to, nto);
    --*nfrom;
    audio_lock();
    memcpy(cur_song->patterns, from[*nfrom].patterns, sizeof cur_song->patterns);
    memcpy(cur_song->order, from[*nfrom].order, sizeof cur_song->order);
    cur_song->song_speed = from[*nfrom].speed;
    audio_unlock();
    doc_touch(RES_MUSIC);
}

/* ---------------------------------------------------------------- playing */

static void stop_all(void) {
    audio_lock();
    synth_stop(-1);
    audio_unlock();
}

static void play(bool pattern_only) {
    audio_lock();
    if (synth_music_pattern() >= 0) synth_stop(-1);
    else if (pattern_only) synth_music(0, U.pat);
    else synth_music(U.opos, -1);
    audio_unlock();
}

static void preview(const Cell *c) {
    if (c->kind != CELL_NOTE || !c->vol) return;
    audio_lock();
    synth_note(PREVIEW_VOICE, c->pitch, c->wave, c->vol, TICK_HZ / 6);
    audio_unlock();
}

/* ---------------------------------------------------------------- the order list */

static void select_entry(int i) {
    U.opos = clampi(i, 0, ORDER_LEN - 1);
    if (cur_song->order[U.opos].kind == ORDER_PATTERN) U.pat = cur_song->order[U.opos].value;
}

static void change_entry(int d) {
    OrderEntry *e = &cur_song->order[U.opos];
    before_change();
    if (e->kind == ORDER_STOP) { e->kind = ORDER_PATTERN; e->value = 0; }
    else e->value = (uint8_t)clampi(e->value + d, 0, (e->kind == ORDER_PATTERN ? NPATTERNS : ORDER_LEN) - 1);
    select_entry(U.opos);
}

static void cycle_entry_kind(void) {
    OrderEntry *e = &cur_song->order[U.opos];
    before_change();
    e->kind = (uint8_t)((e->kind + 1) % 3);
    e->value = 0;
    select_entry(U.opos);
}

static void insert_entry(void) {
    before_change();
    memmove(&cur_song->order[U.opos + 1], &cur_song->order[U.opos], sizeof(OrderEntry) * (ORDER_LEN - 1 - U.opos));
    for (int i = 0; i < ORDER_LEN; i++)       /* jumps past the new entry still land where they did */
        if (i != U.opos && cur_song->order[i].kind == ORDER_JUMP && cur_song->order[i].value > U.opos && cur_song->order[i].value < ORDER_LEN - 1)
            cur_song->order[i].value++;
}

static void delete_entry(void) {
    before_change();
    memmove(&cur_song->order[U.opos], &cur_song->order[U.opos + 1], sizeof(OrderEntry) * (ORDER_LEN - 1 - U.opos));
    cur_song->order[ORDER_LEN - 1] = (OrderEntry){ORDER_STOP, 0};
    for (int i = 0; i < ORDER_LEN; i++)
        if (cur_song->order[i].kind == ORDER_JUMP && cur_song->order[i].value > U.opos) cur_song->order[i].value--;
    select_entry(U.opos);
}

/* ---------------------------------------------------------------- editing cells */

static void advance(void) { if (U.row < PATTERN_ROWS - 1) U.row++; }

static void enter_note(int semitone) {
    before_change();
    Cell *c = cell();
    bool fresh = c->kind != CELL_NOTE;
    c->kind = CELL_NOTE;
    c->pitch = (uint8_t)clampi(U.octave * 12 + semitone, 0, NPITCH - 1);
    if (fresh) { c->wave = (uint8_t)U.pen_wave; c->vol = (uint8_t)U.pen_vol; c->fx = FX_NONE; }
    preview(c);
    advance();
}

static void type_field(char ch) {
    Cell *c = cell();
    if (U.field == F_NOTE || c->kind != CELL_NOTE) return;
    if (U.field == F_WAVE) {
        int w = ch == 'n' || ch == 'N' ? WAVE_NOISE
              : ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : -1;
        if (w < 0) return;
        before_change();
        c->wave = (uint8_t)(U.pen_wave = w);
    } else if (U.field == F_VOL) {
        if (ch < '0' || ch > '7') return;
        before_change();
        c->vol = (uint8_t)(U.pen_vol = ch - '0');
    } else {
        const char *f = strchr(fx_chars, ch);
        if (!f || !ch) return;
        before_change();
        c->fx = (uint8_t)(f - fx_chars);
    }
    preview(c);
    advance();
}

static void move_field(int d) {
    int f = U.track * 4 + U.field + d;
    f = clampi(f, 0, NTRACKS * 4 - 1);
    U.track = f / 4;
    U.field = f % 4;
}

static void key(const KeyEvent *ev) {
    int sc = ev->sc;
    bool ctrl = ev->mod & MOD_CTRL, shift = ev->mod & MOD_SHIFT;
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: {
                char err[256];
                Doc *d = doc_active(RES_MUSIC);
                if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved music/%s.txt", d->name);
                else status(COL_RED, "%s", err);
                return;
            }
            case SDL_SCANCODE_R: stop_all(); ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: undo_redo(shift); return;
            case SDL_SCANCODE_Y: undo_redo(true); return;
            case SDL_SCANCODE_C: U.clip = cur_song->patterns[U.pat]; U.has_clip = true; status(COL_GREY, "copied pattern %d", U.pat); return;
            case SDL_SCANCODE_X:
                U.clip = cur_song->patterns[U.pat];
                U.has_clip = true;
                before_change();
                for (int r = 0; r < PATTERN_ROWS; r++)
                    for (int t = 0; t < NTRACKS; t++) cur_song->patterns[U.pat].cells[r][t] = (Cell){CELL_EMPTY, 24, 0, 0, 0};
                return;
            case SDL_SCANCODE_V:
                if (!U.has_clip) return;
                before_change();
                cur_song->patterns[U.pat] = U.clip;
                return;
            case SDL_SCANCODE_LEFT: select_entry(U.opos - 1); return;
            case SDL_SCANCODE_RIGHT: select_entry(U.opos + 1); return;
            case SDL_SCANCODE_UP: change_entry(1); return;
            case SDL_SCANCODE_DOWN: change_entry(-1); return;
            case SDL_SCANCODE_J: before_change(); cur_song->order[U.opos] = (OrderEntry){ORDER_JUMP, 0}; return;
            case SDL_SCANCODE_PERIOD: before_change(); cur_song->order[U.opos] = (OrderEntry){ORDER_STOP, 0}; return;
            case SDL_SCANCODE_I: case SDL_SCANCODE_INSERT: insert_entry(); return;
            case SDL_SCANCODE_DELETE: case SDL_SCANCODE_BACKSPACE: delete_entry(); return;
            default: return;
        }
    }
    if (U.field == F_NOTE) {
        static const struct { int sc, semi; } notes[] = {
            {SDL_SCANCODE_Z, 0}, {SDL_SCANCODE_S, 1}, {SDL_SCANCODE_X, 2}, {SDL_SCANCODE_D, 3}, {SDL_SCANCODE_C, 4},
            {SDL_SCANCODE_V, 5}, {SDL_SCANCODE_G, 6}, {SDL_SCANCODE_B, 7}, {SDL_SCANCODE_H, 8}, {SDL_SCANCODE_N, 9},
            {SDL_SCANCODE_J, 10}, {SDL_SCANCODE_M, 11}, {SDL_SCANCODE_Q, 12}, {SDL_SCANCODE_W, 14}, {SDL_SCANCODE_E, 16},
            {SDL_SCANCODE_R, 17}, {SDL_SCANCODE_T, 19}, {SDL_SCANCODE_Y, 21}, {SDL_SCANCODE_U, 23},
        };
        for (size_t i = 0; i < sizeof notes / sizeof notes[0]; i++)
            if (sc == notes[i].sc) { enter_note(notes[i].semi); return; }
        if (sc == SDL_SCANCODE_1) {
            before_change();
            *cell() = (Cell){CELL_OFF, 24, 0, 0, 0};
            advance();
            return;
        }
    }
    switch (sc) {
        case SDL_SCANCODE_ESCAPE:
            if (synth_music_pattern() >= 0) { stop_all(); return; }
            ui_switch(MODE_SHELL);
            return;
        case SDL_SCANCODE_SPACE: play(shift); return;
        case SDL_SCANCODE_UP: U.row = clampi(U.row - 1, 0, PATTERN_ROWS - 1); return;
        case SDL_SCANCODE_DOWN: U.row = clampi(U.row + 1, 0, PATTERN_ROWS - 1); return;
        case SDL_SCANCODE_PAGEUP: U.row = clampi(U.row - 8, 0, PATTERN_ROWS - 1); return;
        case SDL_SCANCODE_PAGEDOWN: U.row = clampi(U.row + 8, 0, PATTERN_ROWS - 1); return;
        case SDL_SCANCODE_HOME: U.row = 0; return;
        case SDL_SCANCODE_END: U.row = PATTERN_ROWS - 1; return;
        case SDL_SCANCODE_LEFT: move_field(-1); return;
        case SDL_SCANCODE_RIGHT: move_field(1); return;
        case SDL_SCANCODE_TAB: U.track = clampi(U.track + (shift ? -1 : 1), 0, NTRACKS - 1); return;
        case SDL_SCANCODE_DELETE:
            before_change();
            *cell() = (Cell){CELL_EMPTY, 24, 0, 0, 0};
            advance();
            return;
        case SDL_SCANCODE_BACKSPACE:
            if (U.row > 0) U.row--;
            before_change();
            *cell() = (Cell){CELL_EMPTY, 24, 0, 0, 0};
            return;
        case SDL_SCANCODE_LEFTBRACKET: U.octave = clampi(U.octave - 1, 0, 4); return;
        case SDL_SCANCODE_RIGHTBRACKET: U.octave = clampi(U.octave + 1, 0, 4); return;
        case SDL_SCANCODE_MINUS: if (U.field == F_NOTE) { before_change(); cur_song->song_speed = (uint8_t)clampi(cur_song->song_speed - 1, 1, 64); } return;
        case SDL_SCANCODE_EQUALS: if (U.field == F_NOTE) { before_change(); cur_song->song_speed = (uint8_t)clampi(cur_song->song_speed + 1, 1, 64); } return;
        default: return;
    }
}

/* ---------------------------------------------------------------- mouse */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

static void mouse(void) {
    int btn = in.mouse_hit[1] ? 1 : in.mouse_hit[2] ? 2 : 0;
    if (inside(ORDER_X, SONG_Y, ORDER_SHOWN * ORDER_W, 10)) {
        int i = U.oscroll + (in.mouse_x - ORDER_X) / ORDER_W;
        if (btn) select_entry(i);
        if (btn == 2) cycle_entry_kind();
        if (in.wheel) { select_entry(i); change_entry(in.wheel > 0 ? 1 : -1); }
        return;
    }
    if (in.wheel && inside(0, GRID_Y, UI_W, ROWS_SHOWN * FONT_H)) U.top = clampi(U.top - in.wheel * 2, 0, PATTERN_ROWS - ROWS_SHOWN);
    if (!btn) return;
    if (inside(36, SONG_Y, 8, 10) || inside(62, SONG_Y, 8, 10)) {
        before_change();
        cur_song->song_speed = (uint8_t)clampi(cur_song->song_speed + (in.mouse_x < 50 ? -1 : 1), 1, 64);
        return;
    }
    if (inside(GX, GRID_Y, TSHOWN * TW, ROWS_SHOWN * FONT_H)) {
        U.row = clampi(U.top + (in.mouse_y - GRID_Y) / FONT_H, 0, PATTERN_ROWS - 1);
        int x = in.mouse_x - GX;
        U.track = clampi(U.tleft + x / TW, 0, NTRACKS - 1);
        int col = (x % TW) / FONT_W;           /* "C-4 05." : 0-3 note, 4 wave, 5 vol, 6 fx */
        U.field = col <= 3 ? F_NOTE : col == 4 ? F_WAVE : col == 5 ? F_VOL : F_FX;
    }
}

/* ---------------------------------------------------------------- drawing */

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    char buf[128];
    int mpos = synth_music_pos(), mpat = synth_music_pattern(), mrow = synth_music_row();
    if (mpos >= 0) { U.opos = mpos; U.pat = mpat; }          /* follow the song while it plays */

    /* song: speed and the order list */
    snprintf(buf, sizeof buf, "speed - %-2d +", cur_song->song_speed);
    draw_text(buf, 2, SONG_Y, RGB6(2, 3, 3));
    draw_text("order", ORDER_X - 34, SONG_Y, RGB6(2, 2, 3));
    if (U.opos < U.oscroll) U.oscroll = U.opos;
    if (U.opos >= U.oscroll + ORDER_SHOWN) U.oscroll = U.opos - ORDER_SHOWN + 1;
    for (int k = 0; k < ORDER_SHOWN; k++) {
        int i = U.oscroll + k;
        if (i >= ORDER_LEN) break;
        const OrderEntry *e = &cur_song->order[i];
        int x = ORDER_X + k * ORDER_W;
        if (i == U.opos) draw_rect(x, SONG_Y, ORDER_W - 1, 10, RGB6(1, 2, 3), true);
        if (e->kind == ORDER_PATTERN) snprintf(buf, sizeof buf, "%02d", e->value);
        else if (e->kind == ORDER_JUMP) snprintf(buf, sizeof buf, ">%d", e->value);
        else snprintf(buf, sizeof buf, " .");
        int c = i == U.opos ? COL_BLACK : i == mpos ? COL_YELLOW : e->kind == ORDER_STOP ? RGB6(1, 1, 2) : e->kind == ORDER_JUMP ? COL_ORANGE : COL_WHITE;
        draw_text(buf, x + 3, SONG_Y, c);
    }

    /* grid */
    if (U.row < U.top) U.top = U.row;
    if (U.row >= U.top + ROWS_SHOWN) U.top = U.row - ROWS_SHOWN + 1;
    if (mpat == U.pat && mpos >= 0) {                        /* keep the playing row in view */
        if (mrow < U.top) U.top = mrow;
        if (mrow >= U.top + ROWS_SHOWN) U.top = clampi(mrow - ROWS_SHOWN / 2, 0, PATTERN_ROWS - ROWS_SHOWN);
    }
    if (U.track < U.tleft) U.tleft = U.track;
    if (U.track >= U.tleft + TSHOWN) U.tleft = U.track - TSHOWN + 1;
    U.tleft = clampi(U.tleft, 0, NTRACKS - TSHOWN);
    snprintf(buf, sizeof buf, "%02d", U.pat);
    draw_text(buf, 2, HEAD_Y, COL_YELLOW);
    for (int k = 0; k < TSHOWN; k++) {
        int t = U.tleft + k;
        snprintf(buf, sizeof buf, "track %d", t + 1);
        draw_text(buf, GX + k * TW, HEAD_Y, t == U.track ? COL_WHITE : RGB6(1, 2, 2));
    }
    if (U.tleft > 0) draw_char('<', GX - 8, HEAD_Y, COL_YELLOW);
    if (U.tleft + TSHOWN < NTRACKS) draw_char('>', UI_W - 6, HEAD_Y, COL_YELLOW);
    for (int r = 0; r < ROWS_SHOWN; r++) {
        int row = U.top + r, y = GRID_Y + r * FONT_H;
        if (row >= PATTERN_ROWS) break;
        bool playing = mpat == U.pat && row == mrow;
        if (playing) draw_rect(0, y, UI_W, FONT_H, RGB6(1, 1, 0), true);
        else if (row == U.row) draw_rect(0, y, UI_W, FONT_H, RGB6(0, 0, 1), true);
        snprintf(buf, sizeof buf, "%02d", row);
        draw_text(buf, 2, y, row % 8 == 0 ? COL_YELLOW : row % 4 == 0 ? RGB6(2, 2, 3) : RGB6(1, 1, 2));
        for (int k = 0; k < TSHOWN; k++) {
            int t = U.tleft + k;
            const Cell *c = &cur_song->patterns[U.pat].cells[row][t];
            int x = GX + k * TW;
            if (row == U.row && t == U.track) {
                int fx0 = U.field == F_NOTE ? 0 : U.field + 3, fw = U.field == F_NOTE ? 3 : 1;
                draw_rect(x + fx0 * FONT_W - 1, y, fw * FONT_W + 1, FONT_H, RGB6(1, 2, 3), true);
            }
            if (c->kind == CELL_EMPTY) { draw_text("... ...", x, y, RGB6(0, 1, 2)); continue; }
            if (c->kind == CELL_OFF) { draw_text("=== ...", x, y, COL_RED); draw_text("...", x + 4 * FONT_W, y, RGB6(0, 1, 2)); continue; }
            char name[4];
            pitch_name(c->pitch, name);
            draw_text(name, x, y, COL_WHITE);
            draw_char(c->wave >= NWAVES ? 'n' : "0123456789abcdef"[c->wave], x + 4 * FONT_W, y, COL_CYAN);
            draw_char('0' + c->vol, x + 5 * FONT_W, y, RGB6(3, 1, 2));
            draw_char(fx_chars[c->fx % NFX], x + 6 * FONT_W, y, COL_ORANGE);
        }
    }
    for (int t = 1; t < TSHOWN; t++) draw_line(GX + t * TW - 3, HEAD_Y, GX + t * TW - 3, GRID_Y + ROWS_SHOWN * FONT_H - 1, RGB6(0, 1, 1));

    /* bars */
    ui_draw_tabs();
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, UI_W, FONT_H, RGB6(0, 1, 2), true);
    static const char *const field_names[4] = {"note", "wave", "volume", "effect"};
    snprintf(buf, sizeof buf, "pat %d row %d trk %d %s  oct %d", U.pat, U.row, U.track + 1, field_names[U.field], U.octave + 2);
    int bx = draw_text(buf, 3, by, RGB6(2, 3, 3));
    if (U.status[0] && frame_count < U.status_until) draw_text(U.status, bx + 10, by, U.status_color);
    snprintf(buf, sizeof buf, "%s%s", doc_active(RES_MUSIC)->name, doc_active(RES_MUSIC)->dirty ? "*" : "");
    draw_text(buf, UI_W - 3 - text_width(buf), by, RGB6(2, 3, 3));
}

/* the tree opened another song: its undo history doesn't apply */
void music_doc_changed(void) {
    U.nundo = U.nredo = 0;
    U.opos = U.pat = U.row = U.top = 0;
}

void music_frame(void) {
    init();
    if (!cart.loaded) { ui_switch(MODE_SHELL); return; }
    for (int i = 0; i < in.nevents && ui_mode == MODE_MUSIC; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type == EV_KEY) {
            if (ui_global_key(ev)) { stop_all(); continue; }
            key(ev);
        } else if (ev->text[0] && !ev->text[1]) type_field(ev->text[0]);
    }
    if (ui_mode != MODE_MUSIC) { stop_all(); return; }
    mouse();
    draw();
}
