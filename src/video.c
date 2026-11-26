/* Framebuffer drawing. Everything draws palette indices into fb; the
 * platform layer turns them into real colors once per frame.
 * All drawing goes through the camera offset and the clip rectangle.
 *
 * The dot limit: drawing is done in software, so a cart may only plot
 * dot_limit pixels per frame (flip() starts a new frame). Pixels clipped away
 * don't count, nor do a tile's transparent ones, nor cls() and rect_fill_fast().
 * Past the limit, pixels are
 * dropped until the next flip(), so a cart is rewarded for redrawing only
 * what changed.
 *
 * Depth and blending: every pixel goes through plot(), which skips it if the
 * depth buffer holds something nearer (lower) than the depth it is drawn at,
 * then blends it with what is on screen and records its depth. cls() puts
 * the depth of what it clears at DEPTH_FAR, so anything can be drawn on it. */
#include "engine.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

uint8_t fb[SCREEN_H][SCREEN_W];
int16_t zbuf[SCREEN_H][SCREEN_W];
DrawState ds;
int dot_limit = DOT_LIMIT_DEFAULT, dots_used;
int view_x = 0, view_w = SCREEN_W;

/* 6-bit color: index = r*16 + g*4 + b, each channel 0..3 -> 0, 85, 170, 255 */
#define L(v) ((v) * 85)
#define P(i) ((uint32_t)L(((i) >> 4) & 3) << 16 | (uint32_t)L(((i) >> 2) & 3) << 8 | (uint32_t)L((i) & 3))
#define P4(i) P(i), P(i + 1), P(i + 2), P(i + 3)
#define P16(i) P4(i), P4(i + 4), P4(i + 8), P4(i + 12)
const uint32_t palette[NCOLORS] = {P16(0), P16(16), P16(32), P16(48)};

/* row y of the depth buffer, x0 .. x1 - 1, back to the far plane */
static void zfill(int y, int x0, int x1) {
    for (int x = x0; x < x1; x++) zbuf[y][x] = DEPTH_FAR;
}

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
    ds.blend = BLEND_NORMAL;
    ds.depth = 0;
    for (int y = 0; y < SCREEN_H; y++) zfill(y, 0, SCREEN_W);
}

