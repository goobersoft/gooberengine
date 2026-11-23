/* The built-in tile editor.
 *
 * A tileset is 400 x 240 pixels (960 tiles), more than fits beside the file
 * tree, so the editor works like a paint program:
 *
 *   - the whole tileset is the canvas. Middle-drag (or Space + drag) pans it,
 *     and the wheel zooms it around the mouse.
 *   - the selected tile (or a block of tiles), boxed in yellow, is the only
 *     part that can be painted: a stroke stops at its edge. Clicking outside
 *     it selects the tile clicked instead; Alt + drag selects every tile from
 *     where the drag started to where it is. Tile 0 is selected at first.
 *   - the selection is also the map editor's pen: switch to the map and it
 *     paints with that tile, or stamps the whole block.
 *   - the preview window shows the selection by itself, at a size that fits.
 *     Drag it by its title bar; drag its bottom-right corner to resize it.
 *   - the tools, actions and the 64 colors (4 to a row) are in another,
 *     narrow window, which can also be dragged around.
 *
 * Transparent pixels show as a pink checkerboard (colors 50 and 34). The
 * left button draws with color A. The right button picks the color under
 * the mouse up into A (an eyedropper), or, with the "right click" setting on
 * "color B", draws with color B (transparent at first, so it erases). A
 * middle click (without dragging) picks a color up too. Edits go straight into the open tileset, so Ctrl+R runs
 * them without saving; Ctrl+S writes tiles/NAME.g64. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define TOP FONT_H                  /* the background: between the bars */
#define BOT (SCREEN_H - FONT_H)
#define TITLE_H 9
#define MAX_UNDO 64
#define TOOLS_W 36
#define TOOLS_H 208
#define GRIP 6

enum { T_PENCIL, T_FILL, T_LINE, T_RECT, T_OVAL, T_PICK, NTOOLS };
static const char tool_keys[NTOOLS] = {'P', 'F', 'L', 'R', 'O', 'I'};
static bool has_alt(int t) { return t == T_FILL || t == T_RECT || t == T_OVAL; }
static const char *const tool_names[NTOOLS] = {"pencil (P)", "fill (F)", "line (L), Shift: snap to 45", "rect (R)", "oval (O)", "pick a color (I)"};
enum { A_FLIPX, A_FLIPY, A_TURN, A_CLEAR, NACTS };
static const char act_keys[NACTS] = {'H', 'V', 'T', 'X'};
static const char *const act_names[NACTS] = {"flip across (H)", "flip up-down (V)", "turn clockwise (T)", "clear the block (Del)"};
static const int zooms[] = {1, 2, 3, 4, 6, 8, 12, 16};
#define NZOOM 8
#define START_ZOOM 4                /* 6x: a tile is 60 screen pixels */
#define PREVIEW 82                  /* the preview window's starting size */

/* a floating window: (x, y) is its top-left, w x h its content below the title bar */
typedef struct { int x, y, w, h; } Win;
enum { W_TILE, W_TOOLS, NWIN };

static struct {
    bool ready;
    int sel;                 /* top-left tile of the block being edited */
    int bw, bh;              /* the block's size in tiles */
    int tool, prev_tool;
    bool alt[NTOOLS];        /* a right click on the button swaps: rect / oval filled, fill every pixel of the
                              * color rather than just the touching ones. Ctrl does the other for one use */
    int col_a, col_b;        /* 0..63 or CLEAR */
    bool grid;
    int zoom, ox, oy;        /* the background: zoom index, where the sheet's top-left is */
    Win win[NWIN];
    int top;                 /* the window drawn last (on top) */
    bool placed;             /* windows put in their starting places */
    int placed_w;            /* the editor's width then: the windows keep to its right edge */
    int drag;                /* 0 none, 1 window move, 2 window resize, 3 pan, 4 paint, 5 select tiles */
    int rs_t0, rs_sel, rs_bw, rs_bh;   /* Alt + drag: the tile it started on, and the selection before */
    int drag_win, drag_dx, drag_dy, pan_x, pan_y;
    int pan_btn, pan_sx, pan_sy;   /* the button panning, where it went down */
    bool pan_moved;          /* a middle click that didn't move picks a color instead */
    int stroke_btn, x0, y0, lastx, lasty;
    Tileset *base;           /* the tileset when a line/rect drag started */
    Tileset *undo[MAX_UNDO], *redo[MAX_UNDO];
    int nundo, nredo;
    uint8_t clip[SHEET_W * SHEET_H];   /* clip_w x clip_h tiles of pixels, row by row */
    int clip_w, clip_h;      /* 0 = empty */
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
    T.sel = 0;
    T.bw = T.bh = 1;
    T.col_a = COL_WHITE;
    T.col_b = CLEAR;
    T.grid = true;
    T.zoom = START_ZOOM;
    T.ox = 0;
    T.oy = TOP;
    T.top = W_TOOLS;
}

