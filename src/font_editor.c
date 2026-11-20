/* The built-in font editor (F7).
 *
 *   +---------------------------+----------------+
 *   | the 96 glyphs             | the glyph      |
 *   | (scrolls for big fonts)   | being edited,  |
 *   |                           | big            |
 *   +---------------------------+----------------+
 *   | sample text in the font being edited       |
 *
 * A font's characters are all one size, w x h, from 4 x 4 to 20 x 20
 * (Ctrl+arrows change it). A glyph fills its whole cell, so the space
 * between characters is part of the glyph: leave a column or a row blank
 * for spacing, or draw into it. The left button sets pixels and the right
 * button clears them.
 *
 * Typing a character picks its glyph, so every action is on a key that
 * doesn't type: arrows, Shift+arrows, Del, and Ctrl. A new cart's font is a
 * copy of the built-in one (sys_font, 5 x 10); Ctrl+D puts one glyph back
 * to it. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOP FONT_H
#define BOT (SCREEN_H - FONT_H)
#define AREA_Y (TOP + 3)              /* the grid and the canvas */
#define AREA_H 126
#define SAMPLE_Y (AREA_Y + AREA_H + 4)
#define CANVAS_BOX_W 100              /* the canvas fits in this */
#define CANVAS_BOX_H AREA_H
#define MAX_UNDO 64

#define F (cur_font)

static struct {
    bool ready;
    int sel;                   /* the glyph being edited, 0-95 (character sel + 32) */
    int scroll;                /* the grid's first row shown */
    int paint;                 /* a stroke: 0 none, 1 setting pixels, 2 clearing them */
    int paint_btn, lastx, lasty;
    Font undo[MAX_UNDO], redo[MAX_UNDO];
    int nundo, nredo;
    uint32_t clip[FONT_MAX];
    bool has_clip;
    char status[128];
    int status_color;
    long status_until;
} E;

/* where things are this frame: they depend on the font's size and the editor's width */
static struct { int s, cx, cy; int z, cols, rows, shown, cw, ch; } L;

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.status, sizeof E.status, fmt, ap);
    va_end(ap);
    E.status_color = color;
    E.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[font] %s\n", E.status);
}

static void init(void) {
    if (E.ready) return;
    E.ready = true;
    E.sel = 'a' - FONT_FIRST;
}

static void layout(void) {
    /* the canvas: the biggest whole scale that fits its box, at the right */
    int s = CANVAS_BOX_W / F->w;
    if (CANVAS_BOX_H / F->h < s) s = CANVAS_BOX_H / F->h;
    if (s > 16) s = 16;
    if (s < 1) s = 1;
    L.s = s;
    L.cx = UI_W - 3 - F->w * s;
    L.cy = AREA_Y;
    /* the grid: twice the size if 16 or 8 to a row fit beside the canvas, else as is */
    int room = L.cx - 6 - 3;
    L.z = 1;
    L.cols = 8;
    static const int try_z[] = {2, 2, 1, 1}, try_cols[] = {16, 8, 16, 8};
    for (int i = 0; i < 4; i++)
        if (try_cols[i] * (F->w * try_z[i] + 1) <= room) { L.z = try_z[i]; L.cols = try_cols[i]; break; }
    L.cw = F->w * L.z + 1;
    L.ch = F->h * L.z + 1;
    L.rows = NGLYPHS / L.cols;
    L.shown = AREA_H / L.ch;
    if (L.shown < 1) L.shown = 1;
    if (L.shown > L.rows) L.shown = L.rows;
    int row = E.sel / L.cols;                    /* keep the glyph being edited in view */
    if (row < E.scroll) E.scroll = row;
    if (row >= E.scroll + L.shown) E.scroll = row - L.shown + 1;
    if (E.scroll > L.rows - L.shown) E.scroll = L.rows - L.shown;
    if (E.scroll < 0) E.scroll = 0;
}

/* ---------------------------------------------------------------- undo */

