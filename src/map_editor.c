/* The built-in map editor.
 *
 * The map fills the editor. The brush is one tile, or a block of tiles
 * taken from the tileset (Tab opens the tile picker; drag across it for a
 * block) or copied from the map with the select tool. It follows the tile
 * editor: whatever is selected there (a tile, or a block of them) becomes
 * the brush, and a tile picked here is selected there too. Left button paints,
 * right button picks up the tile under the mouse. E makes the brush the
 * eraser: an empty cell. Tile 0 is a tile like any other.
 *
 * A map is 1-10 screens (40 x 24 cells) each way: Ctrl+arrows add or remove
 * a column or row of screens. Tiles are drawn from the tileset the tile
 * editor has open (T switches to the next one).
 *
 * Zoom shows a cell as 20, 10, 5, 2 or 1 pixels; below 5, each tile is drawn
 * as its most common color. Edits go straight into the open map file, so
 * Ctrl+R runs them without saving. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIEW_Y FONT_H
#define VIEW_H (SCREEN_H - 2 * FONT_H)
#define MAXB 64              /* biggest brush, in cells */
#define MAX_UNDO 64
#define PK_Y (VIEW_Y + 10)   /* tile picker: the sheet, below a title line */
#define PK_H (VIEW_H - 10)

enum { T_PEN, T_RECT, T_FILL, T_SELECT, NTOOLS };
static const char *const tool_names[NTOOLS] = {"pen", "rect", "fill", "select"};
static const int zoom_sizes[] = {20, 10, 5, 2, 1};
#define NZOOM 5

#define MAP (cur_map)

static struct {
    bool ready;
    int vx, vy;                  /* view origin, in map pixels (TILE per cell) */
    int zoom;
    int tool;
    uint16_t brush[MAXB][MAXB];
    int bw, bh;
    bool stroking;
    int sbtn, x0, y0, lastx, lasty;
    TileMap *base;               /* the map when a rect drag started */
    bool sel;
    int sx0, sy0, sx1, sy1;      /* selection, inclusive cells */
    TileMap *undo[MAX_UNDO], *redo[MAX_UNDO];
    int nundo, nredo;
    bool grid;
    bool picker, pk_drag;
    int pk_t0, pk_t1;            /* first and last tile of a picker drag */
    int pk_ox, pk_oy;            /* picker scroll, in sheet pixels */
    bool pk_pan;
    int pk_mx, pk_my, pk_px, pk_py;
    bool panning;
    int pan_mx, pan_my, pan_vx, pan_vy;
    int tsel, tw, th;            /* the tile editor's selection when the brush last followed it */
    char status[128];
    int status_color;
    long status_until;
} M;

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(M.status, sizeof M.status, fmt, ap);
    va_end(ap);
    M.status_color = color;
    M.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[map] %s\n", M.status);
}

/* the brush is now tiles from the sheet: select them in the tile editor too,
 * and remember that selection so following it doesn't undo this brush */
static void tell_tiles(void) {
    if (M.brush[0][0] == MAP_EMPTY) return;
    tiles_select(M.brush[0][0], M.bw, M.bh);
    tiles_selection(&M.tsel, &M.tw, &M.th);
}

static void set_brush_tile(int t) {
    M.bw = M.bh = 1;
    M.brush[0][0] = (uint16_t)t;
    tell_tiles();
}

/* a new selection in the tile editor becomes the brush: its tile, or its whole
 * block as it is laid out on the sheet */
static void follow_tiles(void) {
    int t, w, h;
    tiles_selection(&t, &w, &h);
    if (t == M.tsel && w == M.tw && h == M.th) return;
    M.tsel = t;
    M.tw = w;
    M.th = h;
    M.bw = w < MAXB ? w : MAXB;
    M.bh = h < MAXB ? h : MAXB;
    for (int j = 0; j < M.bh; j++)
        for (int i = 0; i < M.bw; i++) M.brush[j][i] = (uint16_t)(t + j * SHEET_COLS + i);
}

static void init(void) {
    if (M.ready) return;
    M.ready = true;
    M.zoom = 1;
    M.grid = true;
    M.tsel = M.tw = M.th = -1;       /* the first frame takes the tile editor's selection: tile 0 */
}

