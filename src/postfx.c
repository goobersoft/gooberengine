/* Post-processing: what happens to a finished frame on its way to the window.
 * Nothing here changes fb, so screenshots, carts and tests see the real
 * picture; only the window shows the effects (settings.fx_*).
 *
 * The frame goes through two stages:
 *   1. color, at the console's own size: each of the 64 palette colors is
 *      tinted (a hue rotation) and desaturated once, into a lookup table; then
 *      screen burn lets each lit pixel fade out instead of going dark at once.
 *   2. shape, at s screen pixels per console pixel: the CRT bend and the
 *      scanlines. Both depend only on s and the settings, so every screen
 *      pixel's source pixel and shade are worked out once into a map, and a
 *      frame is just one lookup per screen pixel.
 * The renderer then scales the result to the window (by a whole number, when
 * s is a divisor of the window's scale). */
#include "engine.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------- color */

static uint32_t lut[NCOLORS];
static int lut_tint = -1, lut_sat = -1;

static int clamp255(double v) { return v < 0 ? 0 : v > 255 ? 255 : (int)(v + 0.5); }

static void build_lut(void) {
    if (lut_tint == settings.fx_tint && lut_sat == settings.fx_saturation) return;
    lut_tint = settings.fx_tint;
    lut_sat = settings.fx_saturation;
    /* a hue rotation that keeps brightness (the matrix of SVG's feColorMatrix hueRotate) */
    double a = lut_tint * M_PI / 180.0, c = cos(a), s = sin(a);
    double m[3][3] = {
        {0.213 + c * 0.787 - s * 0.213, 0.715 - c * 0.715 - s * 0.715, 0.072 - c * 0.072 + s * 0.928},
        {0.213 - c * 0.213 + s * 0.143, 0.715 + c * 0.285 + s * 0.140, 0.072 - c * 0.072 - s * 0.283},
        {0.213 - c * 0.213 - s * 0.787, 0.715 - c * 0.715 + s * 0.715, 0.072 + c * 0.928 + s * 0.072},
    };
    double sat = lut_sat / 100.0;
    for (int i = 0; i < NCOLORS; i++) {
        double in[3] = {(palette[i] >> 16) & 255, (palette[i] >> 8) & 255, palette[i] & 255}, out[3];
        for (int k = 0; k < 3; k++) out[k] = m[k][0] * in[0] + m[k][1] * in[1] + m[k][2] * in[2];
        double luma = 0.2126 * out[0] + 0.7152 * out[1] + 0.0722 * out[2];
        for (int k = 0; k < 3; k++) out[k] = luma + (out[k] - luma) * sat;
        lut[i] = (uint32_t)clamp255(out[0]) << 16 | (uint32_t)clamp255(out[1]) << 8 | (uint32_t)clamp255(out[2]);
    }
}

/* the frame in color, after tint, saturation and burn */
static uint32_t rgb[SCREEN_H * SCREEN_W];
static bool burn_valid;

void postfx_reset(void) { burn_valid = false; }

static void color_stage(void) {
    build_lut();
    int burn = settings.fx_burn;
    if (burn <= 0 || !burn_valid) {
        for (int y = 0; y < SCREEN_H; y++)
            for (int x = 0; x < SCREEN_W; x++) rgb[y * SCREEN_W + x] = lut[fb[y][x] & 63];
        burn_valid = burn > 0;
        return;
    }
    /* phosphor: a channel lights up at once but fades by `keep`/256 each frame,
     * so a pixel that went dark leaves a fading trail */
    uint32_t keep = (uint32_t)(burn * 256 / 100);
    for (int y = 0; y < SCREEN_H; y++)
        for (int x = 0; x < SCREEN_W; x++) {
            uint32_t now = lut[fb[y][x] & 63], *p = &rgb[y * SCREEN_W + x], was = *p, out = 0;
            for (int sh = 0; sh <= 16; sh += 8) {
                uint32_t n = (now >> sh) & 255, w = (((was >> sh) & 255) * keep) >> 8;
                out |= (n > w ? n : w) << sh;
            }
            *p = out;
        }
}

/* ---------------------------------------------------------------- shape */

/* the CRT bend: a point on the screen (-1..1 each way) to the point of the
 * picture shown there. The middle of each edge stays put and the corners pull
 * in, so the picture bulges like the glass of a tube. */
static void bend(double u, double v, double *su, double *sv) {
    double k = settings.fx_crt / 100.0 * 0.22;
    *su = u * (1 + k * v * v);
    *sv = v * (1 + k * u * u);
}