static void before_change(void) {
    if (E.nundo == MAX_UNDO) { memmove(E.undo, E.undo + 1, sizeof E.undo[0] * (MAX_UNDO - 1)); E.nundo--; }
    E.undo[E.nundo++] = *F;
    E.nredo = 0;
    doc_touch(RES_FONT);
}

static void undo_redo(bool redo) {
    Font *from = redo ? E.redo : E.undo, *to = redo ? E.undo : E.redo;
    int *nfrom = redo ? &E.nredo : &E.nundo, *nto = redo ? &E.nundo : &E.nredo;
    if (!*nfrom) { status(COL_GREY, "nothing to %s", redo ? "redo" : "undo"); return; }
    if (*nto == MAX_UNDO) { memmove(to, to + 1, sizeof to[0] * (MAX_UNDO - 1)); (*nto)--; }
    to[(*nto)++] = *F;
    *F = from[--*nfrom];
    doc_touch(RES_FONT);
}

/* the tree opened another font: its undo history doesn't apply */
void font_doc_changed(void) { E.nundo = E.nredo = 0; E.paint = 0; }

/* ---------------------------------------------------------------- glyph pixels */

static bool gget(int x, int y) { return F->g[E.sel][y] >> x & 1; }
static void gset(int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= F->w || y >= F->h) return;
    if (on) F->g[E.sel][y] |= 1u << x;
    else F->g[E.sel][y] &= ~(1u << x);
}

