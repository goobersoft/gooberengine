/* The built-in tile editor (F2).
 *
 * A tileset is 400 x 240 pixels (960 tiles), more than fits beside the file
 * tree, so the editor works like a paint program:
 *
 *   - the whole tileset is the background. Middle-drag (or Space + drag) pans
 *     it, the wheel zooms it around the mouse, and a click picks a tile.
 *   - the selected tile (or a 2x2 / 4x4 block) is drawn big in its own window.
 *     Paint there. Drag the window by its title bar; drag its bottom-right
 *     corner to resize it.
 *   - the tools, actions and the 64 colours are in another window, which can
 *     also be dragged around.
 *
 * Transparent pixels show as a pink checkerboard (colours 50 and 34). The
 * left button draws with colour A, the right button with colour B
 * (transparent at first, so it erases), and the middle button picks a colour
 * up. Edits go straight into the open tileset, so Ctrl+R runs them without
 * saving; Ctrl+S writes tiles/NAME.g64. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOP FONT_H                  /* the background: between the bars */
#define BOT (SCREEN_H - FONT_H)
#define TITLE_H 9
#define MAX_UNDO 64
#define CHECK_A 50                  /* the transparent checkerboard */
#define CHECK_B 34
#define TOOLS_W 70
#define TOOLS_H 116
#define GRIP 6

enum { T_PENCIL, T_FILL, T_LINE, T_RECT, T_PICK, NTOOLS };
static const char tool_keys[NTOOLS] = {'P', 'F', 'L', 'R', 'I'};
static const char *const tool_names[NTOOLS] = {"pencil (P)", "fill (F)", "line (L)", "rect (R), Shift: filled", "pick a colour (I)"};
enum { A_SIZE, A_FLIPX, A_FLIPY, A_TURN, A_CLEAR, NACTS };
static const char act_keys[NACTS] = {'Z', 'H', 'V', 'T', 'X'};
static const char *const act_names[NACTS] = {"block size 1x1 / 2x2 / 4x4 (Z)", "flip across (H)", "flip up-down (V)", "turn clockwise (T)", "clear the block (Del)"};
static const int zooms[] = {1, 2, 3, 4, 6, 8};
#define NZOOM 6

/* a floating window: (x, y) is its top-left, w x h its content below the title bar */
typedef struct { int x, y, w, h; } Win;
enum { W_TILE, W_TOOLS, NWIN };

static struct {
    bool ready;
    int sel;                 /* top-left tile of the block being edited */
    int bsize;               /* block size in tiles: 1, 2 or 4 */
    int tool, prev_tool;
    int col_a, col_b;        /* 0..63 or CLEAR */
    bool grid;
    int zoom, ox, oy;        /* the background: zoom index, where the sheet's top-left is */
    Win win[NWIN];
    int top;                 /* the window drawn last (on top) */
    bool placed;             /* windows put in their starting places */
    int placed_w;            /* the editor's width then: the windows keep to its right edge */
    int drag;                /* 0 none, 1 window move, 2 window resize, 3 pan, 4 paint */
    int drag_win, drag_dx, drag_dy, pan_x, pan_y;
    int stroke_btn, x0, y0, lastx, lasty;
    Tileset *base;           /* the tileset when a line/rect drag started */
    Tileset *undo[MAX_UNDO], *redo[MAX_UNDO];
    int nundo, nredo;
    uint8_t clip[4 * TILE * 4 * TILE];
    int clip_size;           /* 0 = empty, else block size in tiles */
    char status[128];
    int status_color;
    long status_until;
    const char *hint;        /* what the thing under the mouse does */
} T;

#define TS (cur_tiles)

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(T.status, sizeof T.status, fmt, ap);
    va_end(ap);
    T.status_color = color;
    T.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[tiles] %s\n", T.status);
}

static void init(void) {
    if (T.ready) return;
    T.ready = true;
    T.sel = 1;
    T.bsize = 1;
    T.col_a = COL_WHITE;
    T.col_b = CLEAR;
    T.grid = true;
    T.zoom = 1;
    T.ox = 0;
    T.oy = TOP;
    T.top = W_TOOLS;
}

