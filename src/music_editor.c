/* The built-in music editor: a tracker.
 *
 *   speed - 8 +   order: [00][01][00][>0][ .][ .] ...     the song
 *   pat | row | track 1 | track 2 | ... | track 8          one pattern
 *  00 01|  00 | C-4 05. | ...     |
 *  02 03|
 *
 * The pattern list down the left picks the pattern to edit (Ctrl+PgUp /
 * PgDn); a right click, or Ctrl+Enter, puts it in the order entry.
 *
 * Each cell is note, wave, volume, effect. The cursor sits on one of
 * those four fields: on the note, the Z..M / Q..U keys play and enter
 * notes (1 = note off); on the others, type the value (hex digit or n for
 * the wave, 0-7 for the volume, an effect character). Track t plays on
 * voice t, so a game's sound effects can borrow a track's voice.
 *
 * Ctrl+P swaps the grid for a piano roll of one track: time runs left to
 * right (one column a row), pitch bottom to top, with a keyboard down the
 * side. Drag with the left button to draw a note as long as the drag;
 * the right button rubs notes out. It writes the same cells the grid does:
 * a note where the bar starts, empty cells along it, an off after it. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SONG_Y 11
#define ORDER_LABEL_X 80      /* "order 02 <" then the entries, then ">" */
#define ORDER_IDX_X 116
#define ORDER_PREV_X 132
#define ORDER_X 142
#define ORDER_W 18
#define ORDER_NEXT_X (UI_W - 8)
#define ORDER_SHOWN ((ORDER_NEXT_X - 2 - ORDER_X) / ORDER_W)
#define HEAD_Y 23
#define GRID_Y 33
#define ROWS_SHOWN 19
#define TW 44                 /* track width: "C-4 05." + gap */
#define CELL_W (7 * FONT_W)
#define TSHOWN ((UI_W - GX - CELL_W) / TW + 1 < NTRACKS ? (UI_W - GX - CELL_W) / TW + 1 : NTRACKS)   /* tracks on screen */
#define MAX_UNDO 32
#define PREVIEW_VOICE (NVOICES - 1)
#define LIST_X 2              /* the pattern list down the left, two columns of 16 */
#define LIST_ROW 10
#define LIST_COL 14
#define LIST_W (2 * LIST_COL)
#define TRK_X 50              /* roll header: "trk 1 2 ... 8" */
static bool narrow;           /* beside the file tree: no pattern list (Ctrl+PgUp / PgDn) */
static int GX = 20;           /* first track's x */
static int KEYS_X = 14;       /* roll: the keyboard, octave labels left of it */
static int RX = 36;           /* roll: first column's x */
static int ROWNUM_X = 2;      /* grid: the row numbers */

static void layout(void) {
    narrow = UI_W < LIST_W + 20 + 7 * TW + CELL_W;
    int s = narrow ? 0 : LIST_W + 2;
    GX = 20 + s;
    KEYS_X = 14 + s;
    RX = 36 + s;
    ROWNUM_X = 2 + s;
}
#define PH 6                  /* roll: pixels per pitch */
#define PSHOWN 32             /* roll: pitches on screen */
#define ROLL_BOTTOM (GRID_Y + PSHOWN * PH)
#define CW (UI_W - RX - 1 < PATTERN_ROWS * 4 ? 4 : (UI_W - RX - 1) / PATTERN_ROWS)   /* roll: column width */
#define PEN_X 158             /* roll header: "wave - 0 +  vol - 5 +" */
#define VOLPEN_X 226

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
    bool roll;                /* the piano roll instead of the tracker grid (Ctrl+P) */
    int plow;                 /* lowest pitch the roll shows */
    int rdrag, r0;            /* roll drag: 0 none, 1 drawing a note from row r0, 2 erasing */
    Cell col[PATTERN_ROWS];   /* the track as it was when the drag began */
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
    U.plow = 12;
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

static void select_pattern(int p) { U.pat = clampi(p, 0, NPATTERNS - 1); }

/* the selected order entry plays pattern p */
static void assign_pattern(int p) {
    before_change();
    cur_song->order[U.opos] = (OrderEntry){ORDER_PATTERN, (uint8_t)p};
    U.pat = p;
}

static bool pattern_used(int p) {
    for (int r = 0; r < PATTERN_ROWS; r++)
        for (int t = 0; t < NTRACKS; t++)
            if (cur_song->patterns[p].cells[r][t].kind != CELL_EMPTY) return true;
    return false;
}

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