/* ---------------------------------------------------------------- coordinates */

static int cs(void) { return zoom_sizes[M.zoom]; }
static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }
static int posmod(int a, int b) { int m = a % b; return m < 0 ? m + b : m; }

static int screen_x(int cx) { return (cx * TILE - M.vx) * cs() / TILE; }
static int screen_y(int cy) { return VIEW_Y + (cy * TILE - M.vy) * cs() / TILE; }
static int cell_x(int mx) { return floordiv(mx * TILE / cs() + M.vx, TILE); }
static int cell_y(int my) { return floordiv((my - VIEW_Y) * TILE / cs() + M.vy, TILE); }

static bool in_view(void) { return in.mouse_x >= 0 && in.mouse_y >= VIEW_Y && in.mouse_y < VIEW_Y + VIEW_H; }
static bool on_map(int x, int y) { return x >= 0 && y >= 0 && x < MAP->w && y < MAP->h; }
static uint16_t *cellp(int x, int y) { return &MAP->cells[y * MAP->w + x]; }

static void clamp_view(void) {
    int vw = UI_W * TILE / cs(), vh = VIEW_H * TILE / cs();
    int mw = MAP->w * TILE, mh = MAP->h * TILE;
    int minx = -vw / 2, maxx = mw - vw / 2, miny = -vh / 2, maxy = mh - vh / 2;
    if (vw >= mw) minx = maxx = (mw - vw) / 2;     /* the whole map fits: centre it */
    if (vh >= mh) miny = maxy = (mh - vh) / 2;
    if (M.vx < minx) M.vx = minx;
    if (M.vx > maxx) M.vx = maxx;
    if (M.vy < miny) M.vy = miny;
    if (M.vy > maxy) M.vy = maxy;
}

/* zoom by `dir` steps, keeping the map point under (mx, my) still */
static void zoom_at(int dir, int mx, int my) {
    int z = M.zoom - dir;
    if (z < 0 || z >= NZOOM) return;
    int px = mx * TILE / cs() + M.vx, py = (my - VIEW_Y) * TILE / cs() + M.vy;
    M.zoom = z;
    M.vx = px - mx * TILE / cs();
    M.vy = py - (my - VIEW_Y) * TILE / cs();
    clamp_view();
}

/* ---------------------------------------------------------------- editing */

static void push(TileMap **stack, int *n) {
    if (*n == MAX_UNDO) {
        res_free(RES_MAP, stack[0]);
        memmove(stack, stack + 1, sizeof stack[0] * (MAX_UNDO - 1));
        (*n)--;
    }
    stack[(*n)++] = res_copy(RES_MAP, MAP);
}

static void clear_stack(TileMap **stack, int *n) {
    while (*n > 0) res_free(RES_MAP, stack[--*n]);
}

static void before_change(void) {
    push(M.undo, &M.nundo);
    clear_stack(M.redo, &M.nredo);
    doc_touch(RES_MAP);
}

static void put_map(const TileMap *from) {     /* the map becomes a copy of `from` (size included) */
    map_resize(MAP, from->w, from->h);
    memcpy(MAP->cells, from->cells, sizeof *MAP->cells * (size_t)from->w * (size_t)from->h);
}

static void undo_redo(bool redo) {
    TileMap **from = redo ? M.redo : M.undo, **to = redo ? M.undo : M.redo;
    int *nfrom = redo ? &M.nredo : &M.nundo, *nto = redo ? &M.nundo : &M.nredo;
    if (!*nfrom) { status(COL_GREY, "nothing to %s", redo ? "redo" : "undo"); return; }
    push(to, nto);
    TileMap *m = from[--*nfrom];
    put_map(m);
    res_free(RES_MAP, m);
    doc_touch(RES_MAP);
    clamp_view();
}

/* the tree opened another map: its undo history doesn't apply */
void map_doc_changed(void) {
    clear_stack(M.undo, &M.nundo);
    clear_stack(M.redo, &M.nredo);
    M.sel = M.stroking = false;
    M.vx = M.vy = 0;
}

static void set_cell(int x, int y, int t) { if (on_map(x, y)) *cellp(x, y) = (uint16_t)t; }