/* a stroke between two frames' mouse positions: every pixel on the way, so a fast drag leaves no gaps */
static void gline(int x0, int y0, int x1, int y1, bool on) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
    for (;;) {
        gset(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static uint32_t row_mask(void) { return (1u << F->w) - 1; }

/* move the glyph's pixels one step, wrapping around inside the cell */
static void nudge(int dx, int dy) {
    uint32_t *g = F->g[E.sel], t[FONT_MAX], m = row_mask();
    int w = F->w, h = F->h;
    for (int y = 0; y < h; y++) {
        uint32_t row = g[(y - dy + h) % h] & m;
        if (dx > 0) row = ((row << 1) | (row >> (w - 1))) & m;
        if (dx < 0) row = ((row >> 1) | (row << (w - 1))) & m;
        t[y] = row;
    }
    memcpy(g, t, sizeof g[0] * (size_t)h);
}

/* a new size for every character. Pixels outside it go, so what you see is what's saved */
static void resize(int dw, int dh) {
    int w = F->w + dw, h = F->h + dh;
    if (w < FONT_MIN || h < FONT_MIN || w > FONT_MAX || h > FONT_MAX) {
        status(COL_GREY, "a font is %d x %d to %d x %d", FONT_MIN, FONT_MIN, FONT_MAX, FONT_MAX);
        return;
    }
    before_change();
    F->w = (uint8_t)w;
    F->h = (uint8_t)h;
    for (int g = 0; g < NGLYPHS; g++)
        for (int r = 0; r < FONT_MAX; r++) F->g[g][r] = r < h ? F->g[g][r] & row_mask() : 0;
    status(COL_GREY, "characters are %d x %d", w, h);
}

static const char *char_name(int c, char *buf, size_t n) {
    if (c == ' ') snprintf(buf, n, "space");
    else if (c == 127) snprintf(buf, n, "127 (block)");
    else snprintf(buf, n, "%d '%c'", c, c);
    return buf;
}

/* ---------------------------------------------------------------- keys */

static void key(const KeyEvent *ev) {
    int sc = ev->sc;
    bool ctrl = ev->mod & MOD_CTRL, shift = ev->mod & MOD_SHIFT;
    size_t rows = sizeof E.clip[0] * (size_t)F->h;
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: {
                char err[256];
                Doc *d = doc_active(RES_FONT);
                if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved fonts/%s.g64", d->name);
                else status(COL_RED, "%s", err);
                return;
            }
            case SDL_SCANCODE_R: ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: undo_redo(shift); return;
            case SDL_SCANCODE_Y: undo_redo(true); return;
            case SDL_SCANCODE_C: case SDL_SCANCODE_X:
                memset(E.clip, 0, sizeof E.clip);
                memcpy(E.clip, F->g[E.sel], rows);
                E.has_clip = true;
                if (sc == SDL_SCANCODE_X) { before_change(); memset(F->g[E.sel], 0, rows); }
                status(COL_GREY, "%s the glyph", sc == SDL_SCANCODE_X ? "cut" : "copied");
                return;
            case SDL_SCANCODE_V:
                if (!E.has_clip) { status(COL_GREY, "nothing copied yet"); return; }
                before_change();
                for (int r = 0; r < F->h; r++) F->g[E.sel][r] = E.clip[r] & row_mask();
                return;
            case SDL_SCANCODE_D:          /* the built-in glyph, at the top left of the cell */
                before_change();
                memset(F->g[E.sel], 0, rows);
                for (int r = 0; r < F->h && r < sys_font.h; r++) F->g[E.sel][r] = sys_font.g[E.sel][r] & row_mask();
                status(COL_GREY, "the built-in glyph");
                return;
            case SDL_SCANCODE_LEFT: resize(-1, 0); return;
            case SDL_SCANCODE_RIGHT: resize(1, 0); return;
            case SDL_SCANCODE_UP: resize(0, -1); return;
            case SDL_SCANCODE_DOWN: resize(0, 1); return;
            default: return;
        }
    }
    int dx = 0, dy = 0;
    switch (sc) {
        case SDL_SCANCODE_ESCAPE: ui_switch(MODE_SHELL); return;
        case SDL_SCANCODE_DELETE: case SDL_SCANCODE_BACKSPACE: before_change(); memset(F->g[E.sel], 0, rows); return;
        case SDL_SCANCODE_LEFT: dx = -1; break;
        case SDL_SCANCODE_RIGHT: dx = 1; break;
        case SDL_SCANCODE_UP: dy = -1; break;
        case SDL_SCANCODE_DOWN: dy = 1; break;
        case SDL_SCANCODE_PAGEUP: dy = -L.shown; break;
        case SDL_SCANCODE_PAGEDOWN: dy = L.shown; break;
        default: return;
    }
    if (shift && sc != SDL_SCANCODE_PAGEUP && sc != SDL_SCANCODE_PAGEDOWN) {
        before_change();
        nudge(dx, dy);
        return;
    }
    int col = E.sel % L.cols + dx, row = E.sel / L.cols + dy;
    if (col < 0) col = 0;
    if (col >= L.cols) col = L.cols - 1;
    if (row < 0) row = 0;
    if (row >= L.rows) row = L.rows - 1;
    E.sel = row * L.cols + col;
}

/* typing a character picks its glyph */
static void typed(const char *text) {
    for (const char *p = text; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (c >= FONT_FIRST && c < FONT_FIRST + NGLYPHS) E.sel = c - FONT_FIRST;
    }
}

/* ---------------------------------------------------------------- mouse */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

/* the glyph pixel under the mouse (anywhere: it may be off the glyph) */
static void mouse_px(int *x, int *y) {
    int mx = in.mouse_x - L.cx, my = in.mouse_y - L.cy;
    *x = mx < 0 ? -1 : mx / L.s;
    *y = my < 0 ? -1 : my / L.s;
}

/* true if the mouse is on the glyph's cell */
static bool canvas_px(int *x, int *y) {
    mouse_px(x, y);
    return *x >= 0 && *y >= 0 && *x < F->w && *y < F->h;
}

static int grid_glyph_at(void) {
    if (!inside(3, AREA_Y, L.cols * L.cw, L.shown * L.ch)) return -1;
    return ((in.mouse_y - AREA_Y) / L.ch + E.scroll) * L.cols + (in.mouse_x - 3) / L.cw;
}