/* windows start at the right of the editor, the tools right-most */
static void place_windows(void) {
    T.win[W_TOOLS] = (Win){UI_W - TOOLS_W - 2, TOP + 2, TOOLS_W, TOOLS_H};
    T.win[W_TILE] = (Win){UI_W - TOOLS_W - 2 - 126, TOP + 2, 122, 122};
    T.placed = true;
    T.placed_w = UI_W;
}

static void keep_windows_inside(void) {
    if (UI_W != T.placed_w) {        /* the file tree opened or closed: move with the right edge */
        for (int i = 0; i < NWIN; i++) T.win[i].x += UI_W - T.placed_w;
        T.placed_w = UI_W;
    }
    for (int i = 0; i < NWIN; i++) {
        Win *w = &T.win[i];
        if (w->w > UI_W - 2) w->w = UI_W - 2;
        if (w->x + w->w > UI_W) w->x = UI_W - w->w;
        if (w->x < 0) w->x = 0;
        if (w->y + TITLE_H > BOT) w->y = BOT - TITLE_H;
        if (w->y < TOP) w->y = TOP;
    }
}

/* ---------------------------------------------------------------- block pixels */

static int bpx(void) { return T.bsize * TILE; }

static void clamp_sel(void) {
    int col = T.sel % SHEET_COLS, row = T.sel / SHEET_COLS;
    int maxc = SHEET_COLS - T.bsize, maxr = SHEET_ROWS - T.bsize;
    if (col > maxc) col = maxc;
    if (row > maxr) row = maxr;
    if (col < 0) col = 0;
    if (row < 0) row = 0;
    T.sel = row * SHEET_COLS + col;
}

static uint8_t *bpix(int x, int y) {
    int t = T.sel + (y / TILE) * SHEET_COLS + x / TILE;
    return &TS->t[t][(y % TILE) * TILE + x % TILE];
}

static int bget(int x, int y) {
    if (x < 0 || y < 0 || x >= bpx() || y >= bpx()) return -1;
    return *bpix(x, y);
}

static void bset(int x, int y, int c) {
    if (x < 0 || y < 0 || x >= bpx() || y >= bpx()) return;
    *bpix(x, y) = (uint8_t)c;
}

