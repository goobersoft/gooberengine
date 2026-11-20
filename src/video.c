/* Framebuffer drawing. Everything draws palette indices into fb; the
 * platform layer turns them into real colours once per frame.
 * All drawing goes through the camera offset and the clip rectangle.
 *
 * The dot limit: drawing is done in software, so a cart may only plot
 * dot_limit pixels per frame (flip() starts a new frame). Pixels clipped away
 * don't count, nor do a tile's transparent ones. Past the limit, pixels are
 * dropped until the next flip(), so a cart is rewarded for redrawing only
 * what changed. */
#include "engine.h"
#include <stdlib.h>
#include <string.h>

uint8_t fb[SCREEN_H][SCREEN_W];
DrawState ds;
int dot_limit = DOT_LIMIT_DEFAULT, dots_used;
int view_x = 0, view_w = SCREEN_W;

/* 6-bit colour: index = r*16 + g*4 + b, each channel 0..3 -> 0, 85, 170, 255 */
#define L(v) ((v) * 85)
#define P(i) ((uint32_t)L(((i) >> 4) & 3) << 16 | (uint32_t)L(((i) >> 2) & 3) << 8 | (uint32_t)L((i) & 3))
#define P4(i) P(i), P(i + 1), P(i + 2), P(i + 3)
#define P16(i) P4(i), P4(i + 4), P4(i + 8), P4(i + 12)
const uint32_t palette[NCOLORS] = {P16(0), P16(16), P16(32), P16(48)};

static inline int pen_or(int c) { return c < 0 ? ds.pen : (c & 63); }

void draw_reset(void) {
    ds.cam_x = -view_x;
    ds.cam_y = 0;
    ds.clip_x0 = view_x;
    ds.clip_y0 = 0;
    ds.clip_x1 = view_x + view_w;
    ds.clip_y1 = SCREEN_H;
    ds.pen = COL_WHITE;
    for (int i = 0; i < NCOLORS; i++) ds.pal[i] = (uint8_t)i;
}

/* the clip rectangle, in view coordinates (the whole screen for a cart) */
void draw_clip(int x, int y, int w, int h) {
    x += view_x;
    ds.clip_x0 = x < view_x ? view_x : x;
    ds.clip_y0 = y < 0 ? 0 : y;
    ds.clip_x1 = x + w > view_x + view_w ? view_x + view_w : x + w;
    ds.clip_y1 = y + h > SCREEN_H ? SCREEN_H : y + h;
}

void draw_clip_reset(void) { draw_clip(0, 0, view_w, SCREEN_H); }

/* how many of n pixels may still be drawn this frame */
static inline int dots(int n) {
    if (!ds.count_dots) return n;
    int left = dot_limit - dots_used;
    if (n > left) n = left < 0 ? 0 : left;
    dots_used += n;
    return n;
}

static void hspan(int x0, int x1, int y, uint8_t c);

/* clears the clip rectangle (the whole screen unless clip() was used) */
void draw_cls(int c) {
    uint8_t col = (uint8_t)(c < 0 ? 0 : (c & 63));
    for (int y = ds.clip_y0; y < ds.clip_y1; y++) hspan(ds.clip_x0, ds.clip_x1 - 1, y, col);
}

/* screen-space pixel, clipped */
static inline void put(int x, int y, uint8_t c) {
    if (x >= ds.clip_x0 && x < ds.clip_x1 && y >= ds.clip_y0 && y < ds.clip_y1 && dots(1)) fb[y][x] = ds.pal[c & 63];
}

static void hspan(int x0, int x1, int y, uint8_t c) {
    if (y < ds.clip_y0 || y >= ds.clip_y1) return;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (x0 < ds.clip_x0) x0 = ds.clip_x0;
    if (x1 >= ds.clip_x1) x1 = ds.clip_x1 - 1;
    if (x0 <= x1) memset(&fb[y][x0], ds.pal[c & 63], (size_t)dots(x1 - x0 + 1));
}

void draw_pset(int x, int y, int c) { put(x - ds.cam_x, y - ds.cam_y, (uint8_t)pen_or(c)); }

int draw_pget(int x, int y) {
    x -= ds.cam_x;
    y -= ds.cam_y;
    if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return 0;
    return fb[y][x];
}