/* the brush tile for cell (x, y) when the pattern is anchored at (ax, ay) */
static int pattern(int x, int y, int ax, int ay) { return M.brush[posmod(y - ay, M.bh)][posmod(x - ax, M.bw)]; }

static void stamp(int x, int y) {
    for (int j = 0; j < M.bh; j++)
        for (int i = 0; i < M.bw; i++) set_cell(x + i, y + j, M.brush[j][i]);
}

/* pen: stamp at every brush-aligned cell on the way from the last cell */
static void pen_to(int x, int y) {
    int x0 = M.lastx, y0 = M.lasty;
    int dx = abs(x - x0), sx = x0 < x ? 1 : -1, dy = -abs(y - y0), sy = y0 < y ? 1 : -1, err = dx + dy;
    for (;;) {
        if (posmod(x0 - M.x0, M.bw) == 0 && posmod(y0 - M.y0, M.bh) == 0) stamp(x0, y0);
        if (x0 == x && y0 == y) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    M.lastx = x;
    M.lasty = y;
}

static void fill_rect(int x0, int y0, int x1, int y1) {
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) set_cell(x, y, pattern(x, y, x0, y0));
}

static void flood(int x, int y) {
    if (!on_map(x, y)) return;
    int w = MAP->w, h = MAP->h;
    uint16_t from = *cellp(x, y);
    uint8_t *seen = calloc((size_t)w * (size_t)h, 1);
    int (*stack)[2] = malloc(sizeof *stack * (size_t)w * (size_t)h);
    int n = 0, ax = x, ay = y;
    stack[n][0] = x; stack[n][1] = y; n++;
    seen[y * w + x] = 1;
    while (n > 0) {
        n--;
        int px = stack[n][0], py = stack[n][1];
        *cellp(px, py) = (uint16_t)pattern(px, py, ax, ay);
        static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int k = 0; k < 4; k++) {
            int qx = px + d[k][0], qy = py + d[k][1];
            if (on_map(qx, qy) && !seen[qy * w + qx] && *cellp(qx, qy) == from) {
                seen[qy * w + qx] = 1;
                stack[n][0] = qx; stack[n][1] = qy; n++;
            }
        }
    }
    free(seen);
    free(stack);
}

static void norm_sel(int *x0, int *y0, int *x1, int *y1) {
    *x0 = M.sx0 < M.sx1 ? M.sx0 : M.sx1;
    *x1 = M.sx0 < M.sx1 ? M.sx1 : M.sx0;
    *y0 = M.sy0 < M.sy1 ? M.sy0 : M.sy1;
    *y1 = M.sy0 < M.sy1 ? M.sy1 : M.sy0;
    if (*x0 < 0) *x0 = 0;
    if (*y0 < 0) *y0 = 0;
    if (*x1 >= MAP->w) *x1 = MAP->w - 1;
    if (*y1 >= MAP->h) *y1 = MAP->h - 1;
}

/* Ctrl+C / Ctrl+X: the selection becomes the brush */
static void copy_sel(bool cut) {
    if (!M.sel) { status(COL_GREY, "select an area first (S, then drag)"); return; }
    int x0, y0, x1, y1;
    norm_sel(&x0, &y0, &x1, &y1);
    int w = x1 - x0 + 1, h = y1 - y0 + 1;
    if (w > MAXB) w = MAXB;
    if (h > MAXB) h = MAXB;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) M.brush[j][i] = *cellp(x0 + i, y0 + j);
    M.bw = w;
    M.bh = h;
    if (cut) {
        before_change();
        for (int y = y0; y <= y1; y++)
            for (int x = x0; x <= x1; x++) *cellp(x, y) = MAP_EMPTY;
    }
    M.sel = false;
    M.tool = T_PEN;
    status(COL_GREY, "%s %dx%d cells: the brush now paints them", cut ? "cut" : "copied", w, h);
}

static void clear_sel(void) {
    if (!M.sel) return;
    int x0, y0, x1, y1;
    norm_sel(&x0, &y0, &x1, &y1);
    before_change();
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) *cellp(x, y) = MAP_EMPTY;
}