/* windows start at the right of the editor, the tools right-most */
static void place_windows(void) {
    T.win[W_TOOLS] = (Win){UI_W - TOOLS_W - 2, TOP + 2, TOOLS_W, TOOLS_H};
    T.win[W_TILE] = (Win){UI_W - TOOLS_W - 2 - PREVIEW - 4, TOP + 2, PREVIEW, PREVIEW};
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

static int bpw(void) { return T.bw * TILE; }     /* the block's size in pixels */
static int bph(void) { return T.bh * TILE; }

static void clamp_sel(void) {
    if (T.bw > SHEET_COLS) T.bw = SHEET_COLS;
    if (T.bh > SHEET_ROWS) T.bh = SHEET_ROWS;
    if (T.bw < 1) T.bw = 1;
    if (T.bh < 1) T.bh = 1;
    int col = T.sel % SHEET_COLS, row = T.sel / SHEET_COLS;
    int maxc = SHEET_COLS - T.bw, maxr = SHEET_ROWS - T.bh;
    if (col > maxc) col = maxc;
    if (row > maxr) row = maxr;
    if (col < 0) col = 0;
    if (row < 0) row = 0;
    T.sel = row * SHEET_COLS + col;
}

/* the selection, for the map editor's pen: its top-left tile and size in tiles */
void tiles_selection(int *tile, int *w, int *h) {
    init();
    *tile = T.sel;
    *w = T.bw;
    *h = T.bh;
}

/* the map editor picked w x h tiles from `tile`: select them here too */
void tiles_select(int tile, int w, int h) {
    init();
    if (tile < 0 || tile >= NTILES) return;
    T.sel = tile;
    T.bw = w;
    T.bh = h;
    clamp_sel();
}

static uint8_t *bpix(int x, int y) {
    int t = T.sel + (y / TILE) * SHEET_COLS + x / TILE;
    return &TS->t[t][(y % TILE) * TILE + x % TILE];
}

static int bget(int x, int y) {
    if (x < 0 || y < 0 || x >= bpw() || y >= bph()) return -1;
    return *bpix(x, y);
}

static void bset(int x, int y, int c) {
    if (x < 0 || y < 0 || x >= bpw() || y >= bph()) return;
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

/* a row of pixels from x0 to x1 */
static void bspan(int x0, int x1, int y, int c) {
    for (int x = x0; x <= x1; x++) bset(x, y, c);
}

/* the oval that fits the box with corners (x0, y0) and (x1, y1), so an even
 * width or height comes out symmetric too (A. Zingl's midpoint ellipse) */
static void boval(int x0, int y0, int x1, int y1, int c, bool fill) {
    long a = labs((long)x1 - x0), b = labs((long)y1 - y0), b1 = b & 1;
    long dx = 4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a;   /* the error's steps */
    long err = dx + dy + b1 * a * a, e2;
    if (x0 > x1) { x0 = x1; x1 += (int)a; }
    if (y0 > y1) y0 = y1;
    y0 += (int)((b + 1) / 2);         /* start at the middle row(s), at the left and right ends */
    y1 = y0 - (int)b1;
    a *= 8 * a;
    b1 = 8 * b * b;
    do {
        if (fill) { bspan(x0, x1, y0, c); bspan(x0, x1, y1, c); }
        else { bset(x1, y0, c); bset(x0, y0, c); bset(x0, y1, c); bset(x1, y1, c); }
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);
    while (y0 - y1 <= b) {            /* a very thin oval stops early: finish its tips */
        if (fill) { bspan(x0 - 1, x1 + 1, y0++, c); bspan(x0 - 1, x1 + 1, y1--, c); }
        else { bset(x0 - 1, y0, c); bset(x1 + 1, y0++, c); bset(x0 - 1, y1, c); bset(x1 + 1, y1--, c); }
    }
}

/* paint over the color at (x, y): the pixels of it that touch, or with all, every one in the block */
static void bfill(int x, int y, int c, bool all) {
    int from = bget(x, y);
    if (from < 0 || from == c) return;
    if (all) {
        for (int py = 0; py < bph(); py++)
            for (int px = 0; px < bpw(); px++)
                if (bget(px, py) == from) bset(px, py, c);
        return;
    }
    /* a pixel is painted as it goes on the stack, so none goes on twice: the
     * whole sheet is the most it can ever hold */
    static int16_t stack[SHEET_W * SHEET_H][2];
    int n = 0;
    bset(x, y, c);
    stack[n][0] = (int16_t)x; stack[n][1] = (int16_t)y; n++;
    while (n > 0) {
        n--;
        int px = stack[n][0], py = stack[n][1];
        static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int k = 0; k < 4; k++) {
            int qx = px + d[k][0], qy = py + d[k][1];
            if (bget(qx, qy) == from) {
                bset(qx, qy, c);
                stack[n][0] = (int16_t)qx; stack[n][1] = (int16_t)qy; n++;
            }
        }
    }
}

/* whole-block transforms: read the block out, write it back moved */
static void transform(int kind, int dx, int dy) {
    int w = bpw(), h = bph();          /* turning ('t') needs w == h: act() checks */
    static uint8_t tmp[SHEET_H][SHEET_W];
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) tmp[y][x] = (uint8_t)bget(x, y);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int sx = x, sy = y;
            if (kind == 'h') sx = w - 1 - x;
            else if (kind == 'v') sy = h - 1 - y;
            else if (kind == 't') { sx = y; sy = h - 1 - x; }          /* rotate clockwise */
            else if (kind == 's') { sx = (x - dx + w) % w; sy = (y - dy + h) % h; }
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
    init();
    T.sel = 0;                /* a new tileset starts at tile 0 (the block size stays) */
    clamp_sel();
    clear_stack(T.undo, &T.nundo);
    clear_stack(T.redo, &T.nredo);
    if (T.drag == 4) T.drag = 0;
}