static void bline(int x0, int y0, int x1, int y1, int c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
    for (;;) {
        bset(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void brect(int x0, int y0, int x1, int y1, int c, bool fill) {
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            if (fill || y == y0 || y == y1 || x == x0 || x == x1) bset(x, y, c);
}

static void bfill(int x, int y, int c) {
    int from = bget(x, y);
    if (from < 0 || from == c) return;
    static int stack[4 * TILE * 4 * TILE * 4][2];
    int n = 0;
    stack[n][0] = x; stack[n][1] = y; n++;
    while (n > 0) {
        n--;
        int px = stack[n][0], py = stack[n][1];
        if (bget(px, py) != from) continue;
        bset(px, py, c);
        static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int k = 0; k < 4; k++) {
            int qx = px + d[k][0], qy = py + d[k][1];
            if (bget(qx, qy) == from && n < (int)(sizeof stack / sizeof stack[0])) {
                stack[n][0] = qx; stack[n][1] = qy; n++;
            }
        }
    }
}

/* whole-block transforms: read the block out, write it back moved */
static void transform(int kind, int dx, int dy) {
    int n = bpx();
    static uint8_t tmp[4 * TILE][4 * TILE];
    for (int y = 0; y < n; y++) for (int x = 0; x < n; x++) tmp[y][x] = (uint8_t)bget(x, y);
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            int sx = x, sy = y;
            if (kind == 'h') sx = n - 1 - x;
            else if (kind == 'v') sy = n - 1 - y;
            else if (kind == 't') { sx = y; sy = n - 1 - x; }          /* rotate clockwise */
            else if (kind == 's') { sx = (x - dx + n) % n; sy = (y - dy + n) % n; }
            bset(x, y, tmp[sy][sx]);
        }
}

/* ---------------------------------------------------------------- undo */

static void push(Tileset **stack, int *n) {
    if (*n == MAX_UNDO) {
        res_free(RES_TILES, stack[0]);
        memmove(stack, stack + 1, sizeof stack[0] * (MAX_UNDO - 1));
        (*n)--;
    }
    stack[(*n)++] = res_copy(RES_TILES, TS);
}

static void clear_stack(Tileset **stack, int *n) {
    while (*n > 0) res_free(RES_TILES, stack[--*n]);
}

static void before_change(void) {
    push(T.undo, &T.nundo);
    clear_stack(T.redo, &T.nredo);
    doc_touch(RES_TILES);
}

static void undo_redo(bool redo) {
    Tileset **from = redo ? T.redo : T.undo, **to = redo ? T.undo : T.redo;
    int *nfrom = redo ? &T.nredo : &T.nundo, *nto = redo ? &T.nundo : &T.nredo;
    if (!*nfrom) { status(COL_GREY, "nothing to %s", redo ? "redo" : "undo"); return; }
    push(to, nto);
    Tileset *t = from[--*nfrom];
    memcpy(TS, t, sizeof *TS);
    res_free(RES_TILES, t);
    doc_touch(RES_TILES);
}

/* the tree opened another tileset: its undo history doesn't apply */
void tiles_doc_changed(void) {
    clear_stack(T.undo, &T.nundo);
    clear_stack(T.redo, &T.nredo);
    if (T.drag == 4) T.drag = 0;
}

/* ---------------------------------------------------------------- commands */

static void save(void) {
    char err[256];
    Doc *d = doc_active(RES_TILES);
    if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved tiles/%s.g64", d->name);
    else status(COL_RED, "%s", err);
}

static void copy_block(bool cut) {
    int n = bpx();
    for (int y = 0; y < n; y++) for (int x = 0; x < n; x++) T.clip[y * 4 * TILE + x] = (uint8_t)bget(x, y);
    T.clip_size = T.bsize;
    if (cut) {
        before_change();
        brect(0, 0, n - 1, n - 1, CLEAR, true);
    }
    status(COL_GREY, "%s %dx%d tile%s", cut ? "cut" : "copied", T.bsize, T.bsize, T.bsize > 1 ? "s" : "");
}

static void paste_block(void) {
    if (!T.clip_size) { status(COL_GREY, "nothing copied yet"); return; }
    before_change();
    int n = T.clip_size * TILE;
    int col = T.sel % SHEET_COLS, row = T.sel / SHEET_COLS;
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            int tc = col + x / TILE, tr = row + y / TILE;
            if (tc >= SHEET_COLS || tr >= SHEET_ROWS) continue;
            TS->t[tr * SHEET_COLS + tc][(y % TILE) * TILE + x % TILE] = T.clip[y * 4 * TILE + x];
        }
}

static void set_tool(int t) {
    if (t == T_PICK && T.tool != T_PICK) T.prev_tool = T.tool;
    T.tool = t;
}

static void act(int a) {
    switch (a) {
        case A_SIZE: T.bsize = T.bsize == 1 ? 2 : T.bsize == 2 ? 4 : 1; clamp_sel(); break;
        case A_FLIPX: before_change(); transform('h', 0, 0); break;
        case A_FLIPY: before_change(); transform('v', 0, 0); break;
        case A_TURN: before_change(); transform('t', 0, 0); break;
        case A_CLEAR: before_change(); brect(0, 0, bpx() - 1, bpx() - 1, CLEAR, true); break;
    }
}

/* zoom the background by `dir` steps, keeping the sheet pixel under (mx, my) still */
static void zoom_at(int dir, int mx, int my) {
    int z = T.zoom + dir;
    if (z < 0 || z >= NZOOM) return;
    int old = zooms[T.zoom], now = zooms[z];
    int px = (mx - T.ox) / old, py = (my - T.oy) / old;
    T.zoom = z;
    T.ox = mx - px * now;
    T.oy = my - py * now;
}

static void clamp_pan(void) {
    int z = zooms[T.zoom];
    int w = SHEET_W * z, h = SHEET_H * z;
    /* keep at least a corner of the sheet in view */
    if (T.ox > UI_W - 40) T.ox = UI_W - 40;
    if (T.oy > BOT - 40) T.oy = BOT - 40;
    if (T.ox + w < 40) T.ox = 40 - w;
    if (T.oy + h < TOP + 40) T.oy = TOP + 40 - h;
}