/* Ctrl+arrows: one screen more or less, on the right / bottom */
static void resize_screens(int dw, int dh) {
    int sw = MAP->w / SCREEN_CELLS_W + dw, sh = MAP->h / SCREEN_CELLS_H + dh;
    if (sw < 1 || sh < 1 || sw > MAP_MAX_SCREENS || sh > MAP_MAX_SCREENS) {
        status(COL_GREY, "a map is 1 to %d screens each way", MAP_MAX_SCREENS);
        return;
    }
    before_change();
    map_resize(MAP, sw * SCREEN_CELLS_W, sh * SCREEN_CELLS_H);
    M.sel = false;
    clamp_view();
    status(COL_GREY, "map: %d x %d screens (%d x %d cells)", sw, sh, MAP->w, MAP->h);
}

/* T: draw with the next tileset (the tile editor switches to it too) */
static void next_tileset(void) {
    const char *names[MAX_DOCS * 2];
    int n = docs_list(RES_TILES, names, MAX_DOCS * 2);
    const char *cur = doc_active(RES_TILES)->name;
    int i = 0;
    while (i < n && strcmp(names[i], cur)) i++;
    char err[512];
    for (int k = 1; k <= n; k++) {
        Doc *d = doc_open(RES_TILES, names[(i + k) % n], err, sizeof err);
        if (!d) continue;
        if (d != doc_active(RES_TILES)) { doc_set_active(d); docs_bind(); tiles_doc_changed(); }
        status(COL_GREY, "tiles from %s", d->name);
        return;
    }
}

static void save(void) {
    char err[256];
    Doc *d = doc_active(RES_MAP);
    if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved maps/%s.g64", d->name);
    else status(COL_RED, "%s", err);
}

/* ---------------------------------------------------------------- input */

static void key(const KeyEvent *ev) {
    int sc = ev->sc;
    bool ctrl = ev->mod & MOD_CTRL, shift = ev->mod & MOD_SHIFT;
    if (M.picker) {
        if (sc == SDL_SCANCODE_TAB || sc == SDL_SCANCODE_ESCAPE) M.picker = false;
        int step = shift ? 100 : TILE;
        if (sc == SDL_SCANCODE_LEFT) M.pk_ox -= step;
        if (sc == SDL_SCANCODE_RIGHT) M.pk_ox += step;
        if (sc == SDL_SCANCODE_UP) M.pk_oy -= step;
        if (sc == SDL_SCANCODE_DOWN) M.pk_oy += step;
        return;
    }
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: save(); return;
            case SDL_SCANCODE_R: ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: undo_redo(shift); return;
            case SDL_SCANCODE_Y: undo_redo(true); return;
            case SDL_SCANCODE_C: copy_sel(false); return;
            case SDL_SCANCODE_X: copy_sel(true); return;
            case SDL_SCANCODE_V: if (M.tool == T_SELECT) M.tool = T_PEN; M.sel = false; return;
            case SDL_SCANCODE_RIGHT: resize_screens(1, 0); return;
            case SDL_SCANCODE_LEFT: resize_screens(-1, 0); return;
            case SDL_SCANCODE_DOWN: resize_screens(0, 1); return;
            case SDL_SCANCODE_UP: resize_screens(0, -1); return;
            default: return;
        }
    }
    int step = shift ? 10 : 1;
    switch (sc) {
        case SDL_SCANCODE_ESCAPE:
            if (M.sel) { M.sel = false; return; }
            ui_switch(MODE_SHELL);
            return;
        case SDL_SCANCODE_TAB: M.picker = true; M.pk_drag = false; return;
        case SDL_SCANCODE_P: M.tool = T_PEN; return;
        case SDL_SCANCODE_R: M.tool = T_RECT; return;
        case SDL_SCANCODE_F: M.tool = T_FILL; return;
        case SDL_SCANCODE_S: M.tool = T_SELECT; return;
        case SDL_SCANCODE_G: M.grid = !M.grid; return;
        case SDL_SCANCODE_T: next_tileset(); return;
        case SDL_SCANCODE_E: set_brush_tile(MAP_EMPTY); status(COL_GREY, "brush: empty (erases)"); return;
        case SDL_SCANCODE_LEFTBRACKET:
            set_brush_tile(M.brush[0][0] == MAP_EMPTY ? NTILES - 1 : (M.brush[0][0] + NTILES - 1) % NTILES); return;
        case SDL_SCANCODE_RIGHTBRACKET:
            set_brush_tile(M.brush[0][0] == MAP_EMPTY ? 0 : (M.brush[0][0] + 1) % NTILES); return;
        case SDL_SCANCODE_I:
            if (in_view() && on_map(cell_x(in.mouse_x), cell_y(in.mouse_y)))
                set_brush_tile(*cellp(cell_x(in.mouse_x), cell_y(in.mouse_y)));
            return;
        case SDL_SCANCODE_DELETE: case SDL_SCANCODE_BACKSPACE: clear_sel(); return;
        case SDL_SCANCODE_EQUALS: case SDL_SCANCODE_KP_PLUS: zoom_at(1, UI_W / 2, VIEW_Y + VIEW_H / 2); return;
        case SDL_SCANCODE_MINUS: case SDL_SCANCODE_KP_MINUS: zoom_at(-1, UI_W / 2, VIEW_Y + VIEW_H / 2); return;
        case SDL_SCANCODE_HOME: M.vx = M.vy = 0; clamp_view(); return;
        case SDL_SCANCODE_LEFT: M.vx -= step * TILE; break;
        case SDL_SCANCODE_RIGHT: M.vx += step * TILE; break;
        case SDL_SCANCODE_UP: M.vy -= step * TILE; break;
        case SDL_SCANCODE_DOWN: M.vy += step * TILE; break;
        default: return;
    }
    clamp_view();
}