void draw_set_depth(int z) {
    ds.depth = (int16_t)(z < INT16_MIN ? INT16_MIN : z > INT16_MAX ? INT16_MAX : z);
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

/* the fast fill behind cls() and rect_fill_fast(): screen-space x0..x1-1,
 * y0..y1-1, cut to the clip rectangle, set to color col (through the palette
 * map) at depth DEPTH_FAR. Whole rows are written at once, with no depth test
 * or blending, so none of it counts against the dot limit. */
static void fast_fill(int x0, int y0, int x1, int y1, uint8_t col) {
    if (x0 < ds.clip_x0) x0 = ds.clip_x0;
    if (y0 < ds.clip_y0) y0 = ds.clip_y0;
    if (x1 > ds.clip_x1) x1 = ds.clip_x1;
    if (y1 > ds.clip_y1) y1 = ds.clip_y1;
    for (int y = y0; y < y1 && x0 < x1; y++) {
        memset(&fb[y][x0], ds.pal[col], (size_t)(x1 - x0));
        zfill(y, x0, x1);
    }
}

/* clears the clip rectangle (the whole screen unless clip() was used), and its
 * depth to DEPTH_FAR. Neither the blend mode nor the depth applies to it, and
 * the drawing depth goes back to 0. Free: see fast_fill(). */
void draw_cls(int c) {
    fast_fill(ds.clip_x0, ds.clip_y0, ds.clip_x1, ds.clip_y1, (uint8_t)(c < 0 ? 0 : (c & 63)));
    ds.depth = 0;
}

/* a filled rectangle the way cls() fills: through the camera and the clip
 * rectangle like rect_fill, but its depth goes to DEPTH_FAR, the blend mode
 * and depth test don't apply, and it is free. The drawing depth is left alone. */
void draw_rect_fill_fast(int x, int y, int w, int h, int c) {
    if (w <= 0 || h <= 0) return;
    x -= ds.cam_x;
    y -= ds.cam_y;
    fast_fill(x, y, x + w, y + h, (uint8_t)pen_or(c));
}

/* src drawn over dst, per 2-bit channel */
static inline uint8_t blend(uint8_t s, uint8_t d) {
    if (ds.blend == BLEND_NORMAL) return s;
    if (ds.blend == BLEND_XOR) return s ^ d;      /* the channels are bit fields */
    uint8_t out = 0;
    for (int sh = 0; sh <= 4; sh += 2) {
        int a = (s >> sh) & 3, b = (d >> sh) & 3, r;
        switch (ds.blend) {
        case BLEND_ADD: r = a + b > 3 ? 3 : a + b; break;
        case BLEND_SUB: r = b - a < 0 ? 0 : b - a; break;
        case BLEND_MAX: r = a > b ? a : b; break;
        default:        r = a < b ? a : b; break;    /* BLEND_MIN */
        }
        out |= (uint8_t)(r << sh);
    }
    return out;
}

/* one screen-space pixel of final color c at depth z, already clipped */
static inline void plot(int x, int y, uint8_t c, int16_t z) {
    if (z > zbuf[y][x]) return;       /* something nearer is there */
    if (!dots(1)) return;
    fb[y][x] = blend(c, fb[y][x]);
    zbuf[y][x] = z;
}

static inline bool in_clip(int x, int y) {
    return x >= ds.clip_x0 && x < ds.clip_x1 && y >= ds.clip_y0 && y < ds.clip_y1;
}

/* screen-space pixel, clipped */
static inline void put(int x, int y, uint8_t c) {
    if (in_clip(x, y)) plot(x, y, ds.pal[c & 63], ds.depth);
}

static void hspan(int x0, int x1, int y, uint8_t c) {
    if (y < ds.clip_y0 || y >= ds.clip_y1) return;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (x0 < ds.clip_x0) x0 = ds.clip_x0;
    if (x1 >= ds.clip_x1) x1 = ds.clip_x1 - 1;
    if (x0 > x1) return;
    uint8_t col = ds.pal[c & 63];
    for (int x = x0; x <= x1; x++) plot(x, y, col, ds.depth);
}

void draw_plot(int x, int y, int c) { put(x - ds.cam_x, y - ds.cam_y, (uint8_t)pen_or(c)); }

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

/* the editors' "transparent" backdrop: 5 x 5 pink checks anchored to screen
 * (0, 0), so neighbouring cells and swatches line up seamlessly */
#define CHECK_SIZE 5
void draw_checker(int x, int y, int w, int h) {
    for (int j = 0; j < h; j++) {
        int sy = y + j, ry = ((sy % CHECK_SIZE) + CHECK_SIZE) % CHECK_SIZE;
        int by = (sy - ry) / CHECK_SIZE;                 /* floor, even off-screen */
        for (int i = 0; i < w; i++) {
            int sx = x + i, rx = ((sx % CHECK_SIZE) + CHECK_SIZE) % CHECK_SIZE;
            int bx = (sx - rx) / CHECK_SIZE;
            draw_plot(sx, sy, (bx + by) & 1 ? 50 : 34);
        }
    }
}

void draw_circle(int cx, int cy, int r, int c, bool fill) {
    if (r < 0) return;
    uint8_t col = (uint8_t)pen_or(c);
    cx -= ds.cam_x;
    cy -= ds.cam_y;
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        if (fill) {
            /* each row exactly once, at its widest, or XOR / ADD / SUB would
             * blend the rows drawn twice twice. Rows cy +- y are new every
             * step (cy itself only once); rows cy +- x are drawn on x's last
             * step, when y is widest, unless they are a cy +- y row already */
            hspan(cx - x, cx + x, cy + y, col);
            if (y) hspan(cx - x, cx + x, cy - y, col);
            if ((err >= 0 || y + 1 > x) && x > y) {
                hspan(cx - y, cx + y, cy + x, col); hspan(cx - y, cx + y, cy - x, col);
            }
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
    if (ch < FONT_FIRST || ch >= FONT_FIRST + NGLYPHS) ch = '?';
    const Font *f = ds.font ? ds.font : &sys_font;
    int sx = x - ds.cam_x, sy = y - ds.cam_y;
    for (int r = 0; r < f->h; r++) {
        uint32_t bits = f->g[ch - FONT_FIRST][r];
        for (int i = 0; bits && i < f->w; i++, bits >>= 1)
            if (bits & 1) put(sx + i, sy + r, col);
    }
    return x + f->w;
}

int draw_text(const char *s, int x, int y, int c) {
    int x0 = x;
    for (; *s; s++) {
        if (*s == '\n') { x = x0; y += (ds.font ? ds.font : &sys_font)->h; continue; }
        x = draw_char((unsigned char)*s, x, y, c);
    }
    return x;
}

int text_width(const char *s) {
    int w = 0, best = 0;
    for (; *s; s++) {
        if (*s == '\n') { w = 0; continue; }
        w += (ds.font ? ds.font : &sys_font)->w;
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

/* a tw x th block of tiles from the sheet, starting at tile n, stretched to
 * dw x dh screen pixels (nearest pixel). Each screen pixel is mapped back to
 * one source pixel, so any scale works, up or down, and flips are free. */
void draw_sprite_ex(int n, int x, int y, int tw, int th, int dw, int dh, int flags) {
    if (!cur_tiles || dw <= 0 || dh <= 0) return;
    if (tw < 1) tw = 1;
    if (th < 1) th = 1;
    x -= ds.cam_x;
    y -= ds.cam_y;
    int sw = tw * TILE, sh = th * TILE;          /* source size in pixels */
    int i0 = ds.clip_x0 - x, i1 = ds.clip_x1 - x; /* the columns / rows the clip leaves */
    int j0 = ds.clip_y0 - y, j1 = ds.clip_y1 - y;
    if (i0 < 0) i0 = 0;
    if (j0 < 0) j0 = 0;
    if (i1 > dw) i1 = dw;
    if (j1 > dh) j1 = dh;
    for (int j = j0; j < j1; j++) {
        int v = (int)((long long)j * sh / dh);
        if (flags & 2) v = sh - 1 - v;
        int row = n + (v / TILE) * SHEET_COLS, py = (v % TILE) * TILE;
        for (int i = i0; i < i1; i++) {
            int u = (int)((long long)i * sw / dw);
            if (flags & 1) u = sw - 1 - u;
            int tile = row + u / TILE;
            if (tile < 0 || tile >= NTILES) continue;
            uint8_t c = cur_tiles->t[tile][py + u % TILE];
            if (c != CLEAR) put(x + i, y + j, c);
        }
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

/* n dots of color c at random places in the rectangle (x, y, w, h).
 * unit_rng gives [0, 1): the cart's rnd() stream, so seed_rnd() repeats a spray */
void draw_spray(int x, int y, int w, int h, int n, int c, double (*unit_rng)(void)) {
    if (w <= 0 || h <= 0 || n <= 0) return;
    uint8_t col = (uint8_t)pen_or(c);
    x -= ds.cam_x;
    y -= ds.cam_y;
    if (x >= ds.clip_x1 || y >= ds.clip_y1 || x + w <= ds.clip_x0 || y + h <= ds.clip_y0) return;
    for (int i = 0; i < n; i++) {
        int px = x + (int)(unit_rng() * w), py = y + (int)(unit_rng() * h);
        put(px, py, col);
    }
}

/* ---- textured quads ----
 * Each pixel inside the quad's bounding box is mapped back to (u, v) in the
 * unit square by inverting the bilinear patch
 *     P(u, v) = p0 + (p1 - p0) u + (p3 - p0) v + (p0 - p1 + p2 - p3) u v
 * which is exact for any quad, parallel edges or not: no seam down a diagonal
 * as with two affine triangles. It is a quadratic in v; a quad that folds
 * over itself has two roots and the nearer (or, in 2D, the first) one wins. */

static inline double cross2(double ax, double ay, double bx, double by) { return ax * by - ay * bx; }

/* up to two (u, v) in [0, 1) for point h (relative to p0); returns how many */
static int inv_bilinear(double hx, double hy, double ex, double ey, double fx, double fy,
                        double gx, double gy, double u[2], double v[2]) {
    double k2 = cross2(gx, gy, fx, fy);
    double k1 = cross2(ex, ey, fx, fy) + cross2(hx, hy, gx, gy);
    double k0 = cross2(hx, hy, ex, ey);
    double vs[2];
    int nv = 0;
    if (fabs(k2) < 1e-9) {                      /* opposite edges parallel: linear */
        if (fabs(k1) < 1e-12) return 0;         /* the quad has no area */
        vs[nv++] = -k0 / k1;
    } else {
        double w = k1 * k1 - 4.0 * k0 * k2;
        if (w < 0) return 0;
        w = sqrt(w);
        vs[nv++] = (-k1 - w) / (2.0 * k2);
        vs[nv++] = (-k1 + w) / (2.0 * k2);
    }
    int got = 0;
    for (int i = 0; i < nv; i++) {
        double vv = vs[i];
        if (vv < 0 || vv >= 1) continue;
        /* u from whichever axis is better conditioned */
        double dx = ex + gx * vv, dy = ey + gy * vv, uu;
        if (fabs(dx) >= fabs(dy)) { if (fabs(dx) < 1e-12) continue; uu = (hx - fx * vv) / dx; }
        else uu = (hy - fy * vv) / dy;
        if (uu < 0 || uu >= 1) continue;
        u[got] = uu;
        v[got] = vv;
        got++;
    }
    return got;
}

/* texel (tx, ty) of the w x h tile block at tile n; CLEAR off the sheet */
static inline uint8_t block_texel(int n, int tx, int ty) {
    int tile = n + (ty / TILE) * SHEET_COLS + tx / TILE;
    if (tile < 0 || tile >= NTILES) return CLEAR;
    return cur_tiles->t[tile][(ty % TILE) * TILE + tx % TILE];
}

void draw_tex_quad(const int px[4], const int py[4], const int *z, int n, int w, int h) {
    if (!cur_tiles || w < 1 || h < 1) return;
    int tw = w * TILE, th = h * TILE;
    double ax = px[0] - ds.cam_x, ay = py[0] - ds.cam_y;
    double ex = px[1] - px[0], ey = py[1] - py[0];                         /* p1 - p0 */
    double fx = px[3] - px[0], fy = py[3] - py[0];                         /* p3 - p0 */
    double gx = px[0] - px[1] + px[2] - px[3], gy = py[0] - py[1] + py[2] - py[3];
    int bx0 = px[0], bx1 = px[0], by0 = py[0], by1 = py[0];
    for (int i = 1; i < 4; i++) {
        if (px[i] < bx0) bx0 = px[i];
        if (px[i] > bx1) bx1 = px[i];
        if (py[i] < by0) by0 = py[i];
        if (py[i] > by1) by1 = py[i];
    }
    bx0 -= ds.cam_x; bx1 -= ds.cam_x; by0 -= ds.cam_y; by1 -= ds.cam_y;
    if (bx0 < ds.clip_x0) bx0 = ds.clip_x0;
    if (by0 < ds.clip_y0) by0 = ds.clip_y0;
    if (bx1 > ds.clip_x1 - 1) bx1 = ds.clip_x1 - 1;
    if (by1 > ds.clip_y1 - 1) by1 = ds.clip_y1 - 1;
    if (bx0 > bx1 || by0 > by1) return;
    /* depth, bilinear like the position: z0 + dz1 u + dz3 v + dz2 u v */
    double z0 = 0, zu = 0, zv = 0, zuv = 0;
    if (z) {
        z0 = z[0]; zu = z[1] - z[0]; zv = z[3] - z[0]; zuv = z[0] - z[1] + z[2] - z[3];
    }
    for (int y = by0; y <= by1; y++) {
        double hy = y + 0.5 - ay;
        for (int x = bx0; x <= bx1; x++) {
            double u[2], v[2];
            int k = inv_bilinear(x + 0.5 - ax, hy, ex, ey, fx, fy, gx, gy, u, v);
            if (!k) continue;
            int pick = 0;
            double depth = 0;
            if (z) {
                depth = z0 + zu * u[0] + zv * v[0] + zuv * u[0] * v[0];
                if (k == 2) {
                    double d2 = z0 + zu * u[1] + zv * v[1] + zuv * u[1] * v[1];
                    if (d2 < depth) { depth = d2; pick = 1; }
                }
            }
            int tx = (int)(u[pick] * tw), ty = (int)(v[pick] * th);
            if (tx >= tw) tx = tw - 1;
            if (ty >= th) ty = th - 1;
            uint8_t c = block_texel(n, tx, ty);
            if (c == CLEAR) continue;
            int16_t dz = ds.depth;
            if (z) {
                double r = floor(depth + 0.5);
                dz = (int16_t)(r < INT16_MIN ? INT16_MIN : r > INT16_MAX ? INT16_MAX : r);
            }
            plot(x, y, ds.pal[c], dz);
        }
    }
}