/* ---------------------------------------------------------------- the piano roll's notes */

static Cell *at(int r, int t) { return &cur_song->patterns[U.pat].cells[r][t]; }

/* the row of the note sounding at row r of track t (in this pattern), or -1 */
static int note_start(int t, int r) {
    for (; r >= 0; r--) {
        if (at(r, t)->kind == CELL_NOTE) return r;
        if (at(r, t)->kind == CELL_OFF) return -1;
    }
    return -1;
}

/* the row just past the note that starts at row s: the next note or off, or the pattern's end */
static int note_end(int t, int s) {
    int e = s + 1;
    while (e < PATTERN_ROWS && at(e, t)->kind == CELL_EMPTY) e++;
    return e;
}

static void roll_show(int p) {
    if (p < U.plow) U.plow = p;
    if (p >= U.plow + PSHOWN) U.plow = p - PSHOWN + 1;
    U.plow = clampi(U.plow, 0, NPITCH - PSHOWN);
}

/* scroll the roll to the middle of the track's notes */
static void roll_fit(void) {
    int lo = NPITCH, hi = -1;
    for (int r = 0; r < PATTERN_ROWS; r++)
        if (at(r, U.track)->kind == CELL_NOTE) {
            int p = at(r, U.track)->pitch;
            if (p < lo) lo = p;
            if (p > hi) hi = p;
        }
    if (hi < 0) lo = hi = U.octave * 12 + 6;          /* no notes: the octave the keys play */
    U.plow = clampi((lo + hi) / 2 - PSHOWN / 2, 0, NPITCH - PSHOWN);
}

static void roll_toggle(void) {
    U.roll = !U.roll;
    U.rdrag = 0;
    U.field = F_NOTE;
    if (U.roll) roll_fit();
}

/* draw: the note at row r0 of the track, pitch p, sounding to row r1 (the drag started from U.col) */
static void roll_place(int r0, int r1, int p) {
    for (int r = 0; r < PATTERN_ROWS; r++) *at(r, U.track) = U.col[r];
    Cell *c = at(r0, U.track);
    if (U.col[r0].kind != CELL_NOTE) {        /* a new note takes the pens; an old one keeps its sound */
        c->wave = (uint8_t)U.pen_wave;
        c->vol = (uint8_t)(U.pen_vol ? U.pen_vol : 5);
        c->fx = FX_NONE;
    }
    c->kind = CELL_NOTE;
    c->pitch = (uint8_t)p;
    for (int r = r0 + 1; r <= r1; r++) *at(r, U.track) = (Cell){CELL_EMPTY, 24, 0, 0, 0};
    if (r1 + 1 < PATTERN_ROWS && U.col[r1 + 1].kind == CELL_EMPTY) *at(r1 + 1, U.track) = (Cell){CELL_OFF, 24, 0, 0, 0};
}

/* rub out the note sounding at row r of the track */
static void roll_erase(int r) {
    int s = note_start(U.track, r);
    if (s < 0) return;
    int e = note_end(U.track, s);
    bool before = s > 0 && note_start(U.track, s - 1) >= 0;    /* a note before it would carry on into the gap */
    *at(s, U.track) = (Cell){before ? CELL_OFF : CELL_EMPTY, 24, 0, 0, 0};
    if (e < PATTERN_ROWS && at(e, U.track)->kind == CELL_OFF) *at(e, U.track) = (Cell){CELL_EMPTY, 24, 0, 0, 0};
}