static void clamp_picker(void) {
    int maxx = SHEET_W - UI_W + 4, maxy = SHEET_H - PK_H + 4;
    if (M.pk_ox > maxx) M.pk_ox = maxx;
    if (M.pk_oy > maxy) M.pk_oy = maxy;
    if (M.pk_ox < 0) M.pk_ox = 0;
    if (M.pk_oy < 0) M.pk_oy = 0;
}

/* the sheet tile under the mouse in the picker, clamped to the sheet */
static int picker_tile(void) {
    int tx = (in.mouse_x - 2 + M.pk_ox) / TILE, ty = (in.mouse_y - PK_Y + M.pk_oy) / TILE;
    if (in.mouse_x - 2 + M.pk_ox < 0) tx = 0;
    if (in.mouse_y - PK_Y + M.pk_oy < 0) ty = 0;
    if (tx >= SHEET_COLS) tx = SHEET_COLS - 1;
    if (ty >= SHEET_ROWS) ty = SHEET_ROWS - 1;
    return ty * SHEET_COLS + tx;
}

/* returns true if a drag just finished, so a new press this frame belongs to the map */
static bool picker_mouse(void) {
    /* scroll: wheel (Shift: across), middle drag */
    bool shift = in.key[SDL_SCANCODE_LSHIFT] || in.key[SDL_SCANCODE_RSHIFT];
    if (in.wheel) { if (shift) M.pk_ox -= in.wheel * 20; else M.pk_oy -= in.wheel * 20; }
    if (M.pk_pan) {
        if (in.mouse[3]) { M.pk_ox = M.pk_px - (in.mouse_x - M.pk_mx); M.pk_oy = M.pk_py - (in.mouse_y - M.pk_my); }
        else M.pk_pan = false;
    } else if (in.mouse_hit[3]) {
        M.pk_pan = true;
        M.pk_mx = in.mouse_x; M.pk_my = in.mouse_y;
        M.pk_px = M.pk_ox; M.pk_py = M.pk_oy;
    }
    clamp_picker();
    bool inside = in.mouse_x >= 0 && in.mouse_x < UI_W && in.mouse_y >= PK_Y && in.mouse_y < PK_Y + PK_H;
    if (M.pk_drag && in.mouse[1] && !in.mouse_hit[1]) { M.pk_t1 = picker_tile(); return false; }
    if (!M.pk_drag) {
        if (!in.mouse_hit[1]) return false;
        if (!inside) { M.picker = false; return false; }
        M.pk_drag = true;
        M.pk_t0 = M.pk_t1 = picker_tile();
        return false;
    }
    /* released (or pressed again): the block from the first tile to the last one is the brush */
    int tx = M.pk_t1 % SHEET_COLS, ty = M.pk_t1 / SHEET_COLS;
    int c0 = M.pk_t0 % SHEET_COLS, r0 = M.pk_t0 / SHEET_COLS;
    int cx0 = c0 < tx ? c0 : tx, cx1 = c0 < tx ? tx : c0, cy0 = r0 < ty ? r0 : ty, cy1 = r0 < ty ? ty : r0;
    M.bw = cx1 - cx0 + 1;
    M.bh = cy1 - cy0 + 1;
    if (M.bw > MAXB) M.bw = MAXB;
    if (M.bh > MAXB) M.bh = MAXB;
    for (int j = 0; j < M.bh; j++)
        for (int i = 0; i < M.bw; i++) M.brush[j][i] = (uint16_t)((cy0 + j) * SHEET_COLS + cx0 + i);
    M.pk_drag = false;
    M.picker = false;
    M.sel = false;
    if (M.tool == T_SELECT) M.tool = T_PEN;
    tell_tiles();
    return true;
}