/* IMPORT file.png [name]: the picture becomes tiles/NAME, a new tileset or the
 * pixels of the one with that name. Either way it's an unsaved change the editor
 * shows (Ctrl+Z puts an existing tileset back, Ctrl+S keeps it). */
bool tiles_import(const char *path, const char *name, char *msg, int msglen) {
    char nm[64], info[256];
    if (name && *name) snprintf(nm, sizeof nm, "%s", name);
    else {                                        /* the file's name, made a file name: "Forest Tiles.png" -> Forest-Tiles */
        const char *b = path;
        for (const char *p = path; *p; p++) if (*p == '/' || *p == '\\') b = p + 1;
        size_t j = 0;
        for (; *b && j < sizeof nm - 1; b++) {
            if (!strcasecmp(b, ".png")) break;
            nm[j++] = isalnum((unsigned char)*b) || *b == '_' ? *b : '-';
        }
        nm[j] = 0;
    }
    if (!res_name_ok(nm)) { snprintf(msg, (size_t)msglen, "'%s' is not a file name (letters, digits, - and _): IMPORT %s name", nm, path); return false; }
    Tileset *img = malloc(sizeof *img);
    if (!img) { snprintf(msg, (size_t)msglen, "out of memory"); return false; }
    if (!png_to_tileset(path, img, info, sizeof info)) { snprintf(msg, (size_t)msglen, "%s", info); free(img); return false; }
    char err[512];
    char path_g64[PATH_MAX + 128];
    res_path(RES_TILES, nm, path_g64, sizeof path_g64);
    FILE *f = fopen(path_g64, "r");
    if (f) fclose(f);
    Doc *d = doc_find(RES_TILES, nm);
    bool fresh = !d && !f;
    if (!d) d = fresh ? doc_create(RES_TILES, nm, err, sizeof err) : doc_open(RES_TILES, nm, err, sizeof err);
    if (!d) { snprintf(msg, (size_t)msglen, "%s", err); free(img); return false; }
    if (cart.active[RES_TILES] != d) { doc_set_active(d); docs_bind(); tiles_doc_changed(); }
    before_change();                              /* Ctrl+Z gets the old pixels back */
    memcpy(TS, img, sizeof *TS);
    free(img);
    ui_switch(MODE_TILES);
    snprintf(msg, (size_t)msglen, "imported %s tiles/%s: %s (Ctrl+S saves it)", fresh ? "into a new" : "into", nm, info);
    status(COL_GREEN, "imported tiles/%s: Ctrl+S saves it", nm);
    return true;
}

/* ---------------------------------------------------------------- commands */

static void save(void) {
    char err[256];
    Doc *d = doc_active(RES_TILES);
    if (doc_save(d, err, sizeof err)) status(COL_GREEN, "saved tiles/%s.g64", d->name);
    else status(COL_RED, "%s", err);
}

static void copy_block(bool cut) {
    int w = bpw(), h = bph();
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) T.clip[y * w + x] = (uint8_t)bget(x, y);
    T.clip_w = T.bw;
    T.clip_h = T.bh;
    if (cut) {
        before_change();
        brect(0, 0, w - 1, h - 1, CLEAR, true);
    }
    status(COL_GREY, "%s %dx%d tile%s", cut ? "cut" : "copied", T.bw, T.bh, T.bw * T.bh > 1 ? "s" : "");
}

/* the copied tiles go in with their top-left at the selection's; what would be off the sheet is left out */
static void paste_block(void) {
    if (!T.clip_w) { status(COL_GREY, "nothing copied yet"); return; }
    before_change();
    int w = T.clip_w * TILE, h = T.clip_h * TILE;
    int col = T.sel % SHEET_COLS, row = T.sel / SHEET_COLS;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int tc = col + x / TILE, tr = row + y / TILE;
            if (tc >= SHEET_COLS || tr >= SHEET_ROWS) continue;
            TS->t[tr * SHEET_COLS + tc][(y % TILE) * TILE + x % TILE] = T.clip[y * w + x];
        }
}

static void set_tool(int t) {
    if (t == T_PICK && T.tool != T_PICK) T.prev_tool = T.tool;
    T.tool = t;
}