static void enter_note(int semitone) {
    before_change();
    Cell *c = cell();
    bool fresh = c->kind != CELL_NOTE;
    c->kind = CELL_NOTE;
    c->pitch = (uint8_t)clampi(U.octave * 12 + semitone, 0, NPITCH - 1);
    if (fresh) { c->wave = (uint8_t)U.pen_wave; c->vol = (uint8_t)U.pen_vol; c->fx = FX_NONE; }
    if (U.roll) roll_show(c->pitch);
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
                if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved music/%s.g64", d->name);
                else status(COL_RED, "%s", err);
                return;
            }
            case SDL_SCANCODE_R: stop_all(); ui_run_from_editor(); return;
            case SDL_SCANCODE_P: roll_toggle(); return;
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
            case SDL_SCANCODE_PAGEUP: select_pattern(U.pat - 1); return;
            case SDL_SCANCODE_PAGEDOWN: select_pattern(U.pat + 1); return;
            case SDL_SCANCODE_RETURN: assign_pattern(U.pat); return;
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
    if (U.roll) {                     /* time runs across the roll, pitch up it */
        switch (sc) {
            case SDL_SCANCODE_LEFT: U.row = clampi(U.row - (shift ? 8 : 1), 0, PATTERN_ROWS - 1); return;
            case SDL_SCANCODE_RIGHT: U.row = clampi(U.row + (shift ? 8 : 1), 0, PATTERN_ROWS - 1); return;
            case SDL_SCANCODE_UP: U.plow = clampi(U.plow + (shift ? 12 : 1), 0, NPITCH - PSHOWN); return;
            case SDL_SCANCODE_DOWN: U.plow = clampi(U.plow - (shift ? 12 : 1), 0, NPITCH - PSHOWN); return;
            case SDL_SCANCODE_TAB: U.track = clampi(U.track + (shift ? -1 : 1), 0, NTRACKS - 1); return;
            default: break;
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

/* the bottom bar's grid / roll button sits left of the song's name */
static const char *doc_label(void) {
    static char buf[80];
    snprintf(buf, sizeof buf, "%s%s", doc_active(RES_MUSIC)->name, doc_active(RES_MUSIC)->dirty ? "*" : "");
    return buf;
}
#define VIEW_BTN_W 34
static int view_btn_x(void) { return UI_W - STATUS_CLOSE_W - 2 - text_width(doc_label()) - 8 - VIEW_BTN_W; }

static int roll_row(void) { return clampi((in.mouse_x - RX) / CW, 0, PATTERN_ROWS - 1); }
static int roll_pitch(void) { return clampi(U.plow + (ROLL_BOTTOM - 1 - in.mouse_y) / PH, 0, NPITCH - 1); }
static bool over_roll(void) { return inside(RX, GRID_Y, PATTERN_ROWS * CW, PSHOWN * PH); }
static bool over_keys(void) { return inside(KEYS_X - 14, GRID_Y, RX - KEYS_X + 14, PSHOWN * PH); }

static void roll_mouse(int btn) {
    if (U.rdrag) {
        int b = U.rdrag == 1 ? 1 : 2;
        if (in.mouse[b] && !in.mouse_hit[b]) {
            if (U.rdrag == 2) { if (over_roll()) roll_erase(roll_row()); return; }
            int r1 = roll_row() > U.r0 ? roll_row() : U.r0, p = roll_pitch();
            int was = at(U.r0, U.track)->pitch;
            roll_place(U.r0, r1, p);
            if (p != was) preview(at(U.r0, U.track));
            return;
        }
        U.rdrag = 0;
    }
    if (in.wheel && (over_roll() || over_keys())) U.plow = clampi(U.plow + in.wheel * 2, 0, NPITCH - PSHOWN);
    if (!btn) return;
    if (inside(TRK_X + 20, HEAD_Y, NTRACKS * 10, 10)) { U.track = (in.mouse_x - TRK_X - 20) / 10; return; }
    const struct { int x, *pen, lo, hi, d; } pens[] = {
        {PEN_X + 30, &U.pen_wave, 0, WAVE_NOISE, -1}, {PEN_X + 54, &U.pen_wave, 0, WAVE_NOISE, 1},
        {VOLPEN_X + 24, &U.pen_vol, 1, 7, -1}, {VOLPEN_X + 48, &U.pen_vol, 1, 7, 1},
    };
    for (size_t i = 0; i < sizeof pens / sizeof pens[0]; i++)
        if (inside(pens[i].x - 1, HEAD_Y, 8, 10)) { *pens[i].pen = clampi(*pens[i].pen + pens[i].d, pens[i].lo, pens[i].hi); return; }
    if (over_keys() && btn == 1) {            /* a key of the keyboard: hear it */
        Cell c = {CELL_NOTE, (uint8_t)roll_pitch(), (uint8_t)U.pen_wave, (uint8_t)(U.pen_vol ? U.pen_vol : 5), FX_NONE};
        preview(&c);
        return;
    }
    if (!over_roll()) return;
    U.row = roll_row();
    before_change();
    for (int r = 0; r < PATTERN_ROWS; r++) U.col[r] = *at(r, U.track);
    if (btn == 1) {
        U.rdrag = 1;
        U.r0 = U.row;
        roll_place(U.r0, U.r0, roll_pitch());
        preview(at(U.r0, U.track));
    } else {
        U.rdrag = 2;
        roll_erase(U.row);
    }
}

static void mouse(void) {
    int btn = in.mouse_hit[1] ? 1 : in.mouse_hit[2] ? 2 : 0;
    if (btn == 1 && inside(view_btn_x(), SCREEN_H - FONT_H, VIEW_BTN_W, FONT_H)) { roll_toggle(); return; }
    if (btn == 1 && inside(ORDER_PREV_X - 1, SONG_Y, 8, 10)) { select_entry(U.opos - 1); return; }
    if (btn == 1 && inside(ORDER_NEXT_X - 1, SONG_Y, 8, 10)) { select_entry(U.opos + 1); return; }
    if (inside(ORDER_X, SONG_Y, ORDER_SHOWN * ORDER_W, 10)) {
        int i = U.oscroll + (in.mouse_x - ORDER_X) / ORDER_W;
        if (btn) select_entry(i);
        if (btn == 2) cycle_entry_kind();
        if (in.wheel) { select_entry(i); change_entry(in.wheel > 0 ? 1 : -1); }
        return;
    }
    /* the pattern list: left click edits one, right click puts it in the order entry */
    if (!narrow && inside(LIST_X, GRID_Y, LIST_W, (NPATTERNS / 2) * LIST_ROW)) {
        int p = ((in.mouse_y - GRID_Y) / LIST_ROW) * 2 + (in.mouse_x - LIST_X) / LIST_COL;
        if (btn == 1) select_pattern(p);
        if (btn == 2) assign_pattern(p);
        if (in.wheel) select_pattern(U.pat - in.wheel);
        return;
    }
    if (btn && (inside(36, SONG_Y, 8, 10) || inside(62, SONG_Y, 8, 10))) {
        before_change();
        cur_song->song_speed = (uint8_t)clampi(cur_song->song_speed + (in.mouse_x < 50 ? -1 : 1), 1, 64);
        return;
    }
    if (U.roll) { roll_mouse(btn); return; }
    if (in.wheel && inside(0, GRID_Y, UI_W, ROWS_SHOWN * FONT_H)) U.top = clampi(U.top - in.wheel * 2, 0, PATTERN_ROWS - ROWS_SHOWN);
    if (!btn) return;
    if (inside(GX, GRID_Y, TSHOWN * TW, ROWS_SHOWN * FONT_H)) {
        U.row = clampi(U.top + (in.mouse_y - GRID_Y) / FONT_H, 0, PATTERN_ROWS - 1);
        int x = in.mouse_x - GX;
        U.track = clampi(U.tleft + x / TW, 0, NTRACKS - 1);
        int col = (x % TW) / FONT_W;           /* "C-4 05." : 0-3 note, 4 wave, 5 vol, 6 fx */
        U.field = col <= 3 ? F_NOTE : col == 4 ? F_WAVE : col == 5 ? F_VOL : F_FX;
    }
}

/* ---------------------------------------------------------------- drawing */

/* the tracker: rows down, the tracks across */
static void draw_grid(int mpos, int mpat, int mrow) {
    char buf[128];
    if (U.row < U.top) U.top = U.row;
    if (U.row >= U.top + ROWS_SHOWN) U.top = U.row - ROWS_SHOWN + 1;
    if (mpat == U.pat && mpos >= 0) {                        /* keep the playing row in view */
        if (mrow < U.top) U.top = mrow;
        if (mrow >= U.top + ROWS_SHOWN) U.top = clampi(mrow - ROWS_SHOWN / 2, 0, PATTERN_ROWS - ROWS_SHOWN);
    }
    if (U.track < U.tleft) U.tleft = U.track;
    if (U.track >= U.tleft + TSHOWN) U.tleft = U.track - TSHOWN + 1;
    U.tleft = clampi(U.tleft, 0, NTRACKS - TSHOWN);
    snprintf(buf, sizeof buf, narrow ? "%02d" : "pat: %02d", U.pat);    /* beside the tree, track 1 starts at 20 */
    draw_text(buf, 2, HEAD_Y, COL_YELLOW);
    for (int k = 0; k < TSHOWN; k++) {
        int t = U.tleft + k;
        snprintf(buf, sizeof buf, "track %d", t + 1);
        draw_text(buf, GX + k * TW, HEAD_Y, t == U.track ? COL_WHITE : UI_RGB(1, 2, 2));
    }
    if (U.tleft > 0) draw_char('<', GX - 8, HEAD_Y, COL_YELLOW);
    if (U.tleft + TSHOWN < NTRACKS) draw_char('>', UI_W - 6, HEAD_Y, COL_YELLOW);
    for (int r = 0; r < ROWS_SHOWN; r++) {
        int row = U.top + r, y = GRID_Y + r * FONT_H;
        if (row >= PATTERN_ROWS) break;
        bool playing = mpat == U.pat && row == mrow;
        int bx = ROWNUM_X - 2;
        if (playing) draw_rect(bx, y, UI_W - bx, FONT_H, UI_RGB(1, 1, 0), true);
        else if (row == U.row) draw_rect(bx, y, UI_W - bx, FONT_H, UI_RGB(0, 0, 1), true);
        snprintf(buf, sizeof buf, "%02d", row);
        draw_text(buf, ROWNUM_X, y, row % 8 == 0 ? COL_YELLOW : row % 4 == 0 ? UI_RGB(2, 2, 3) : UI_RGB(1, 1, 2));
        for (int k = 0; k < TSHOWN; k++) {
            int t = U.tleft + k;
            const Cell *c = &cur_song->patterns[U.pat].cells[row][t];
            int x = GX + k * TW;
            if (row == U.row && t == U.track) {
                int fx0 = U.field == F_NOTE ? 0 : U.field + 3, fw = U.field == F_NOTE ? 3 : 1;
                draw_rect(x + fx0 * FONT_W - 1, y, fw * FONT_W + 1, FONT_H, UI_RGB(1, 2, 3), true);
            }
            if (c->kind == CELL_EMPTY) { draw_text("... ...", x, y, UI_RGB(0, 1, 2)); continue; }
            if (c->kind == CELL_OFF) { draw_text("=== ...", x, y, COL_RED); draw_text("...", x + 4 * FONT_W, y, UI_RGB(0, 1, 2)); continue; }
            char name[4];
            pitch_name(c->pitch, name);
            draw_text(name, x, y, COL_WHITE);
            draw_char(c->wave >= NWAVES ? 'n' : "0123456789abcdef"[c->wave], x + 4 * FONT_W, y, COL_CYAN);
            draw_char('0' + c->vol, x + 5 * FONT_W, y, UI_RGB(3, 1, 2));
            draw_char(fx_chars[c->fx % NFX], x + 6 * FONT_W, y, COL_ORANGE);
        }
    }
    for (int t = 1; t < TSHOWN; t++) draw_line(GX + t * TW - 2, HEAD_Y, GX + t * TW - 2, GRID_Y + ROWS_SHOWN * FONT_H - 1, UI_RGB(0, 1, 1));
}

static bool black_key(int p) { int s = p % 12; return s == 1 || s == 3 || s == 6 || s == 8 || s == 10; }

/* the piano roll: the rows across, pitch up the screen, one track's notes as bars */
static void draw_roll(int mpat, int mrow) {
    char buf[64];
    int cw = CW, gw = PATTERN_ROWS * cw;
    int hover = over_roll() || over_keys() ? roll_pitch() : -1;

    /* header: which track, the pens new notes take */
    snprintf(buf, sizeof buf, "pat: %02d", U.pat);
    draw_text(buf, 2, HEAD_Y, COL_YELLOW);
    draw_text("trk", TRK_X, HEAD_Y, UI_RGB(1, 2, 2));
    for (int t = 0; t < NTRACKS; t++) {
        int x = TRK_X + 20 + t * 10;
        if (t == U.track) draw_rect(x - 1, HEAD_Y, 9, 10, UI_RGB(1, 2, 3), true);
        draw_char('1' + t, x + 1, HEAD_Y, t == U.track ? COL_BLACK : UI_RGB(2, 2, 3));
    }
    snprintf(buf, sizeof buf, "wave - %c +", U.pen_wave >= NWAVES ? 'n' : "0123456789abcdef"[U.pen_wave]);
    draw_text(buf, PEN_X, HEAD_Y, COL_CYAN);
    snprintf(buf, sizeof buf, "vol - %d +", U.pen_vol);
    draw_text(buf, VOLPEN_X, HEAD_Y, UI_RGB(3, 1, 2));

    /* the lanes: dark for black keys, a line where each octave starts */
    for (int i = 0; i < PSHOWN; i++) {
        int p = U.plow + i, y = ROLL_BOTTOM - (i + 1) * PH;
        draw_rect(RX, y, gw, PH, p == hover ? UI_RGB(0, 1, 1) : black_key(p) ? COL_BLACK : UI_RGB(0, 0, 1), true);
        if (p % 12 == 0) draw_line(RX, y + PH - 1, RX + gw - 1, y + PH - 1, UI_RGB(0, 1, 2));
    }
    for (int r = 4; r < PATTERN_ROWS; r += 4)                  /* beats: a line every 4 rows, brighter every 8 */
        draw_line(RX + r * cw, GRID_Y, RX + r * cw, ROLL_BOTTOM - 1, r % 8 ? UI_RGB(0, 1, 1) : UI_RGB(0, 1, 2));
    if (mpat == U.pat && mrow >= 0) draw_rect(RX + mrow * cw, GRID_Y, cw, PSHOWN * PH, UI_RGB(1, 1, 0), true);
    draw_rect(RX + U.row * cw, GRID_Y, cw, PSHOWN * PH, UI_RGB(1, 2, 3), false);

    /* the other tracks, faint; then this one */
    for (int pass = 0; pass < 2; pass++)
        for (int t = 0; t < NTRACKS; t++) {
            if ((t == U.track) != (pass == 1)) continue;
            for (int s = 0; s < PATTERN_ROWS; s++) {
                const Cell *c = at(s, t);
                if (c->kind != CELL_NOTE) continue;
                int e = note_end(t, s), x = RX + s * cw, w = (e - s) * cw - 1;
                int i = c->pitch - U.plow;
                if (pass == 0) {
                    if (i >= 0 && i < PSHOWN) draw_rect(x, ROLL_BOTTOM - (i + 1) * PH + 2, w, 2, UI_RGB(1, 1, 2), true);
                    continue;
                }
                if (i < 0 || i >= PSHOWN) {                 /* out of view: a mark at the edge it's past */
                    draw_rect(x, i < 0 ? ROLL_BOTTOM - 2 : GRID_Y, w, 2, COL_ORANGE, true);
                    continue;
                }
                int y = ROLL_BOTTOM - (i + 1) * PH;
                bool sounding = mpat == U.pat && mrow >= s && mrow < e;
                draw_rect(x, y, w, PH - 1, sounding ? COL_YELLOW : UI_RGB(1 + c->vol / 3, 2 + c->vol / 4, 3), true);
                draw_rect(x, y, 2, PH - 1, COL_WHITE, true);
            }
        }

    /* the keyboard, octave names beside each C */
    draw_rect(KEYS_X, GRID_Y, RX - 2 - KEYS_X, PSHOWN * PH, COL_WHITE, true);
    for (int i = 0; i < PSHOWN; i++) {
        int p = U.plow + i, y = ROLL_BOTTOM - (i + 1) * PH;
        if (black_key(p)) draw_rect(KEYS_X, y, 12, PH, p == hover ? COL_ORANGE : COL_BLACK, true);
        else {
            if (p == hover) draw_rect(KEYS_X, y, RX - 2 - KEYS_X, PH, COL_YELLOW, true);
            if (p % 12 == 0 || p % 12 == 5) draw_line(KEYS_X, y + PH - 1, RX - 3, y + PH - 1, COL_GREY);   /* under C and F */
        }
        if (p % 12 == 0) {
            snprintf(buf, sizeof buf, "C%d", p / 12 + 2);
            draw_text(buf, KEYS_X - 13, y - 2, p == hover ? COL_YELLOW : UI_RGB(2, 2, 3));
        }
    }
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    char buf[128];
    int mpos = synth_music_pos(), mpat = synth_music_pattern(), mrow = synth_music_row();
    if (mpos >= 0) { U.opos = mpos; U.pat = mpat; }          /* follow the song while it plays */

    /* song: speed and the order list */
    snprintf(buf, sizeof buf, "speed - %-2d +", cur_song->song_speed);
    draw_text(buf, 2, SONG_Y, UI_RGB(2, 3, 3));
    draw_text("order", ORDER_LABEL_X, SONG_Y, UI_RGB(2, 2, 3));
    snprintf(buf, sizeof buf, "%02d", U.opos);
    draw_text(buf, ORDER_IDX_X, SONG_Y, COL_YELLOW);
    bool hot_prev = inside(ORDER_PREV_X - 1, SONG_Y, 8, 10), hot_next = inside(ORDER_NEXT_X - 1, SONG_Y, 8, 10);
    draw_char('<', ORDER_PREV_X, SONG_Y, U.opos > 0 ? (hot_prev ? COL_YELLOW : COL_WHITE) : UI_RGB(1, 1, 2));
    draw_char('>', ORDER_NEXT_X, SONG_Y, U.opos < ORDER_LEN - 1 ? (hot_next ? COL_YELLOW : COL_WHITE) : UI_RGB(1, 1, 2));
    if (U.opos < U.oscroll) U.oscroll = U.opos;
    if (U.opos >= U.oscroll + ORDER_SHOWN) U.oscroll = U.opos - ORDER_SHOWN + 1;
    for (int k = 0; k < ORDER_SHOWN; k++) {
        int i = U.oscroll + k;
        if (i >= ORDER_LEN) break;
        const OrderEntry *e = &cur_song->order[i];
        int x = ORDER_X + k * ORDER_W;
        if (i == U.opos) draw_rect(x, SONG_Y, ORDER_W - 1, 10, UI_RGB(1, 2, 3), true);
        if (e->kind == ORDER_PATTERN) snprintf(buf, sizeof buf, "%02d", e->value);
        else if (e->kind == ORDER_JUMP) snprintf(buf, sizeof buf, ">%d", e->value);
        else snprintf(buf, sizeof buf, " .");
        int c = i == U.opos ? COL_BLACK : i == mpos ? COL_YELLOW : e->kind == ORDER_STOP ? UI_RGB(1, 1, 2) : e->kind == ORDER_JUMP ? COL_ORANGE : COL_WHITE;
        draw_text(buf, x + 3, SONG_Y, c);
    }

    /* the pattern list */
    if (!narrow) draw_rect(0, GRID_Y, LIST_X + LIST_W, (NPATTERNS / 2) * LIST_ROW, UI_RGB(0, 0, 1), true);
    for (int p = 0; p < NPATTERNS && !narrow; p++) {
        int lx = LIST_X + (p % 2) * LIST_COL, ly = GRID_Y + (p / 2) * LIST_ROW;
        if (p == U.pat) draw_rect(lx - 1, ly, LIST_COL, LIST_ROW, UI_RGB(1, 2, 3), true);
        snprintf(buf, sizeof buf, "%02d", p);
        int c = p == U.pat ? COL_BLACK : p == mpat ? COL_YELLOW : pattern_used(p) ? COL_WHITE : UI_RGB(1, 1, 2);
        draw_text(buf, lx, ly, c);
    }

    if (U.roll) draw_roll(mpat, mrow);
    else draw_grid(mpos, mpat, mrow);

    /* bars */
    ui_draw_tabs();
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
    static const char *const field_names[4] = {"note", "wave", "volume", "effect"};
    if (U.roll) {                     /* where the mouse is: the row and the note it would draw */
        char name[4];
        bool over = over_roll() || over_keys();
        pitch_name(over ? roll_pitch() : 0, name);
        snprintf(buf, sizeof buf, "pat %d trk %d row %d %s  oct %d", U.pat, U.track + 1, over_roll() ? roll_row() : U.row,
                 over ? name : "   ", U.octave + 2);
    } else snprintf(buf, sizeof buf, "pat %d row %d trk %d %s  oct %d", U.pat, U.row, U.track + 1, field_names[U.field], U.octave + 2);
    int bx = draw_text(buf, 3, by, UI_RGB(2, 3, 3));
    if (U.status[0] && frame_count < U.status_until) {
        draw_text(U.status, bx + 10, by, U.status_color);
        if (ui_status_close(by)) U.status[0] = 0;
    }
    int vx = view_btn_x();
    bool hot = inside(vx, by, VIEW_BTN_W, FONT_H);
    draw_rect(vx, by, VIEW_BTN_W, FONT_H, hot ? COL_YELLOW : UI_RGB(1, 2, 3), true);
    draw_text(U.roll ? "grid" : "roll", vx + 5, by, COL_BLACK);
    draw_text(doc_label(), UI_W - STATUS_CLOSE_W - 2 - text_width(doc_label()), by, UI_RGB(2, 3, 3));
}

/* the tree opened another song: its undo history doesn't apply */
void music_doc_changed(void) {
    U.nundo = U.nredo = 0;
    U.opos = U.pat = U.row = U.top = 0;
}

void music_frame(void) {
    init();
    layout();
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