static void key(const KeyEvent *ev) {
    int sc = ev->sc;
    bool ctrl = ev->mod & MOD_CTRL, shift = ev->mod & MOD_SHIFT;
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: save(); return;
            case SDL_SCANCODE_R: ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: undo_redo(shift); return;
            case SDL_SCANCODE_Y: undo_redo(true); return;
            case SDL_SCANCODE_C: copy_block(false); return;
            case SDL_SCANCODE_X: copy_block(true); return;
            case SDL_SCANCODE_V: paste_block(); return;
            default: return;
        }
    }
    int dx = 0, dy = 0;
    switch (sc) {
        case SDL_SCANCODE_ESCAPE: ui_switch(MODE_SHELL); return;
        case SDL_SCANCODE_P: set_tool(T_PENCIL); return;
        case SDL_SCANCODE_F: set_tool(T_FILL); return;
        case SDL_SCANCODE_L: set_tool(T_LINE); return;
        case SDL_SCANCODE_R: set_tool(T_RECT); return;
        case SDL_SCANCODE_I: set_tool(T_PICK); return;
        case SDL_SCANCODE_G: T.grid = !T.grid; return;
        case SDL_SCANCODE_X: { int t = T.col_a; T.col_a = T.col_b; T.col_b = t; return; }
        case SDL_SCANCODE_Z: act(A_SIZE); return;
        case SDL_SCANCODE_H: act(A_FLIPX); return;
        case SDL_SCANCODE_V: act(A_FLIPY); return;
        case SDL_SCANCODE_T: act(A_TURN); return;
        case SDL_SCANCODE_DELETE: case SDL_SCANCODE_BACKSPACE: act(A_CLEAR); return;
        case SDL_SCANCODE_EQUALS: case SDL_SCANCODE_KP_PLUS: zoom_at(1, UI_W / 2, (TOP + BOT) / 2); return;
        case SDL_SCANCODE_MINUS: case SDL_SCANCODE_KP_MINUS: zoom_at(-1, UI_W / 2, (TOP + BOT) / 2); return;
        case SDL_SCANCODE_HOME: T.zoom = 1; T.ox = 0; T.oy = TOP; place_windows(); return;
        case SDL_SCANCODE_LEFT: dx = -1; break;
        case SDL_SCANCODE_RIGHT: dx = 1; break;
        case SDL_SCANCODE_UP: dy = -1; break;
        case SDL_SCANCODE_DOWN: dy = 1; break;
        default: return;
    }
    if (shift) { before_change(); transform('s', dx, dy); return; }   /* nudge the pixels, wrapping */
    int col = T.sel % SHEET_COLS + dx, row = T.sel / SHEET_COLS + dy;
    if (col < 0 || row < 0 || col > SHEET_COLS - T.bsize || row > SHEET_ROWS - T.bsize) return;
    T.sel = row * SHEET_COLS + col;
}

/* ---------------------------------------------------------------- the windows' insides */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

static bool in_win(int i) { const Win *w = &T.win[i]; return inside(w->x, w->y, w->w, TITLE_H + w->h); }

/* the tile window's canvas: pixels of `scale` screen pixels, centred */
static int canvas_scale(void) {
    const Win *w = &T.win[W_TILE];
    int s = (w->w < w->h ? w->w : w->h) / bpx();
    return s < 1 ? 1 : s;
}
static void canvas_origin(int *cx, int *cy) {
    const Win *w = &T.win[W_TILE];
    int size = canvas_scale() * bpx();
    *cx = w->x + (w->w - size) / 2;
    *cy = w->y + TITLE_H + (w->h - size) / 2;
}

/* the tools window's parts */
static int tool_btn(int i, int *x, int *y) { *x = T.win[W_TOOLS].x + 2 + i * 13; *y = T.win[W_TOOLS].y + TITLE_H + 2; return 12; }
static int act_btn(int i, int *x, int *y) { *x = T.win[W_TOOLS].x + 2 + i * 13; *y = T.win[W_TOOLS].y + TITLE_H + 14; return 12; }
static void pal_origin(int *x, int *y) { *x = T.win[W_TOOLS].x + 3; *y = T.win[W_TOOLS].y + TITLE_H + 28; }
#define PAL_CELL 8