static void mouse(void) {
    if (M.picker && (!picker_mouse() || !in.mouse_hit[1])) return;
    if (in.wheel && in_view()) zoom_at(in.wheel > 0 ? 1 : -1, in.mouse_x, in.mouse_y);

    /* pan: middle button, or Space + left button */
    bool space = in.key[SDL_SCANCODE_SPACE];
    if (M.panning) {
        if (in.mouse[3] || (space && in.mouse[1])) {
            M.vx = M.pan_vx - (in.mouse_x - M.pan_mx) * TILE / cs();
            M.vy = M.pan_vy - (in.mouse_y - M.pan_my) * TILE / cs();
            clamp_view();
        } else M.panning = false;
        return;
    }
    if (in_view() && (in.mouse_hit[3] || (space && in.mouse_hit[1]))) {
        M.panning = true;
        M.pan_mx = in.mouse_x; M.pan_my = in.mouse_y;
        M.pan_vx = M.vx; M.pan_vy = M.vy;
        return;
    }

    int x = cell_x(in.mouse_x), y = cell_y(in.mouse_y);
    if (M.stroking) {
        if (in.mouse[M.sbtn] && !in.mouse_hit[M.sbtn]) {
            if (M.tool == T_PEN) pen_to(x, y);
            else if (M.tool == T_RECT) { put_map(M.base); fill_rect(M.x0, M.y0, x, y); }
            else if (M.tool == T_SELECT) { M.sx1 = x; M.sy1 = y; }
            return;
        }
        M.stroking = false;
    }
    if (!in_view()) return;
    if (in.mouse_hit[2]) {                       /* right button: pick up the tile */
        if (on_map(x, y)) set_brush_tile(*cellp(x, y));
        if (M.tool == T_SELECT) M.tool = T_PEN;
        return;
    }
    if (!in.mouse_hit[1]) return;
    M.stroking = true;
    M.sbtn = 1;
    M.x0 = M.lastx = x;
    M.y0 = M.lasty = y;
    switch (M.tool) {
        case T_PEN: before_change(); stamp(x, y); break;
        case T_RECT:
            before_change();
            res_free(RES_MAP, M.base);
            M.base = res_copy(RES_MAP, MAP);
            fill_rect(x, y, x, y);
            break;
        case T_FILL: before_change(); flood(x, y); M.stroking = false; break;
        case T_SELECT: M.sel = true; M.sx0 = M.sx1 = x; M.sy0 = M.sy1 = y; break;
    }
}

/* ---------------------------------------------------------------- drawing */

static int rep[NTILES];          /* each tile's most common color, for the small zooms */

static void find_rep_colors(void) {
    for (int t = 0; t < NTILES; t++) {
        rep[t] = -1;
        if (!cur_tiles) continue;
        int count[NCOLORS] = {0}, bestn = 0;
        const uint8_t *px = cur_tiles->t[t];
        for (int k = 0; k < TILE * TILE; k++)
            if (px[k] != CLEAR && ++count[px[k]] > bestn) { bestn = count[px[k]]; rep[t] = px[k]; }
    }
}