static void act(int a) {
    switch (a) {
        case A_FLIPX: before_change(); transform('h', 0, 0); break;
        case A_FLIPY: before_change(); transform('v', 0, 0); break;
        case A_TURN:
            if (T.bw != T.bh) { status(COL_GREY, "only a square block can turn (this one is %dx%d)", T.bw, T.bh); break; }
            before_change(); transform('t', 0, 0); break;
        case A_CLEAR: before_change(); brect(0, 0, bpw() - 1, bph() - 1, CLEAR, true); break;
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

/* scroll the sheet the least that brings the whole selection into view */
static void show_selection(void) {
    int z = zooms[T.zoom], w = bpw() * z, h = bph() * z;
    int x = T.ox + (T.sel % SHEET_COLS) * TILE * z, y = T.oy + (T.sel / SHEET_COLS) * TILE * z;
    if (x + w > UI_W) T.ox -= x + w - UI_W;
    if (y + h > BOT) T.oy -= y + h - BOT;
    x = T.ox + (T.sel % SHEET_COLS) * TILE * z;
    y = T.oy + (T.sel / SHEET_COLS) * TILE * z;
    if (x < 0) T.ox -= x;
    if (y < TOP) T.oy += TOP - y;
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
        case SDL_SCANCODE_O: set_tool(T_OVAL); return;
        case SDL_SCANCODE_I: set_tool(T_PICK); return;
        case SDL_SCANCODE_G: T.grid = !T.grid; return;
        case SDL_SCANCODE_X: { int t = T.col_a; T.col_a = T.col_b; T.col_b = t; return; }
        case SDL_SCANCODE_H: act(A_FLIPX); return;
        case SDL_SCANCODE_V: act(A_FLIPY); return;
        case SDL_SCANCODE_T: act(A_TURN); return;
        case SDL_SCANCODE_DELETE: case SDL_SCANCODE_BACKSPACE: act(A_CLEAR); return;
        case SDL_SCANCODE_EQUALS: case SDL_SCANCODE_KP_PLUS: zoom_at(1, UI_W / 2, (TOP + BOT) / 2); return;
        case SDL_SCANCODE_MINUS: case SDL_SCANCODE_KP_MINUS: zoom_at(-1, UI_W / 2, (TOP + BOT) / 2); return;
        case SDL_SCANCODE_HOME: T.zoom = START_ZOOM; T.ox = 0; T.oy = TOP; place_windows(); show_selection(); return;
        case SDL_SCANCODE_LEFT: dx = -1; break;
        case SDL_SCANCODE_RIGHT: dx = 1; break;
        case SDL_SCANCODE_UP: dy = -1; break;
        case SDL_SCANCODE_DOWN: dy = 1; break;
        default: return;
    }
    if (shift) { before_change(); transform('s', dx, dy); return; }   /* nudge the pixels, wrapping */
    int col = T.sel % SHEET_COLS + dx, row = T.sel / SHEET_COLS + dy;
    if (col < 0 || row < 0 || col > SHEET_COLS - T.bw || row > SHEET_ROWS - T.bh) return;
    T.sel = row * SHEET_COLS + col;
    show_selection();
}

/* ---------------------------------------------------------------- the windows' insides */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

static bool in_win(int i) { const Win *w = &T.win[i]; return inside(w->x, w->y, w->w, TITLE_H + w->h); }

/* the preview window: the selection at the biggest whole scale that fits, centred */
static int preview_scale(void) {
    const Win *w = &T.win[W_TILE];
    int sx = w->w / bpw(), sy = w->h / bph(), s = sx < sy ? sx : sy;
    return s < 1 ? 1 : s;
}
static void preview_origin(int *cx, int *cy) {
    const Win *w = &T.win[W_TILE];
    int s = preview_scale();
    *cx = w->x + (w->w - s * bpw()) / 2;
    *cy = w->y + TITLE_H + (w->h - s * bph()) / 2;
}

/* the tools window's parts, top to bottom: tool buttons and action buttons
 * (3 to a row, 2 rows each), the palette (4 colors to a row), the transparent
 * swatch, then A and B with their indices */
#define BTN_COLS 3
#define BTN_STEP 11
static int btn_at(int i, int row0, int *x, int *y) {
    *x = T.win[W_TOOLS].x + 2 + (i % BTN_COLS) * BTN_STEP;
    *y = T.win[W_TOOLS].y + TITLE_H + 1 + (row0 + i / BTN_COLS) * BTN_STEP;
    return BTN_STEP - 1;
}
static int tool_btn(int i, int *x, int *y) { return btn_at(i, 0, x, y); }
static int act_btn(int i, int *x, int *y) { return btn_at(i, 2, x, y); }
#define PAL_CELL 8
#define PAL_COLS 4
#define PAL_ROWS (NCOLORS / PAL_COLS)
#define PAL_W (PAL_COLS * PAL_CELL)
#define PAL_H (PAL_ROWS * PAL_CELL)
static void pal_origin(int *x, int *y) { *x = T.win[W_TOOLS].x + 2; *y = T.win[W_TOOLS].y + TITLE_H + 3 + 4 * BTN_STEP; }

static int floor_div(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* the sheet pixel at screen (mx, my), in the selected block's coordinates: true
 * if it is inside the block (and so may be painted) */
static bool block_px_at(int mx, int my, int *bx, int *by) {
    int z = zooms[T.zoom];
    *bx = floor_div(mx - T.ox, z) - (T.sel % SHEET_COLS) * TILE;
    *by = floor_div(my - T.oy, z) - (T.sel / SHEET_COLS) * TILE;
    return *bx >= 0 && *by >= 0 && *bx < bpw() && *by < bph();
}

static int sheet_tile_at(int mx, int my) {     /* -1 if not on the sheet */
    int z = zooms[T.zoom];
    int px = mx - T.ox, py = my - T.oy;
    if (px < 0 || py < 0 || px >= SHEET_W * z || py >= SHEET_H * z) return -1;
    return (py / z / TILE) * SHEET_COLS + px / z / TILE;
}

/* pick up the color under the mouse, from anywhere on the sheet: reading can't hurt */
static void pick_color(bool into_b) {
    int z = zooms[T.zoom], px = in.mouse_x - T.ox, py = in.mouse_y - T.oy;
    if (px < 0 || py < 0 || px >= SHEET_W * z || py >= SHEET_H * z) return;
    px /= z;
    py /= z;
    int c = TS->t[(py / TILE) * SHEET_COLS + px / TILE][(py % TILE) * TILE + px % TILE];
    if (into_b) T.col_b = c; else T.col_a = c;
}

static bool ctrl_held(void) { return in.key[SDL_SCANCODE_LCTRL] || in.key[SDL_SCANCODE_RCTRL]; }

/* a left or right press on the sheet: paint inside the selection, select outside it.
 * With the eyedrop setting the right button picks a color up into A instead. */
static void sheet_press(int btn) {
    if (T.tool == T_PICK) { pick_color(btn == 2); T.tool = T.prev_tool; return; }
    if (btn == 2 && settings.tile_right == RIGHT_EYEDROP) { pick_color(false); return; }
    int x, y;
    if (!block_px_at(in.mouse_x, in.mouse_y, &x, &y)) {
        int t = sheet_tile_at(in.mouse_x, in.mouse_y);
        if (t >= 0) { T.sel = t; clamp_sel(); }
        return;
    }
    int c = btn == 1 ? T.col_a : T.col_b;
    before_change();
    if (T.tool == T_FILL) { bfill(x, y, c, T.alt[T_FILL] != ctrl_held()); return; }
    res_free(RES_TILES, T.base);
    T.base = res_copy(RES_TILES, TS);
    T.drag = 4;
    T.stroke_btn = btn;
    T.x0 = T.lastx = x;
    T.y0 = T.lasty = y;
    if (T.tool == T_PENCIL) bset(x, y, c);
}

/* Shift keeps a shape 1:1: a line snaps to across, up-down or 45 degrees, and a
 * rect or oval's box becomes a square, as big as the drag's shorter side */
static void snap(int *x, int *y) {
    int dx = *x - T.x0, dy = *y - T.y0, ax = abs(dx), ay = abs(dy);
    int d = ax < ay ? ax : ay;
    if (T.tool == T_LINE) {
        if (ay * 5 < ax * 2) { *y = T.y0; return; }    /* under about 22 degrees: across */
        if (ax * 5 < ay * 2) { *x = T.x0; return; }    /* and up-down */
        d = ax > ay ? ax : ay;                         /* a diagonal follows the mouse */
    }
    *x = T.x0 + (dx < 0 ? -d : d);
    *y = T.y0 + (dy < 0 ? -d : d);
}

/* a stroke goes on while the button is held; bset leaves out what falls outside the block */
static void sheet_drag(void) {
    int x, y;
    block_px_at(in.mouse_x, in.mouse_y, &x, &y);
    if (x < -1) x = -1;               /* just past the edge is enough to reach it */
    if (y < -1) y = -1;
    if (x > bpw()) x = bpw();
    if (y > bph()) y = bph();
    int c = T.stroke_btn == 1 ? T.col_a : T.col_b;
    if (T.tool == T_PENCIL) {
        bline(T.lastx, T.lasty, x, y, c);
    } else {
        memcpy(TS, T.base, sizeof *TS);
        bool fill = T.alt[T.tool] != ctrl_held();
        if (in.key[SDL_SCANCODE_LSHIFT] || in.key[SDL_SCANCODE_RSHIFT]) snap(&x, &y);
        if (T.tool == T_LINE) bline(T.x0, T.y0, x, y, c);
        else if (T.tool == T_OVAL) boval(T.x0, T.y0, x, y, c, fill);
        else brect(T.x0, T.y0, x, y, c, fill);
    }
    T.lastx = x;
    T.lasty = y;
}

static bool alt_held(void) { return in.key[SDL_SCANCODE_LALT] || in.key[SDL_SCANCODE_RALT]; }

/* the sheet's tile column and row at the mouse, kept on the sheet */
static void sheet_cell_at(int *col, int *row) {
    int z = zooms[T.zoom];
    *col = floor_div(in.mouse_x - T.ox, z * TILE);
    *row = floor_div(in.mouse_y - T.oy, z * TILE);
    if (*col < 0) *col = 0;
    if (*row < 0) *row = 0;
    if (*col >= SHEET_COLS) *col = SHEET_COLS - 1;
    if (*row >= SHEET_ROWS) *row = SHEET_ROWS - 1;
}

/* Alt + drag: the tiles from the one the drag started on to the one under the
 * mouse. Back on the first tile, the selection is what it was before. */
static void region_drag(void) {
    int c1, r1;
    sheet_cell_at(&c1, &r1);
    int c0 = T.rs_t0 % SHEET_COLS, r0 = T.rs_t0 / SHEET_COLS;
    if (c1 == c0 && r1 == r0) { T.sel = T.rs_sel; T.bw = T.rs_bw; T.bh = T.rs_bh; return; }
    T.sel = (r0 < r1 ? r0 : r1) * SHEET_COLS + (c0 < c1 ? c0 : c1);
    T.bw = abs(c1 - c0) + 1;
    T.bh = abs(r1 - r0) + 1;
}

static void tools_press(int btn) {
    int x, y, w;
    for (int i = 0; i < NTOOLS; i++) {
        w = tool_btn(i, &x, &y);
        if (!inside(x, y, w, 10)) continue;
        if (btn == 2 && has_alt(i)) T.alt[i] = !T.alt[i];   /* a right click: filled <-> outline, all <-> touching */
        set_tool(i);
        return;
    }
    for (int i = 0; i < NACTS; i++) { w = act_btn(i, &x, &y); if (inside(x, y, w, 10)) { act(i); return; } }
    pal_origin(&x, &y);
    if (inside(x, y, PAL_W, PAL_H)) {
        int c = ((in.mouse_y - y) / PAL_CELL) * PAL_COLS + (in.mouse_x - x) / PAL_CELL;
        if (btn == 2) T.col_b = c; else T.col_a = c;
        return;
    }
    if (inside(x, y + PAL_H + 2, PAL_W, 10)) { if (btn == 2) T.col_b = CLEAR; else T.col_a = CLEAR; }
}

/* ---------------------------------------------------------------- mouse */

static void mouse(void) {
    bool space = in.key[SDL_SCANCODE_SPACE];
    if (T.drag) {
        int held = T.drag == 4 ? T.stroke_btn : T.drag == 3 ? T.pan_btn : 1;   /* 1, 2 and 5: the left button */
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
                case 3:
                    T.ox = in.mouse_x - T.pan_x; T.oy = in.mouse_y - T.pan_y; clamp_pan();
                    if (abs(in.mouse_x - T.pan_sx) > 2 || abs(in.mouse_y - T.pan_sy) > 2) T.pan_moved = true;
                    break;
                case 4: sheet_drag(); break;
                case 5: region_drag(); break;
            }
            return;
        }
        if (T.drag == 3 && T.pan_btn == 3 && !T.pan_moved) {     /* a middle click: pick the color */
            T.ox = in.mouse_x - T.pan_x; T.oy = in.mouse_y - T.pan_y;
            pick_color(false);
        }
        if (T.drag == 5) { region_drag(); show_selection(); }
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
        if (i == W_TOOLS) tools_press(btn);      /* the preview is only to look at */
        return;
    }

    /* the background */
    if (in.mouse_y < TOP || in.mouse_y >= BOT) return;
    if (in.wheel) { zoom_at(in.wheel > 0 ? 1 : -1, in.mouse_x, in.mouse_y); clamp_pan(); }
    if (btn == 3 || (btn == 1 && space)) {
        T.drag = 3;
        T.pan_btn = btn;
        T.pan_x = in.mouse_x - T.ox;
        T.pan_y = in.mouse_y - T.oy;
        T.pan_sx = in.mouse_x;
        T.pan_sy = in.mouse_y;
        T.pan_moved = false;
        return;
    }
    if (btn == 1 && alt_held()) {
        int t = sheet_tile_at(in.mouse_x, in.mouse_y);
        if (t < 0) return;
        T.drag = 5;
        T.rs_t0 = t;
        T.rs_sel = T.sel; T.rs_bw = T.bw; T.rs_bh = T.bh;
        return;
    }
    if (btn == 1 || btn == 2) sheet_press(btn);
}

