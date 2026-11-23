/* Reading PNGs, without a library: IMPORT file.png turns one into a tileset.
 *
 * Every color type (gray, RGB, indexed, gray + alpha, RGBA) at every bit
 * depth, with tRNS transparency. Not interlaced: Aseprite and most editors
 * don't interlace, and the message says how to save it plainly.
 * The pixel data is zlib (a 2-byte header, deflate, a checksum): zlib.c's
 * ge_inflate does the deflate part. */
#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

static uint8_t paeth(int a, int b, int c) {
    int p = a + b - c, pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
    return (uint8_t)(pa <= pb && pa <= pc ? a : pb <= pc ? b : c);
}

#define FAIL(...) do { snprintf(err, (size_t)errlen, __VA_ARGS__); goto out; } while (0)

uint8_t *png_read_rgba(const char *path, int *w_out, int *h_out, char *err, int errlen) {
    uint8_t *file = NULL, *z = NULL, *raw = NULL, *rgba = NULL;
    err[0] = 0;
    FILE *f = fopen(path, "rb");
    if (!f) { snprintf(err, (size_t)errlen, "can't open %s", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n > 0 && n < 64L << 20) file = malloc((size_t)n);
    size_t got = file ? fread(file, 1, (size_t)n, f) : 0;
    fclose(f);
    if (!file || got != (size_t)n) FAIL("can't read %s", path);
    if (n < 8 || memcmp(file, "\x89PNG\r\n\x1a\n", 8)) FAIL("%s is not a PNG", path);

    uint32_t w = 0, h = 0;
    int depth = 0, type = -1, interlace = 0;
    uint8_t plte[256][4];
    int nplte = 0;
    memset(plte, 255, sizeof plte);
    int trns_gray = -1, trns_r = -1, trns_g = -1, trns_b = -1;
    size_t zn = 0, zcap = 0;
    for (size_t p = 8; p + 12 <= (size_t)n;) {
        uint32_t len = be32(file + p);
        const uint8_t *t = file + p + 4, *d = file + p + 8;
        if (len > (size_t)n - p - 12) FAIL("%s is cut short", path);
        if (!memcmp(t, "IHDR", 4) && len >= 13) {
            w = be32(d); h = be32(d + 4); depth = d[8]; type = d[9]; interlace = d[12];
        } else if (!memcmp(t, "PLTE", 4)) {
            nplte = (int)(len / 3 > 256 ? 256 : len / 3);
            for (int i = 0; i < nplte; i++) { plte[i][0] = d[i * 3]; plte[i][1] = d[i * 3 + 1]; plte[i][2] = d[i * 3 + 2]; }
        } else if (!memcmp(t, "tRNS", 4)) {
            if (type == 3) for (uint32_t i = 0; i < len && i < 256; i++) plte[i][3] = d[i];
            else if (type == 0 && len >= 2) trns_gray = d[0] << 8 | d[1];
            else if (type == 2 && len >= 6) { trns_r = d[0] << 8 | d[1]; trns_g = d[2] << 8 | d[3]; trns_b = d[4] << 8 | d[5]; }
        } else if (!memcmp(t, "IDAT", 4)) {
            if (zn + len > zcap) {
                zcap = (zn + len) * 2;
                uint8_t *nz = realloc(z, zcap);
                if (!nz) FAIL("out of memory");
                z = nz;
            }
            memcpy(z + zn, d, len);
            zn += len;
        } else if (!memcmp(t, "IEND", 4)) break;
        p += 12 + len;
    }
    if (type < 0) FAIL("%s has no image header", path);
    if (w == 0 || h == 0 || w > 4096 || h > 4096) FAIL("%s is %ux%u: too big to import", path, w, h);
    if (interlace) FAIL("%s is interlaced: save it again without interlacing", path);
    int chans = type == 0 ? 1 : type == 2 ? 3 : type == 3 ? 1 : type == 4 ? 2 : type == 6 ? 4 : 0;
    bool depth_ok = depth == 8 || depth == 16 || ((type == 0 || type == 3) && (depth == 1 || depth == 2 || depth == 4));
    if (!chans || !depth_ok || (type == 3 && depth == 16)) FAIL("%s: unknown PNG kind (color type %d, %d bits)", path, type, depth);
    if (type == 3 && !nplte) FAIL("%s is indexed but has no palette", path);
    if (zn < 6) FAIL("%s has no pixels", path);

    size_t bpp = (size_t)(chans * depth + 7) / 8;           /* bytes per pixel, at least 1: the filters step by this */
    size_t stride = ((size_t)w * (size_t)chans * (size_t)depth + 7) / 8;
    size_t want = (stride + 1) * h, rawn = 0;
    const char *zerr = NULL;
    raw = ge_inflate(z + 2, zn - 2, want, &rawn, NULL, &zerr);   /* past the zlib header */
    if (!raw) FAIL("%s: %s", path, zerr ? zerr : "the pixel data is damaged");
    if (rawn < want) FAIL("%s is cut short", path);

    for (uint32_t y = 0; y < h; y++) {                       /* undo the per-row filters, in place */
        uint8_t *row = raw + y * (stride + 1), *cur = row + 1, *prev = y ? row - stride : NULL;
        int ft = row[0];
        for (size_t i = 0; i < stride; i++) {
            int a = i >= bpp ? cur[i - bpp] : 0, b = prev ? prev[i] : 0, c = prev && i >= bpp ? prev[i - bpp] : 0;
            switch (ft) {
                case 0: break;
                case 1: cur[i] = (uint8_t)(cur[i] + a); break;
                case 2: cur[i] = (uint8_t)(cur[i] + b); break;
                case 3: cur[i] = (uint8_t)(cur[i] + (a + b) / 2); break;
                case 4: cur[i] = (uint8_t)(cur[i] + paeth(a, b, c)); break;
                default: FAIL("%s: the pixel data is damaged", path);
            }
        }
    }

    rgba = malloc((size_t)w * h * 4);
    if (!rgba) FAIL("out of memory");
    int maxv = (1 << depth) - 1;
    for (uint32_t y = 0; y < h; y++) {
        const uint8_t *s = raw + y * (stride + 1) + 1;
        for (uint32_t x = 0; x < w; x++) {
            uint8_t *o = rgba + ((size_t)y * w + x) * 4;
            int v[4];                                        /* the samples, at the file's depth */
            for (int c = 0; c < chans; c++) {
                if (depth == 16) { size_t i = ((size_t)x * chans + c) * 2; v[c] = s[i] << 8 | s[i + 1]; }
                else if (depth == 8) v[c] = s[(size_t)x * chans + c];
                else { size_t bit = (size_t)x * depth; v[c] = s[bit / 8] >> (8 - depth - bit % 8) & maxv; }
            }
#define TO8(q) ((uint8_t)(depth == 16 ? (q) >> 8 : (q) * 255 / maxv))
            if (type == 3) {
                int i = v[0] < 256 ? v[0] : 0;
                memcpy(o, plte[i], 4);
            } else if (type == 0 || type == 4) {
                o[0] = o[1] = o[2] = TO8(v[0]);
                o[3] = type == 4 ? TO8(v[1]) : v[0] == trns_gray ? 0 : 255;
            } else {
                o[0] = TO8(v[0]); o[1] = TO8(v[1]); o[2] = TO8(v[2]);
                o[3] = type == 6 ? TO8(v[3]) : v[0] == trns_r && v[1] == trns_g && v[2] == trns_b ? 0 : 255;
            }
#undef TO8
        }
    }
    *w_out = (int)w;
    *h_out = (int)h;
out:
    free(file);
    free(z);
    free(raw);
    if (err[0] && rgba) { free(rgba); rgba = NULL; }
    return rgba;
}

/* the palette color nearest an RGB value (each channel's levels are 0, 85, 170, 255) */
static int nearest(const uint8_t *p, bool *exact) {
    int c = 0;
    for (int k = 0; k < 3; k++) c = c * 4 + (p[k] + 42) / 85;
    *exact = palette[c] == ((uint32_t)p[0] << 16 | (uint32_t)p[1] << 8 | p[2]);
    return c;
}

bool png_to_tileset(const char *path, Tileset *ts, char *msg, int msglen) {
    int w, h;
    msg[0] = 0;
    uint8_t *px = png_read_rgba(path, &w, &h, msg, msglen);
    if (!px) return false;
    memset(ts, CLEAR, sizeof *ts);
    bool used[NCOLORS] = {false};
    int ncolors = 0, inexact = 0;
    for (int y = 0; y < h && y < SHEET_H; y++)
        for (int x = 0; x < w && x < SHEET_W; x++) {
            const uint8_t *p = px + ((size_t)y * w + x) * 4;
            if (p[3] < 128) continue;                        /* mostly see-through: transparent */
            bool exact;
            int c = nearest(p, &exact);
            if (!exact) inexact++;
            if (!used[c]) { used[c] = true; ncolors++; }
            ts->t[(y / TILE) * SHEET_COLS + x / TILE][(y % TILE) * TILE + x % TILE] = (uint8_t)c;
        }
    free(px);
    int len = snprintf(msg, (size_t)msglen, "%d color%s", ncolors, ncolors == 1 ? "" : "s");
    if (len < msglen && inexact)
        len += snprintf(msg + len, (size_t)(msglen - len), ", %d pixel%s not in the palette (matched to the nearest)", inexact, inexact == 1 ? "" : "s");
    if (len < msglen && (w > SHEET_W || h > SHEET_H))
        snprintf(msg + len, (size_t)(msglen - len), ", cut to %dx%d from %dx%d", SHEET_W, SHEET_H, w, h);
    return true;
}