void draw_line(int x0, int y0, int x1, int y1, int c) {
    uint8_t col = (uint8_t)pen_or(c);
    x0 -= ds.cam_x; x1 -= ds.cam_x; y0 -= ds.cam_y; y1 -= ds.cam_y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (int guard = 0; guard < 4096; guard++) {
        put(x0, y0, col);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_rect(int x, int y, int w, int h, int c, bool fill) {
    if (w <= 0 || h <= 0) return;
    uint8_t col = (uint8_t)pen_or(c);
    x -= ds.cam_x;
    y -= ds.cam_y;
    if (fill) {
        for (int j = 0; j < h; j++) hspan(x, x + w - 1, y + j, col);
        return;
    }
    hspan(x, x + w - 1, y, col);
    hspan(x, x + w - 1, y + h - 1, col);
    for (int j = 1; j < h - 1; j++) { put(x, y + j, col); put(x + w - 1, y + j, col); }
}

void draw_circle(int cx, int cy, int r, int c, bool fill) {
    if (r < 0) return;
    uint8_t col = (uint8_t)pen_or(c);
    cx -= ds.cam_x;
    cy -= ds.cam_y;
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        if (fill) {
            hspan(cx - x, cx + x, cy + y, col); hspan(cx - x, cx + x, cy - y, col);
            hspan(cx - y, cx + y, cy + x, col); hspan(cx - y, cx + y, cy - x, col);
        } else {
            put(cx + x, cy + y, col); put(cx - x, cy + y, col); put(cx + x, cy - y, col); put(cx - x, cy - y, col);
            put(cx + y, cy + x, col); put(cx - y, cy + x, col); put(cx + y, cy - x, col); put(cx - y, cy - x, col);
        }
        y++;
        if (err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x) + 1; }
    }
}

int draw_char(int ch, int x, int y, int c) {
    uint8_t col = (uint8_t)pen_or(c);
    if (ch < 0 || ch > 127) ch = '?';
    int sx = x - ds.cam_x, sy = y - ds.cam_y;
    for (int r = 0; r < FONT_H; r++) {
        uint8_t bits = font[ch][r];
        if (!bits) continue;
        for (int i = 0; i < 5; i++)
            if (bits & (0x10 >> i)) put(sx + i, sy + r, col);
    }
    return x + FONT_W;
}

int draw_text(const char *s, int x, int y, int c) {
    int x0 = x;
    for (; *s; s++) {
        if (*s == '\n') { x = x0; y += FONT_H; continue; }
        x = draw_char((unsigned char)*s, x, y, c);
    }
    return x;
}

int text_width(const char *s) {
    int w = 0, best = 0;
    for (; *s; s++) {
        if (*s == '\n') { w = 0; continue; }
        w += FONT_W;
        if (w > best) best = w;
    }
    return best;
}

/* one tile, screen space, flags: 1 = flip x, 2 = flip y */
static void blit_tile(int n, int sx, int sy, int flags) {
    if (sx >= ds.clip_x1 || sy >= ds.clip_y1 || sx + TILE <= ds.clip_x0 || sy + TILE <= ds.clip_y0) return;
    if (!cur_tiles || n < 0 || n >= NTILES) return;
    const uint8_t *t = cur_tiles->t[n];
    for (int j = 0; j < TILE; j++) {
        int ty = (flags & 2) ? TILE - 1 - j : j;
        for (int i = 0; i < TILE; i++) {
            int tx = (flags & 1) ? TILE - 1 - i : i;
            uint8_t c = t[ty * TILE + tx];
            if (c != CLEAR) put(sx + i, sy + j, c);
        }
    }
}

/* a w x h block of tiles from the sheet, starting at tile n */
void draw_sprite(int n, int x, int y, int w, int h, int flags) {
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    x -= ds.cam_x;
    y -= ds.cam_y;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            int ti = (flags & 1) ? w - 1 - i : i;
            int tj = (flags & 2) ? h - 1 - j : j;
            int tile = n + tj * SHEET_COLS + ti;
            if (tile < 0 || tile >= NTILES) continue;
            blit_tile(tile, x + i * TILE, y + j * TILE, flags);
        }
}

/* map cells (mx, my) .. (mx + w, my + h) of the current map drawn with their
 * top-left at (sx, sy). Empty cells are not drawn. */
void draw_map(int mx, int my, int sx, int sy, int w, int h) {
    const TileMap *m = cur_map;
    if (!m) return;
    sx -= ds.cam_x;
    sy -= ds.cam_y;
    for (int j = 0; j < h; j++) {
        int cy = my + j;
        if (cy < 0 || cy >= m->h) continue;
        int py = sy + j * TILE;
        if (py >= ds.clip_y1 || py + TILE <= ds.clip_y0) continue;
        for (int i = 0; i < w; i++) {
            int cx = mx + i;
            if (cx < 0 || cx >= m->w) continue;
            uint16_t t = m->cells[cy * m->w + cx];
            if (t != MAP_EMPTY) blit_tile(t, sx + i * TILE, py, 0);
        }
    }
}