/* ---------------------------------------------------------------- drawing */

#define checker draw_checker        /* transparent: pink checks, 5 screen pixels */

static void swatch(int x, int y, int w, int h, int c) {
    if (c == CLEAR) checker(x, y, w, h);
    else draw_rect(x, y, w, h, c, true);
}

static void color_name(int c, char *buf, size_t n) {
    if (c == CLEAR) snprintf(buf, n, "clear");
    else snprintf(buf, n, "%d (r%d g%d b%d)", c, c >> 4, (c >> 2) & 3, c & 3);
}

/* hx, hy: the selection's pixel under the mouse, or -1 */
static void draw_background(int *hx, int *hy) {
    int z = zooms[T.zoom];
    bool over_win = in_win(W_TILE) || in_win(W_TOOLS);
    if (over_win || in.mouse_y < TOP || in.mouse_y >= BOT || T.tool == T_PICK || !block_px_at(in.mouse_x, in.mouse_y, hx, hy))
        *hx = *hy = -1;
    draw_clip(0, TOP, UI_W, BOT - TOP);
    draw_rect(0, TOP, UI_W, BOT - TOP, UI_RGB(0, 0, 1), true);
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
            else if (z == 1) draw_plot(sx, sy, c);
            else draw_rect(sx, sy, z, z, c, true);
        }
    int scol = T.sel % SHEET_COLS, srow = T.sel / SHEET_COLS, nw = bpw(), nh = bph(), btw = nw * z, bth = nh * z;
    int bx0 = T.ox + scol * TILE * z, by0 = T.oy + srow * TILE * z;
    if (T.grid && z >= 2) {    /* lines between tiles, but not across the selection: a block reads as one picture */
        int sheet_w = SHEET_W * z, sheet_h = SHEET_H * z;
        for (int c = 0; c <= SHEET_COLS; c++) {
            int x = T.ox + c * TILE * z;
            if (c > scol && c < scol + T.bw) {
                draw_line(x, T.oy, x, by0 - 1, UI_RGB(1, 0, 1));
                draw_line(x, by0 + bth, x, T.oy + sheet_h - 1, UI_RGB(1, 0, 1));
            } else draw_line(x, T.oy, x, T.oy + sheet_h - 1, UI_RGB(1, 0, 1));
        }
        for (int r = 0; r <= SHEET_ROWS; r++) {
            int y = T.oy + r * TILE * z;
            if (r > srow && r < srow + T.bh) {
                draw_line(T.ox, y, bx0 - 1, y, UI_RGB(1, 0, 1));
                draw_line(bx0 + btw, y, T.ox + sheet_w - 1, y, UI_RGB(1, 0, 1));
            } else draw_line(T.ox, y, T.ox + sheet_w - 1, y, UI_RGB(1, 0, 1));
        }
    }
    draw_rect(T.ox - 1, T.oy - 1, SHEET_W * z + 2, SHEET_H * z + 2, UI_RGB(1, 2, 3), false);
    if (T.grid && z >= 6)               /* a faint pixel grid where painting happens, the same at tile edges */
    {
        for (int k = 1; k < nw; k++) draw_line(bx0 + k * z, by0, bx0 + k * z, by0 + bth - 1, UI_RGB(1, 1, 1));
        for (int k = 1; k < nh; k++) draw_line(bx0, by0 + k * z, bx0 + btw - 1, by0 + k * z, UI_RGB(1, 1, 1));
    }
    draw_rect(bx0 - 1, by0 - 1, btw + 2, bth + 2, COL_YELLOW, false);
    if (z >= 2 && *hx >= 0)              /* the pixel the mouse would paint */
        draw_rect(bx0 + *hx * z - 1, by0 + *hy * z - 1, z + 2, z + 2, COL_WHITE, false);
    draw_clip_reset();
}