static void draw_cell_tile(int t, int sx, int sy, int size) {
    if (t == MAP_EMPTY || t >= NTILES || !cur_tiles) return;
    const uint8_t *px = cur_tiles->t[t];
    switch (size) {
        case 10: draw_sprite(t, sx, sy, 1, 1, 0); break;
        case 20:
            for (int j = 0; j < TILE; j++)
                for (int i = 0; i < TILE; i++)
                    if (px[j * TILE + i] != CLEAR) draw_rect(sx + i * 2, sy + j * 2, 2, 2, px[j * TILE + i], true);
            break;
        case 5:
            for (int j = 0; j < 5; j++)
                for (int i = 0; i < 5; i++)
                    if (px[j * 2 * TILE + i * 2] != CLEAR) draw_plot(sx + i, sy + j, px[j * 2 * TILE + i * 2]);
            break;
        default:
            if (rep[t] >= 0) draw_rect(sx, sy, size, size, rep[t], true);
    }
}

static void draw_brush_at(int cx, int cy) {
    int s = cs();
    for (int j = 0; j < M.bh; j++)
        for (int i = 0; i < M.bw; i++) {
            int t = M.brush[j][i];
            int sx = screen_x(cx + i), sy = screen_y(cy + j);
            if (t == MAP_EMPTY) draw_checker(sx, sy, s, s);
            else { draw_rect(sx, sy, s, s, COL_BLACK, true); draw_cell_tile(t, sx, sy, s); }
        }
    draw_rect(screen_x(cx) - 1, screen_y(cy) - 1, M.bw * s + 2, M.bh * s + 2, COL_WHITE, false);
}