static void canvas_press(int btn) {
    int s = canvas_scale(), cx, cy;
    canvas_origin(&cx, &cy);
    int x = (in.mouse_x - cx) / s, y = (in.mouse_y - cy) / s;
    if (in.mouse_x < cx || in.mouse_y < cy || x >= bpx() || y >= bpx()) return;
    int c = btn == 1 ? T.col_a : T.col_b;
    if (btn == 3 || T.tool == T_PICK) {          /* middle button always picks */
        int got = bget(x, y);
        if (btn == 2) T.col_b = got; else T.col_a = got;
        if (T.tool == T_PICK) T.tool = T.prev_tool;
        return;
    }
    before_change();
    if (T.tool == T_FILL) { bfill(x, y, c); return; }
    res_free(RES_TILES, T.base);
    T.base = res_copy(RES_TILES, TS);
    T.drag = 4;
    T.stroke_btn = btn;
    T.x0 = T.lastx = x;
    T.y0 = T.lasty = y;
    if (T.tool == T_PENCIL) bset(x, y, c);
}

static void canvas_drag(void) {
    int s = canvas_scale(), n = bpx(), cx, cy;
    canvas_origin(&cx, &cy);
    int x = (in.mouse_x - cx) / s, y = (in.mouse_y - cy) / s;
    if (in.mouse_x < cx) x = -1;
    if (in.mouse_y < cy) y = -1;
    if (x >= n) x = n;
    if (y >= n) y = n;
    int c = T.stroke_btn == 1 ? T.col_a : T.col_b;
    if (T.tool == T_PENCIL) {
        bline(T.lastx, T.lasty, x, y, c);
    } else {
        memcpy(TS, T.base, sizeof *TS);
        if (T.tool == T_LINE) bline(T.x0, T.y0, x, y, c);
        else brect(T.x0, T.y0, x, y, c, in.key[SDL_SCANCODE_LSHIFT] || in.key[SDL_SCANCODE_RSHIFT]);
    }
    T.lastx = x;
    T.lasty = y;
}

static void tools_press(int btn) {
    int x, y, w;
    for (int i = 0; i < NTOOLS; i++) { w = tool_btn(i, &x, &y); if (inside(x, y, w, 10)) { set_tool(i); return; } }
    for (int i = 0; i < NACTS; i++) { w = act_btn(i, &x, &y); if (inside(x, y, w, 10)) { act(i); return; } }
    pal_origin(&x, &y);
    if (inside(x, y, 8 * PAL_CELL, 8 * PAL_CELL)) {
        int c = ((in.mouse_y - y) / PAL_CELL) * 8 + (in.mouse_x - x) / PAL_CELL;
        if (btn == 2) T.col_b = c; else T.col_a = c;
        return;
    }
    if (inside(x, y + 8 * PAL_CELL + 2, 22, 10)) { if (btn == 2) T.col_b = CLEAR; else T.col_a = CLEAR; }
}

/* ---------------------------------------------------------------- mouse */

static int sheet_tile_at(int mx, int my) {     /* -1 if not on the sheet */
    int z = zooms[T.zoom];
    int px = mx - T.ox, py = my - T.oy;
    if (px < 0 || py < 0 || px >= SHEET_W * z || py >= SHEET_H * z) return -1;
    return (py / z / TILE) * SHEET_COLS + px / z / TILE;
}