static void window_frame(int i, const char *title) {
    const Win *w = &T.win[i];
    bool on = T.top == i;
    draw_rect(w->x - 1, w->y - 1, w->w + 2, TITLE_H + w->h + 2, on ? UI_RGB(1, 2, 3) : UI_RGB(1, 1, 2), false);
    draw_rect(w->x, w->y, w->w, TITLE_H, on ? UI_RGB(0, 1, 2) : UI_RGB(0, 1, 1), true);
    draw_rect(w->x, w->y + TITLE_H, w->w, w->h, UI_RGB(0, 0, 0), true);
    draw_clip(w->x, w->y, w->w, TITLE_H);
    draw_text(title, w->x + 2, w->y - 1, on ? COL_WHITE : UI_RGB(2, 2, 3));
    draw_clip_reset();
}

/* the selection by itself, as a cart would draw it */
static void draw_preview(void) {
    const Win *w = &T.win[W_TILE];
    char title[48];
    if (T.bw * T.bh > 1) snprintf(title, sizeof title, "tiles %d %dx%d", T.sel, T.bw, T.bh);
    else snprintf(title, sizeof title, "tile %d", T.sel);
    window_frame(W_TILE, title);
    int s = preview_scale(), nw = bpw(), nh = bph(), cx, cy;
    preview_origin(&cx, &cy);
    draw_clip(w->x, w->y + TITLE_H, w->w, w->h);
    for (int y = 0; y < nh; y++)
        for (int x = 0; x < nw; x++) {
            int c = bget(x, y);
            if (c == CLEAR) checker(cx + x * s, cy + y * s, s, s);
            else draw_rect(cx + x * s, cy + y * s, s, s, c, true);
        }
    if (T.top == W_TILE && in_win(W_TILE)) T.hint = "preview: paint on the tileset, in the yellow box";
    /* the resize grip */
    for (int k = 0; k < GRIP; k += 2) draw_line(w->x + w->w - 1 - k, w->y + TITLE_H + w->h - 1, w->x + w->w - 1, w->y + TITLE_H + w->h - 1 - k, UI_RGB(2, 2, 3));
    draw_clip_reset();
}