static void mouse(void) {
    if (E.paint) {
        if (!in.mouse[E.paint_btn] || in.mouse_hit[E.paint_btn]) E.paint = 0;
        else {
            int x, y;
            mouse_px(&x, &y);
            if (x > F->w) x = F->w;                  /* just past an edge is as far as it needs to go */
            if (y > F->h) y = F->h;
            gline(E.lastx, E.lasty, x, y, E.paint == 1);   /* gset leaves out what's off the glyph */
            E.lastx = x;
            E.lasty = y;
            return;
        }
    }
    if (in.wheel && inside(3, AREA_Y, L.cols * L.cw, L.shown * L.ch)) {      /* scroll the grid */
        E.scroll -= in.wheel > 0 ? 1 : -1;
        if (E.scroll < 0) E.scroll = 0;
        if (E.scroll > L.rows - L.shown) E.scroll = L.rows - L.shown;
        int row = E.sel / L.cols;                     /* the glyph being edited moves into view */
        if (row < E.scroll) E.sel += (E.scroll - row) * L.cols;
        if (row >= E.scroll + L.shown) E.sel -= (row - E.scroll - L.shown + 1) * L.cols;
    }
    int btn = in.mouse_hit[1] ? 1 : in.mouse_hit[2] ? 2 : 0;
    if (!btn || in.mouse_y < TOP || in.mouse_y >= BOT) return;
    int g = grid_glyph_at(), x, y;
    if (g >= 0 && g < NGLYPHS) { E.sel = g; return; }
    if (canvas_px(&x, &y)) {
        before_change();
        E.paint = btn;
        E.paint_btn = btn;
        E.lastx = x;
        E.lasty = y;
        gset(x, y, btn == 1);
    }
}

/* ---------------------------------------------------------------- drawing */

/* a glyph of the font being edited, each pixel s x s */
static void draw_glyph_big(int g, int x, int y, int s, int c) {
    for (int r = 0; r < F->h; r++)
        for (int k = 0; k < F->w; k++)
            if (F->g[g][r] >> k & 1) draw_rect(x + k * s, y + r * s, s, s, c, true);
}

static void draw_grid(void) {
    draw_clip(0, AREA_Y, L.cx - 3, L.shown * L.ch);
    for (int i = 0; i < L.shown * L.cols; i++) {
        int g = E.scroll * L.cols + i;
        if (g >= NGLYPHS) break;
        int x = 3 + (i % L.cols) * L.cw, y = AREA_Y + (i / L.cols) * L.ch;
        bool on = g == E.sel;
        draw_rect(x, y, F->w * L.z, F->h * L.z, on ? UI_RGB(1, 2, 3) : UI_RGB(0, 0, 1), true);
        draw_glyph_big(g, x, y, L.z, on ? COL_BLACK : COL_WHITE);
    }
    int g = grid_glyph_at();
    if (g >= 0 && g < NGLYPHS) {
        int i = g - E.scroll * L.cols;
        int x = 3 + (i % L.cols) * L.cw, y = AREA_Y + (i / L.cols) * L.ch;
        draw_rect(x - 1, y - 1, F->w * L.z + 2, F->h * L.z + 2, COL_YELLOW, false);
    }
    draw_clip_reset();
    if (L.rows > L.shown) {                       /* a scroll bar */
        int bx = 3 + L.cols * L.cw + 1, bh = L.shown * L.ch;
        if (bx < L.cx - 4) {
            draw_rect(bx, AREA_Y, 2, bh, UI_RGB(0, 0, 1), true);
            draw_rect(bx, AREA_Y + bh * E.scroll / L.rows, 2, bh * L.shown / L.rows, UI_RGB(1, 2, 3), true);
        }
    }
}