static void mouse(void) {
    bool space = in.key[SDL_SCANCODE_SPACE];
    if (T.drag) {
        int held = T.drag == 4 ? T.stroke_btn : T.drag == 3 ? (in.mouse[3] ? 3 : 1) : 1;
        /* a new press (even one with no frame of release before it) ends the drag */
        if (in.mouse[held] && !in.mouse_hit[held]) {
            Win *w = &T.win[T.drag_win];
            switch (T.drag) {
                case 1: w->x = in.mouse_x - T.drag_dx; w->y = in.mouse_y - T.drag_dy; keep_windows_inside(); break;
                case 2:
                    w->w = in.mouse_x - w->x + T.drag_dx;
                    w->h = in.mouse_y - w->y - TITLE_H + T.drag_dy;
                    if (w->w < 44) w->w = 44;
                    if (w->h < 44) w->h = 44;
                    if (w->h > BOT - w->y - TITLE_H) w->h = BOT - w->y - TITLE_H;
                    keep_windows_inside();
                    break;
                case 3: T.ox = in.mouse_x - T.pan_x; T.oy = in.mouse_y - T.pan_y; clamp_pan(); break;
                case 4: canvas_drag(); break;
            }
            return;
        }
        T.drag = 0;
    }
    int btn = in.mouse_hit[1] ? 1 : in.mouse_hit[2] ? 2 : in.mouse_hit[3] ? 3 : 0;

    /* the windows, top one first */
    int order[NWIN] = {T.top, 1 - T.top};
    for (int k = 0; k < NWIN; k++) {
        int i = order[k];
        if (!in_win(i)) continue;
        Win *w = &T.win[i];
        if (!btn) return;                       /* the wheel over a window does nothing */
        T.top = i;
        if (btn == 1 && in.mouse_y < w->y + TITLE_H) {          /* title bar: move */
            T.drag = 1; T.drag_win = i;
            T.drag_dx = in.mouse_x - w->x; T.drag_dy = in.mouse_y - w->y;
            return;
        }
        if (btn == 1 && i == W_TILE && inside(w->x + w->w - GRIP, w->y + TITLE_H + w->h - GRIP, GRIP, GRIP)) {
            T.drag = 2; T.drag_win = i;                            /* corner: resize */
            T.drag_dx = w->x + w->w - in.mouse_x; T.drag_dy = w->y + TITLE_H + w->h - in.mouse_y;
            return;
        }
        if (i == W_TILE) canvas_press(btn);
        else tools_press(btn);
        return;
    }

    /* the background */
    if (in.mouse_y < TOP || in.mouse_y >= BOT) return;
    if (in.wheel) { zoom_at(in.wheel > 0 ? 1 : -1, in.mouse_x, in.mouse_y); clamp_pan(); }
    if (btn == 3 || (btn == 1 && space)) {
        T.drag = 3;
        T.pan_x = in.mouse_x - T.ox;
        T.pan_y = in.mouse_y - T.oy;
        return;
    }
    if (btn == 1 || btn == 2) {
        int t = sheet_tile_at(in.mouse_x, in.mouse_y);
        if (t >= 0) { T.sel = t; clamp_sel(); }
    }
}

/* ---------------------------------------------------------------- drawing */

static void checker(int x, int y, int w, int h) {   /* transparent: pink checks, 4 screen pixels */
    for (int j = 0; j < h; j++) {
        int sy = y + j;
        for (int i = 0; i < w; i++) {
            int sx = x + i;
            draw_pset(sx, sy, ((sx >> 2) + (sy >> 2)) & 1 ? CHECK_A : CHECK_B);
        }
    }
}

static void swatch(int x, int y, int w, int h, int c) {
    if (c == CLEAR) checker(x, y, w, h);
    else draw_rect(x, y, w, h, c, true);
}

static void colour_name(int c, char *buf, size_t n) {
    if (c == CLEAR) snprintf(buf, n, "clear");
    else snprintf(buf, n, "%d (r%d g%d b%d)", c, c >> 4, (c >> 2) & 3, c & 3);
}

static void draw_background(void) {
    int z = zooms[T.zoom];
    draw_clip(0, TOP, UI_W, BOT - TOP);
    draw_rect(0, TOP, UI_W, BOT - TOP, RGB6(0, 0, 1), true);
    /* the sheet: only the part in view */
    int x0 = T.ox < 0 ? -T.ox / z : 0, y0 = T.oy < TOP ? (TOP - T.oy) / z : 0;
    int x1 = (UI_W - T.ox) / z + 1, y1 = (BOT - T.oy) / z + 1;
    if (x1 > SHEET_W) x1 = SHEET_W;
    if (y1 > SHEET_H) y1 = SHEET_H;
    for (int py = y0; py < y1; py++)
        for (int px = x0; px < x1; px++) {
            uint8_t c = TS->t[(py / TILE) * SHEET_COLS + px / TILE][(py % TILE) * TILE + px % TILE];
            int sx = T.ox + px * z, sy = T.oy + py * z;
            if (c == CLEAR) checker(sx, sy, z, z);
            else if (z == 1) draw_pset(sx, sy, c);
            else draw_rect(sx, sy, z, z, c, true);
        }
    if (T.grid && z >= 2) {
        for (int c = 0; c <= SHEET_COLS; c++) draw_line(T.ox + c * TILE * z, T.oy, T.ox + c * TILE * z, T.oy + SHEET_H * z - 1, RGB6(1, 0, 1));
        for (int r = 0; r <= SHEET_ROWS; r++) draw_line(T.ox, T.oy + r * TILE * z, T.ox + SHEET_W * z - 1, T.oy + r * TILE * z, RGB6(1, 0, 1));
    }
    draw_rect(T.ox - 1, T.oy - 1, SHEET_W * z + 2, SHEET_H * z + 2, RGB6(1, 2, 3), false);
    int scol = T.sel % SHEET_COLS, srow = T.sel / SHEET_COLS, bt = T.bsize * TILE * z;
    draw_rect(T.ox + scol * TILE * z - 1, T.oy + srow * TILE * z - 1, bt + 2, bt + 2, COL_YELLOW, false);
    draw_clip_reset();
}