static int32_t *map;            /* per screen pixel: the picture pixel it shows, or -1 (outside, black) */
static uint8_t *shade;          /* per screen pixel: 255 lit .. 0 black (the scanlines) */
static int map_s, map_crt = -1, map_scan = -1;

static bool build_map(int s) {
    if (s == map_s && settings.fx_crt == map_crt && settings.fx_scanlines == map_scan) return true;
    int w = SCREEN_W * s, h = SCREEN_H * s;
    int32_t *m = realloc(map, sizeof *m * (size_t)w * h);
    if (!m) return false;
    map = m;
    uint8_t *sh = realloc(shade, (size_t)w * h);
    if (!sh) return false;
    shade = sh;
    map_s = s;
    map_crt = settings.fx_crt;
    map_scan = settings.fx_scanlines;
    /* scanlines darken the last screen row of each console row; at 1x there is no such row */
    uint8_t gap = (uint8_t)(255 - (s >= 2 ? map_scan * 255 / 100 : 0));
    double edge = (s - 1) / (double)s;
    for (int oy = 0; oy < h; oy++) {
        double v = (oy + 0.5) / h * 2 - 1;
        for (int ox = 0; ox < w; ox++) {
            size_t i = (size_t)oy * w + ox;
            double u = (ox + 0.5) / w * 2 - 1, su = u, sv = v;
            if (map_crt) bend(u, v, &su, &sv);
            if (su < -1 || su >= 1 || sv < -1 || sv >= 1) { map[i] = -1; shade[i] = 0; continue; }
            double sx = (su + 1) / 2 * SCREEN_W, sy = (sv + 1) / 2 * SCREEN_H;
            int ix = (int)sx, iy = (int)sy;
            map[i] = iy * SCREEN_W + ix;
            shade[i] = sy - iy >= edge ? gap : 255;
        }
    }
    return true;
}

int postfx_scale(int k) {
    bool shape = settings.fx_crt > 0 || (settings.fx_scanlines > 0 && k >= 2);
    bool color = settings.fx_tint != 0 || settings.fx_saturation != 100 || settings.fx_burn > 0;
    if (!color) burn_valid = false;
    if (!shape) return color ? 1 : 0;
    if (k <= 4) return k < 1 ? 1 : k;
    /* big windows: a divisor of k keeps the renderer's last step a whole number
     * and saves work (8x is drawn at 4x, then doubled); a prime is drawn as it is */
    for (int d = 4; d >= 2; d--) if (k % d == 0) return d;
    return k;
}

void postfx_render(uint32_t *out, int pitch, int s) {
    color_stage();
    if (s == 1 && !settings.fx_crt) {           /* color only: no map needed */
        for (int y = 0; y < SCREEN_H; y++) {
            uint32_t *row = (uint32_t *)((uint8_t *)out + (size_t)y * pitch);
            for (int x = 0; x < SCREEN_W; x++) row[x] = 0xFF000000u | rgb[y * SCREEN_W + x];
        }
        return;
    }
    if (!build_map(s)) return;
    int w = SCREEN_W * s, h = SCREEN_H * s;
    for (int oy = 0; oy < h; oy++) {
        uint32_t *row = (uint32_t *)((uint8_t *)out + (size_t)oy * pitch);
        const int32_t *m = map + (size_t)oy * w;
        const uint8_t *sh = shade + (size_t)oy * w;
        for (int ox = 0; ox < w; ox++) {
            uint32_t c = m[ox] < 0 ? 0 : rgb[m[ox]], f = sh[ox];
            if (f != 255)
                c = (((c >> 16 & 255) * f / 255) << 16) | (((c >> 8 & 255) * f / 255) << 8) | ((c & 255) * f / 255);
            row[ox] = 0xFF000000u | c;
        }
    }
}

void postfx_map_mouse(int *x, int *y) {
    if (!settings.fx_crt) return;
    double su, sv;
    bend((*x + 0.5) / SCREEN_W * 2 - 1, (*y + 0.5) / SCREEN_H * 2 - 1, &su, &sv);
    double sx = floor((su + 1) / 2 * SCREEN_W), sy = floor((sv + 1) / 2 * SCREEN_H);
    *x = sx < -1 ? -1 : sx > SCREEN_W ? SCREEN_W : (int)sx;
    *y = sy < -1 ? -1 : sy > SCREEN_H ? SCREEN_H : (int)sy;
}