static void draw_canvas(int *hx, int *hy) {
    int s = L.s, gap = s >= 4 ? 1 : 0;
    draw_rect(L.cx - 1, L.cy - 1, F->w * s + 1, F->h * s + 1, UI_RGB(1, 2, 3), false);
    for (int y = 0; y < F->h; y++)
        for (int x = 0; x < F->w; x++)
            draw_rect(L.cx + x * s, L.cy + y * s, s - gap, s - gap, gget(x, y) ? COL_WHITE : UI_RGB(0, 1, 2), true);
    *hx = *hy = -1;
    if (canvas_px(hx, hy)) draw_rect(L.cx + *hx * s - 1, L.cy + *hy * s - 1, s + 1, s + 1, COL_YELLOW, false);
    else *hx = *hy = -1;
    char size[24];
    snprintf(size, sizeof size, "%d x %d", F->w, F->h);
    int ty = L.cy + F->h * s + 2;
    if (ty + FONT_H <= SAMPLE_Y) draw_text(size, L.cx, ty, UI_RGB(1, 2, 2));
}

static void draw_sample(void) {
    static const char *const lines[] = {
        "the quick brown fox jumps over the lazy dog.",
        "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!",
        "0123456789 +-*/=<> ()[]{} #$%&@ ~^_`|\\:;'\",.?",
        "if btn(BTN_A) then text(\"hi\", 8, 8, COL_RED)",
    };
    draw_clip(0, SAMPLE_Y - 2, UI_W, BOT - SAMPLE_Y + 2);
    ds.font = F;
    int y = SAMPLE_Y;
    for (size_t i = 0; i < sizeof lines / sizeof lines[0] && y < BOT; i++, y += F->h)
        draw_text(lines[i], 3, y, i == 3 ? UI_RGB(2, 3, 3) : COL_WHITE);
    ds.font = NULL;
    /* "goober FONT" at twice the size, if there's room */
    const char *big = "goober FONT";
    if (y + 2 + F->h * 2 <= BOT)
        for (int i = 0; big[i]; i++) draw_glyph_big(big[i] - FONT_FIRST, 3 + i * F->w * 2, y + 2, 2, COL_YELLOW);
    draw_clip_reset();
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    draw_grid();
    int hx, hy;
    draw_canvas(&hx, &hy);
    draw_sample();

    ui_draw_tabs();
    int by = BOT;
    draw_rect(0, by, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
    Doc *d = doc_active(RES_FONT);
    char info[64], line[160], cn[24];
    snprintf(info, sizeof info, "%s%s", d->name, d->dirty ? "*" : "");
    int x = draw_text(info, 3, by, UI_RGB(2, 3, 3)) + 8;
    int g = grid_glyph_at();
    if (E.status[0] && frame_count < E.status_until) draw_text(E.status, x, by, E.status_color);
    else if (hx >= 0) {
        snprintf(line, sizeof line, "%s  x%d y%d", char_name(E.sel + FONT_FIRST, cn, sizeof cn), hx, hy);
        draw_text(line, x, by, UI_RGB(2, 3, 3));
    } else if (g >= 0 && g < NGLYPHS) draw_text(char_name(g + FONT_FIRST, cn, sizeof cn), x, by, UI_RGB(2, 3, 3));
    else {
        snprintf(line, sizeof line, "%s  type: pick  ^arrows: size  ^D: built-in", char_name(E.sel + FONT_FIRST, cn, sizeof cn));
        draw_text(line, x, by, UI_RGB(1, 2, 2));
    }
}

void font_frame(void) {
    init();
    if (!cart.loaded || !F) { ui_switch(MODE_SHELL); return; }
    layout();
    for (int i = 0; i < in.nevents && ui_mode == MODE_FONT; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type == EV_TEXT) { if (!(ev->mod & MOD_CTRL)) typed(ev->text); continue; }
        if (ev->type != EV_KEY) continue;
        if (ui_global_key(ev)) continue;
        key(ev);
        layout();                 /* a new size or glyph can move things */
    }
    if (ui_mode != MODE_FONT) return;
    mouse();
    layout();
    draw();
}