static void window_frame(int i, const char *title) {
    const Win *w = &T.win[i];
    bool on = T.top == i;
    draw_rect(w->x - 1, w->y - 1, w->w + 2, TITLE_H + w->h + 2, on ? RGB6(1, 2, 3) : RGB6(1, 1, 2), false);
    draw_rect(w->x, w->y, w->w, TITLE_H, on ? RGB6(0, 1, 2) : RGB6(0, 1, 1), true);
    draw_rect(w->x, w->y + TITLE_H, w->w, w->h, RGB6(0, 0, 0), true);
    draw_clip(w->x, w->y, w->w, TITLE_H);
    draw_text(title, w->x + 2, w->y - 1, on ? COL_WHITE : RGB6(2, 2, 3));
    draw_clip_reset();
}

static void draw_tile_window(int *hx, int *hy) {
    const Win *w = &T.win[W_TILE];
    char title[48];
    if (T.bsize > 1) snprintf(title, sizeof title, "tiles %d %dx%d", T.sel, T.bsize, T.bsize);
    else snprintf(title, sizeof title, "tile %d", T.sel);
    window_frame(W_TILE, title);
    int s = canvas_scale(), n = bpx(), cx, cy;
    canvas_origin(&cx, &cy);
    draw_clip(w->x, w->y + TITLE_H, w->w, w->h);
    int gap = T.grid && s >= 8 ? 1 : 0;
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            int c = bget(x, y);
            if (c == CLEAR) checker(cx + x * s, cy + y * s, s - gap, s - gap);
            else draw_rect(cx + x * s, cy + y * s, s - gap, s - gap, c, true);
        }
    if (T.bsize > 1)       /* tile edges inside a block */
        for (int k = 1; k < T.bsize; k++) {
            int o = k * TILE * s - (gap ? 1 : 0);
            draw_line(cx + o, cy, cx + o, cy + n * s - 1, RGB6(1, 2, 3));
            draw_line(cx, cy + o, cx + n * s - 1, cy + o, RGB6(1, 2, 3));
        }
    *hx = *hy = -1;
    if (T.top == W_TILE || !in_win(W_TOOLS)) {
        if (inside(cx, cy, n * s, n * s)) {
            *hx = (in.mouse_x - cx) / s;
            *hy = (in.mouse_y - cy) / s;
            draw_rect(cx + *hx * s - 1, cy + *hy * s - 1, s + 1, s + 1, COL_WHITE, false);
        }
    }
    /* the resize grip */
    for (int k = 0; k < GRIP; k += 2) draw_line(w->x + w->w - 1 - k, w->y + TITLE_H + w->h - 1, w->x + w->w - 1, w->y + TITLE_H + w->h - 1 - k, RGB6(2, 2, 3));
    draw_clip_reset();
}

static void button(int x, int y, int w, char label, bool on, bool hover) {
    draw_rect(x, y, w, 10, on ? RGB6(1, 2, 3) : hover ? RGB6(0, 1, 2) : RGB6(0, 0, 1), true);
    draw_char(label, x + (w - 5) / 2, y, on ? COL_BLACK : RGB6(2, 3, 3));
}