static void draw_picker(void) {
    draw_rect(0, VIEW_Y, UI_W, VIEW_H, UI_RGB(0, 0, 1), true);
    char title[96];
    snprintf(title, sizeof title, "pick tiles from %s: drag for a block", doc_active(RES_TILES)->name);
    draw_text(title, 2, VIEW_Y, UI_RGB(2, 3, 3));
    draw_clip(0, PK_Y, UI_W, PK_H);
    int ox = 2 - M.pk_ox, oy = PK_Y - M.pk_oy;
    for (int t = 0; t < NTILES; t++) {
        int x = ox + (t % SHEET_COLS) * TILE, y = oy + (t / SHEET_COLS) * TILE;
        if (x >= UI_W || y >= PK_Y + PK_H || x + TILE <= 0 || y + TILE <= PK_Y) continue;
        if (((t % SHEET_COLS) + (t / SHEET_COLS)) & 1) draw_rect(x, y, TILE, TILE, UI_RGB(0, 0, 0), true);
        draw_sprite(t, x, y, 1, 1, 0);
    }
    int t1 = picker_tile();
    int c0, r0, c1 = t1 % SHEET_COLS, r1 = t1 / SHEET_COLS;
    if (M.pk_drag) {
        c0 = M.pk_t0 % SHEET_COLS; r0 = M.pk_t0 / SHEET_COLS;
        if (c0 > c1) { int t = c0; c0 = c1; c1 = t; }
        if (r0 > r1) { int t = r0; r0 = r1; r1 = t; }
    } else { c0 = c1; r0 = r1; }
    draw_rect(ox + c0 * TILE - 1, oy + r0 * TILE - 1, (c1 - c0 + 1) * TILE + 2, (r1 - r0 + 1) * TILE + 2, COL_YELLOW, false);
    draw_clip_reset();
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    find_rep_colors();
    int s = cs();
    draw_clip(0, VIEW_Y, UI_W, VIEW_H);

    /* outside the map: dark stripes */
    int cx0 = cell_x(0), cy0 = cell_y(VIEW_Y);
    int cols = UI_W / s + 2, rows = VIEW_H / s + 2;
    for (int y = 0; y < VIEW_H; y++)
        for (int x = (y % 4); x < UI_W; x += 4) {
            int cx = cell_x(x), cy = cell_y(VIEW_Y + y);
            if (!on_map(cx, cy)) draw_plot(x, VIEW_Y + y, UI_RGB(0, 0, 1));
        }
    for (int j = 0; j < rows; j++)
        for (int i = 0; i < cols; i++) {
            int cx = cx0 + i, cy = cy0 + j;
            if (!on_map(cx, cy)) continue;
            int t = *cellp(cx, cy);
            int sx = screen_x(cx), sy = screen_y(cy);
            if (t != MAP_EMPTY) draw_cell_tile(t, sx, sy, s);
            else draw_checker(sx, sy, s, s);
        }

    /* screen-sized guides (40 x 24 cells) and the map's edge */
    if (M.grid)
        for (int k = 0; k <= MAP->w; k += SCREEN_CELLS_W) draw_line(screen_x(k), screen_y(0), screen_x(k), screen_y(MAP->h), UI_RGB(0, 1, 2));
    if (M.grid)
        for (int k = 0; k <= MAP->h; k += SCREEN_CELLS_H) draw_line(screen_x(0), screen_y(k), screen_x(MAP->w), screen_y(k), UI_RGB(0, 1, 2));
    draw_rect(screen_x(0) - 1, screen_y(0) - 1, MAP->w * s + 2, MAP->h * s + 2, UI_RGB(1, 2, 3), false);

    /* selection, brush preview */
    if (M.sel) {
        int x0, y0, x1, y1;
        norm_sel(&x0, &y0, &x1, &y1);
        int c = (frame_count / 8) % 2 ? COL_YELLOW : COL_ORANGE;
        draw_rect(screen_x(x0), screen_y(y0), (x1 - x0 + 1) * s, (y1 - y0 + 1) * s, c, false);
    }
    int mx = cell_x(in.mouse_x), my = cell_y(in.mouse_y);
    bool hover = in_view() && !M.picker && !M.panning;
    if (hover && !M.stroking && M.tool != T_SELECT) draw_brush_at(mx, my);
    else if (hover && M.tool == T_SELECT && !M.stroking) draw_rect(screen_x(mx), screen_y(my), s, s, COL_WHITE, false);
    draw_clip_reset();

    if (M.picker) draw_picker();

    /* bars */
    ui_draw_tabs();
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
    draw_rect(2, by, 10, 10, COL_BLACK, true);
    draw_cell_tile(M.brush[0][0], 2, by, 10);
    char info[96];
    if (M.bw > 1 || M.bh > 1) snprintf(info, sizeof info, "%dx%d %s", M.bw, M.bh, tool_names[M.tool]);
    else if (M.brush[0][0] == MAP_EMPTY) snprintf(info, sizeof info, "erase %s", tool_names[M.tool]);
    else snprintf(info, sizeof info, "%d %s", M.brush[0][0], tool_names[M.tool]);
    int x = draw_text(info, 15, by, COL_WHITE);
    if (M.status[0] && frame_count < M.status_until) {
        draw_text(M.status, x + 8, by, M.status_color);
        if (ui_status_close(by)) M.status[0] = 0;
    } else if (hover && on_map(mx, my)) {
        uint16_t t = *cellp(mx, my);
        if (t == MAP_EMPTY) snprintf(info, sizeof info, "x%d y%d empty", mx, my);
        else snprintf(info, sizeof info, "x%d y%d = %d", mx, my, t);
        draw_text(info, x + 8, by, UI_RGB(2, 3, 3));
    }
    Doc *d = doc_active(RES_MAP);
    snprintf(info, sizeof info, "%s%s %dx%d", d->name, d->dirty ? "*" : "", MAP->w / SCREEN_CELLS_W, MAP->h / SCREEN_CELLS_H);
    draw_text(info, UI_W - STATUS_CLOSE_W - 2 - text_width(info), by, UI_RGB(2, 3, 3));
}

void map_frame(void) {
    init();
    if (!cart.loaded || !MAP) { ui_switch(MODE_SHELL); return; }
    follow_tiles();
    for (int i = 0; i < in.nevents && ui_mode == MODE_MAP; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        if (!M.picker && ui_global_key(ev)) continue;
        key(ev);
    }
    if (ui_mode != MODE_MAP) return;
    clamp_view();
    mouse();
    draw();
}