static void button(int x, int y, int w, char label, bool on, bool hover) {
    draw_rect(x, y, w, 10, on ? UI_RGB(1, 2, 3) : hover ? UI_RGB(0, 1, 2) : UI_RGB(0, 0, 1), true);
    draw_char(label, x + (w - FONT_INK_W) / 2, y, on ? COL_BLACK : UI_RGB(2, 3, 3));
}

static void draw_tools_window(void) {
    window_frame(W_TOOLS, "tools");
    int x, y, w;
    bool live = T.top == W_TOOLS || !in_win(W_TILE);
    for (int i = 0; i < NTOOLS; i++) {
        w = tool_btn(i, &x, &y);
        bool hover = live && inside(x, y, w, 10);
        button(x, y, w, tool_keys[i], T.tool == i, hover);
        if (T.alt[i]) draw_rect(x + w - 2, y + 8, 2, 2, T.tool == i ? COL_BLACK : UI_RGB(2, 3, 3), true);
        if (hover && i == T_FILL) {
            T.hint = T.alt[i] ? "fill all of a color  Ctrl: just touching  right: swap" : "fill (F)  Ctrl: all of the color  right: swap";
        } else if (hover && has_alt(i)) {
            static char hint[80];
            bool f = T.alt[i];
            snprintf(hint, sizeof hint, "%s%s  Ctrl: %s  Shift: %s  right: swap", f ? "filled " : "", i == T_RECT ? "rect" : "oval",
                     f ? "outline" : "filled", i == T_RECT ? "square" : "circle");
            T.hint = hint;
        } else if (hover) T.hint = tool_names[i];
    }
    for (int i = 0; i < NACTS; i++) {
        w = act_btn(i, &x, &y);
        bool hover = live && inside(x, y, w, 10);
        button(x, y, w, act_keys[i], false, hover);
        if (hover) T.hint = act_names[i];
    }
    int px, py;
    pal_origin(&px, &py);
    for (int i = 0; i < NCOLORS; i++)
        draw_rect(px + (i % PAL_COLS) * PAL_CELL, py + (i / PAL_COLS) * PAL_CELL, PAL_CELL - 1, PAL_CELL - 1, i, true);
    int cy = py + PAL_H + 2;
    swatch(px, cy, PAL_W - 1, 9, CLEAR);
    draw_rect(px - 1, cy - 1, PAL_W + 1, 11, UI_RGB(1, 1, 2), false);
    if (live && inside(px, cy, PAL_W, 10)) T.hint = "transparent";
    /* A and B: outlines on the palette, and swatches */
    for (int k = 0; k < 2; k++) {
        int c = k ? T.col_b : T.col_a;
        int ox = c == CLEAR ? px - 1 : px + (c % PAL_COLS) * PAL_CELL - 1;
        int oy = c == CLEAR ? cy - 1 : py + (c / PAL_COLS) * PAL_CELL - 1;
        draw_rect(ox, oy, c == CLEAR ? PAL_W + 1 : PAL_CELL + 1, c == CLEAR ? 11 : PAL_CELL + 1, k ? COL_GREY : COL_WHITE, false);
    }
    int ay = cy + 11;
    draw_text("A", px, ay, COL_WHITE);
    swatch(px + 6, ay, 9, 9, T.col_a);
    draw_text("B", px + 18, ay, COL_GREY);
    swatch(px + 24, ay, 9, 9, T.col_b);
    /* their palette indices, centred under each label+swatch pair; -1 is transparent */
    for (int k = 0; k < 2; k++) {
        int c = k ? T.col_b : T.col_a;
        char num[8];
        snprintf(num, sizeof num, "%d", c == CLEAR ? -1 : c);
        int bx = px + (k ? 18 : 0);
        draw_text(num, bx + (15 - text_width(num)) / 2, ay + 10, k ? COL_GREY : COL_WHITE);
    }
    if (live && inside(px, py, PAL_W, PAL_H)) T.hint = "left: color A  right: color B";
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    T.hint = NULL;
    int hx, hy;
    draw_background(&hx, &hy);
    if (T.top == W_TOOLS) { draw_preview(); draw_tools_window(); }
    else { draw_tools_window(); draw_preview(); }

    /* bars */
    ui_draw_tabs();
    int by = BOT;
    draw_rect(0, by, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
    Doc *d = doc_active(RES_TILES);
    char info[64], line[128];
    snprintf(info, sizeof info, "%s%s", d->name, d->dirty ? "*" : "");
    int x = draw_text(info, 3, by, UI_RGB(2, 3, 3)) + 8;
    bool over_win = in_win(W_TILE) || in_win(W_TOOLS);
    int ht = over_win || in.mouse_y < TOP || in.mouse_y >= BOT ? -1 : sheet_tile_at(in.mouse_x, in.mouse_y);
    if (T.status[0] && frame_count < T.status_until) {
        draw_text(T.status, x, by, T.status_color);
        if (ui_status_close(by)) T.status[0] = 0;
    } else if (hx >= 0) {
        char c[32];
        color_name(bget(hx, hy), c, sizeof c);
        snprintf(line, sizeof line, "tile %d x%d y%d %s", T.sel + (hy / TILE) * SHEET_COLS + hx / TILE, hx % TILE, hy % TILE, c);
        draw_text(line, x, by, UI_RGB(2, 3, 3));
    } else if (T.hint) draw_text(T.hint, x, by, UI_RGB(2, 3, 3));
    else if (ht >= 0) {
        snprintf(line, sizeof line, "tile %d (col %d row %d) x%d%s", ht, ht % SHEET_COLS, ht / SHEET_COLS, zooms[T.zoom],
                 T.tool == T_PICK ? "" : "  click, Alt+drag: select");
        draw_text(line, x, by, UI_RGB(2, 3, 3));
    } else draw_text("middle drag: pan  wheel: zoom", x, by, UI_RGB(1, 2, 2));
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