static void draw_tools_window(void) {
    window_frame(W_TOOLS, "tools");
    int x, y, w;
    bool live = T.top == W_TOOLS || !in_win(W_TILE);
    for (int i = 0; i < NTOOLS; i++) {
        w = tool_btn(i, &x, &y);
        bool hover = live && inside(x, y, w, 10);
        button(x, y, w, tool_keys[i], T.tool == i, hover);
        if (hover) T.hint = tool_names[i];
    }
    for (int i = 0; i < NACTS; i++) {
        w = act_btn(i, &x, &y);
        bool hover = live && inside(x, y, w, 10);
        char label = i == A_SIZE ? (char)('0' + T.bsize) : act_keys[i];
        button(x, y, w, label, false, hover);
        if (hover) T.hint = act_names[i];
    }
    int px, py;
    pal_origin(&px, &py);
    for (int i = 0; i < NCOLORS; i++) draw_rect(px + (i % 8) * PAL_CELL, py + (i / 8) * PAL_CELL, PAL_CELL - 1, PAL_CELL - 1, i, true);
    int cy = py + 8 * PAL_CELL + 2;
    swatch(px, cy, 21, 9, CLEAR);
    draw_rect(px - 1, cy - 1, 23, 11, RGB6(1, 1, 2), false);
    if (live && inside(px, cy, 22, 10)) T.hint = "transparent";
    /* A and B: outlines on the palette, and swatches */
    for (int k = 0; k < 2; k++) {
        int c = k ? T.col_b : T.col_a;
        int ox = c == CLEAR ? px - 1 : px + (c % 8) * PAL_CELL - 1;
        int oy = c == CLEAR ? cy - 1 : py + (c / 8) * PAL_CELL - 1;
        draw_rect(ox, oy, c == CLEAR ? 23 : PAL_CELL + 1, c == CLEAR ? 11 : PAL_CELL + 1, k ? COL_GREY : COL_WHITE, false);
    }
    draw_text("A", px + 26, cy, COL_WHITE);
    swatch(px + 33, cy, 9, 9, T.col_a);
    draw_text("B", px + 46, cy, COL_GREY);
    swatch(px + 53, cy, 9, 9, T.col_b);
    if (live && inside(px, py, 8 * PAL_CELL, 8 * PAL_CELL)) T.hint = "left: colour A  right: colour B";
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    T.hint = NULL;
    draw_background();
    int hx = -1, hy = -1;
    if (T.top == W_TOOLS) { draw_tile_window(&hx, &hy); draw_tools_window(); }
    else { draw_tools_window(); draw_tile_window(&hx, &hy); }

    /* bars */
    ui_draw_tabs();
    int by = BOT;
    draw_rect(0, by, UI_W, FONT_H, RGB6(0, 1, 2), true);
    Doc *d = doc_active(RES_TILES);
    char info[64], line[128];
    snprintf(info, sizeof info, "%s%s", d->name, d->dirty ? "*" : "");
    int x = draw_text(info, 3, by, RGB6(2, 3, 3)) + 8;
    bool over_win = in_win(W_TILE) || in_win(W_TOOLS);
    int ht = over_win || in.mouse_y < TOP || in.mouse_y >= BOT ? -1 : sheet_tile_at(in.mouse_x, in.mouse_y);
    if (T.status[0] && frame_count < T.status_until) draw_text(T.status, x, by, T.status_color);
    else if (hx >= 0 && hx < bpx() && hy >= 0 && hy < bpx()) {
        char c[32];
        colour_name(bget(hx, hy), c, sizeof c);
        snprintf(line, sizeof line, "tile %d x%d y%d %s", T.sel + (hy / TILE) * SHEET_COLS + hx / TILE, hx % TILE, hy % TILE, c);
        draw_text(line, x, by, RGB6(2, 3, 3));
    } else if (T.hint) draw_text(T.hint, x, by, RGB6(2, 3, 3));
    else if (ht >= 0) {
        snprintf(line, sizeof line, "tile %d  (col %d row %d)  x%d", ht, ht % SHEET_COLS, ht / SHEET_COLS, zooms[T.zoom]);
        draw_text(line, x, by, RGB6(2, 3, 3));
    } else draw_text("middle drag: pan  wheel: zoom", x, by, RGB6(1, 2, 2));
}

void tiles_frame(void) {
    init();
    if (!cart.loaded || !TS) { ui_switch(MODE_SHELL); return; }
    if (!T.placed) place_windows();
    keep_windows_inside();
    for (int i = 0; i < in.nevents && ui_mode == MODE_TILES; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        if (ui_global_key(ev)) continue;
        key(ev);
    }
    if (ui_mode != MODE_TILES) return;
    mouse();
    draw();
}
