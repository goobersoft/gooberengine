/* Resource files, the documents the editors work on, and the slots a cart
 * loads them into while it runs.
 *
 * Files (in the cart folder):
 *   tiles/NAME.g64   tileset    maps/NAME.g64   map
 *   sfx/NAME.g64     sound bank music/NAME.g64  song
 *   fonts/NAME.g64   font
 *
 * The .g64 files are base-64 text. Each character of the alphabet
 *   0-9 a-z A-Z + *
 * stands for a number 0-63 (in that order: the same characters stand for the
 * same colors as in the old tiles.txt). Whitespace is ignored, so a file can
 * be broken into lines; GooberEngine writes one row per line.
 *
 *   tileset   X Y then X*10 x Y*10 pixel characters, row by row
 *             X, Y: width and height in pixels as (digit + 1) * 10, so 10-640;
 *             a tileset holds at most 400 x 240 (960 tiles, 40 per row).
 *             A pixel is a color 0-63, or '.' for transparent.
 *   map       X Y then two characters per cell, row by row
 *             X, Y: width and height in screens as digit + 1, at most 10
 *             (a screen is 40 x 24 cells). A cell is a tile number hi*64 + lo,
 *             or ".." for no tile. Tile 0 is a tile like any other.
 *   sound     W S: the number of waves and of sound effects, each as digit + 1.
 *             Then W waves of 32 samples 0-f (7-8 is the middle), then S sound
 *             effects: speed - 1, length - 1, loop start, loop end, then 32
 *             steps of pitch (0-63, 24 = C-4), wave (0-f, g = noise),
 *             volume (0-7), effect (0-6: none slide vibrato drop fade-in
 *             fade-out arpeggio). Waves and effects left out are the defaults.
 *   music     S O P T: speed - 1, order entries - 1, patterns - 1, tracks - 1.
 *             Then O order entries of two characters: 0N plays pattern N,
 *             1N jumps to entry N, .. stops. Then P patterns of 32 rows, each
 *             row T cells of four characters: pitch wave volume effect (as in
 *             a sound effect), .... for nothing new, ==== for note off.
 *   font      W H: a character's size, each as digit + 1 (4 to 20; the built-in
 *             font is 5 x 10, "49"). Then 96 glyphs, for characters 32-127, one
 *             per line: H rows top to bottom, each row (W + 5) / 6 characters of
 *             six pixels, the character's top bit (32) the leftmost pixel. A
 *             glyph fills its cell: the space between characters is its own.
 *
 * Old carts kept one of each at the top level (tiles.txt, map.txt, sfx.txt,
 * music.txt), and sound and music were text in sfx/NAME.txt, music/NAME.txt.
 * Those are read as before and written as .g64 by the next SAVE, which renames
 * the old file to *.old. */
#include "engine.h"
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char *const res_dir[NRES] = {"tiles", "maps", "sfx", "music", "fonts"};
const char *const res_ext[NRES] = {".g64", ".g64", ".g64", ".g64", ".g64"};
const char *const res_default[NRES] = {"tileset", "map", "sfx", "music", "font"};
const char *const res_word[NRES] = {"tileset", "map", "sound", "music", "font"};
static const char *const legacy_file[NRES] = {"tiles.txt", "map.txt", "sfx.txt", "music.txt", NULL};   /* fonts are new */

Tileset *cur_tiles;
TileMap *cur_map;
SoundBank *cur_snd;
Song *cur_song;
Font *cur_font;

/* ---------------------------------------------------------------- base 64 */

static int b64(int c) {
    const char *p = c && c != '.' ? strchr(ge_pixel_chars, c) : NULL;
    return p ? (int)(p - ge_pixel_chars) : -1;
}

/* the file without its whitespace, malloc'd */
static char *read_packed(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    size_t cap = 4096, n = 0;
    char *s = malloc(cap);
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (isspace(c)) continue;
        if (n + 1 >= cap) s = realloc(s, cap *= 2);
        s[n++] = (char)c;
    }
    s[n] = 0;
    fclose(f);
    *len = n;
    return s;
}

static const char *file_part(const char *path) {
    const char *s = strrchr(path, '/');
    return s ? s + 1 : path;
}

static bool parse_tileset(const char *path, Tileset *ts, char *err, int errlen) {
    size_t n;
    char *s = read_packed(path, &n);
    if (!s) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = file_part(path);
    bool ok = false;
    int dw = n >= 2 ? b64(s[0]) : -1, dh = n >= 2 ? b64(s[1]) : -1;
    int w = (dw + 1) * TILE, h = (dh + 1) * TILE;
    if (dw < 0 || dh < 0) snprintf(err, (size_t)errlen, "%s: starts with its width and height (two characters 0-9 a-z A-Z + *)", fn);
    else if (w > SHEET_W || h > SHEET_H) snprintf(err, (size_t)errlen, "%s: is %d x %d pixels; a tileset is at most %d x %d", fn, w, h, SHEET_W, SHEET_H);
    else if (n - 2 != (size_t)w * (size_t)h) snprintf(err, (size_t)errlen, "%s: a %d x %d tileset has %d pixels, this one has %zu", fn, w, h, w * h, n - 2);
    else {
        ok = true;
        for (int y = 0; y < h && ok; y++)
            for (int x = 0; x < w; x++) {
                char c = s[2 + y * w + x];
                int v = c == '.' ? CLEAR : b64(c);
                if (v < 0) {
                    snprintf(err, (size_t)errlen, "%s: '%c' at x %d y %d is not a color (0-9 a-z A-Z + *, or . for clear)", fn, c, x, y);
                    ok = false;
                    break;
                }
                ts->t[(y / TILE) * SHEET_COLS + x / TILE][(y % TILE) * TILE + x % TILE] = (uint8_t)v;
            }
    }
    free(s);
    return ok;
}

static bool tile_empty(const Tileset *ts, int t) {
    for (int k = 0; k < TILE * TILE; k++) if (ts->t[t][k] != CLEAR) return false;
    return true;
}

static bool write_tileset(const char *path, const Tileset *ts, char *err, int errlen) {
    int cols = 1, rows = 1;          /* only as much of the sheet as is used */
    for (int t = 0; t < NTILES; t++)
        if (!tile_empty(ts, t)) {
            if (t % SHEET_COLS + 1 > cols) cols = t % SHEET_COLS + 1;
            if (t / SHEET_COLS + 1 > rows) rows = t / SHEET_COLS + 1;
        }
    char tmp[PATH_MAX + 16];
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    FILE *f = fopen(tmp, "w");
    if (!f) { snprintf(err, (size_t)errlen, "could not write %s", tmp); return false; }
    fprintf(f, "%c%c\n", ge_pixel_chars[cols - 1], ge_pixel_chars[rows - 1]);
    for (int y = 0; y < rows * TILE; y++) {
        for (int x = 0; x < cols * TILE; x++) {
            uint8_t c = ts->t[(y / TILE) * SHEET_COLS + x / TILE][(y % TILE) * TILE + x % TILE];
            fputc(c == CLEAR ? '.' : ge_pixel_chars[c & 63], f);
        }
        fputc('\n', f);
    }
    bool ok = fclose(f) == 0 && rename(tmp, path) == 0;
    if (!ok) snprintf(err, (size_t)errlen, "could not write %s", path);
    return ok;
}

static bool parse_map(const char *path, TileMap *m, char *err, int errlen) {
    size_t n;
    char *s = read_packed(path, &n);
    if (!s) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = file_part(path);
    bool ok = false;
    int sw = n >= 2 ? b64(s[0]) + 1 : 0, sh = n >= 2 ? b64(s[1]) + 1 : 0;
    int w = sw * SCREEN_CELLS_W, h = sh * SCREEN_CELLS_H;
    if (sw < 1 || sh < 1) snprintf(err, (size_t)errlen, "%s: starts with its size in screens (two characters, 0 = 1 screen ... 9 = 10)", fn);
    else if (sw > MAP_MAX_SCREENS || sh > MAP_MAX_SCREENS) snprintf(err, (size_t)errlen, "%s: is %d x %d screens; a map is at most %d x %d", fn, sw, sh, MAP_MAX_SCREENS, MAP_MAX_SCREENS);
    else if (n - 2 != (size_t)w * (size_t)h * 2) snprintf(err, (size_t)errlen, "%s: a %d x %d cell map has %d cells (2 characters each), this one has %zu characters", fn, w, h, w * h, n - 2);
    else {
        ok = map_resize(m, w, h);
        for (int i = 0; i < w * h && ok; i++) {
            char a = s[2 + i * 2], b = s[3 + i * 2];
            if (a == '.' && b == '.') { m->cells[i] = MAP_EMPTY; continue; }
            int hi = b64(a), lo = b64(b);
            int t = hi * 64 + lo;
            if (hi < 0 || lo < 0 || t >= NTILES) {
                snprintf(err, (size_t)errlen, "%s: cell x %d y %d is '%c%c', not a tile 0-%d or ..", fn, i % w, i / w, a, b, NTILES - 1);
                ok = false;
            } else m->cells[i] = (uint16_t)t;
        }
    }
    free(s);
    return ok;
}

static bool write_map(const char *path, const TileMap *m, char *err, int errlen) {
    char tmp[PATH_MAX + 16];
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    FILE *f = fopen(tmp, "w");
    if (!f) { snprintf(err, (size_t)errlen, "could not write %s", tmp); return false; }
    fprintf(f, "%c%c\n", ge_pixel_chars[m->w / SCREEN_CELLS_W - 1], ge_pixel_chars[m->h / SCREEN_CELLS_H - 1]);
    for (int y = 0; y < m->h; y++) {
        for (int x = 0; x < m->w; x++) {
            uint16_t t = m->cells[y * m->w + x];
            if (t == MAP_EMPTY) fputs("..", f);
            else { fputc(ge_pixel_chars[(t >> 6) & 63], f); fputc(ge_pixel_chars[t & 63], f); }
        }
        fputc('\n', f);
    }
    bool ok = fclose(f) == 0 && rename(tmp, path) == 0;
    if (!ok) snprintf(err, (size_t)errlen, "could not write %s", path);
    return ok;
}

/* writing goes to NAME.saving first, so a failed save leaves the old file whole */
static FILE *save_begin(const char *path, char *tmp, size_t n, char *err, int errlen) {
    snprintf(tmp, n, "%s.saving", path);
    FILE *f = fopen(tmp, "w");
    if (!f) snprintf(err, (size_t)errlen, "could not write %s", tmp);
    return f;
}

static bool save_end(FILE *f, const char *tmp, const char *path, char *err, int errlen) {
    bool ok = fclose(f) == 0 && rename(tmp, path) == 0;
    if (!ok) snprintf(err, (size_t)errlen, "could not write %s", path);
    return ok;
}

#define SFX_CHARS (4 + SFX_STEPS * 4)            /* one sound effect: its settings, then its steps */
#define CELL_CHARS 4

static bool parse_sound(const char *path, SoundBank *b, char *err, int errlen) {
    size_t n;
    char *s = read_packed(path, &n);
    if (!s) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = file_part(path);
    bool ok = false;
    int nw = n >= 2 ? b64(s[0]) + 1 : 0, ns = n >= 2 ? b64(s[1]) + 1 : 0;
    size_t want = 2 + (size_t)nw * WAVE_LEN + (size_t)ns * SFX_CHARS;
    if (nw < 1 || ns < 1) snprintf(err, (size_t)errlen, "%s: starts with its number of waves and sound effects (two characters 0-9 a-z A-Z + *)", fn);
    else if (nw > OLD_NWAVES) snprintf(err, (size_t)errlen, "%s: has %d waves; a sound bank has at most %d", fn, nw, OLD_NWAVES);
    else if (n != want) snprintf(err, (size_t)errlen, "%s: %d waves and %d sound effects are %zu characters, this one has %zu", fn, nw, ns, want, n);
    else {
        ok = true;
        const char *p = s + 2;
        for (int w = 0; w < nw && ok; w++)
            for (int i = 0; i < WAVE_LEN; i++, p++) {
                int v = b64(*p);
                if (v < 0 || v > 15) {
                    snprintf(err, (size_t)errlen, "%s: wave %d sample %d is '%c', not 0-f", fn, w, i, *p);
                    ok = false;
                    break;
                }
                if (w < NWAVES) b->waves[w][i] = (uint8_t)v;     /* an old bank's waves 8-15 are instruments now */
            }
        for (int k = 0; k < ns && ok; k++, p += SFX_CHARS) {
            Sfx *x = &b->sfx[k];
            int sp = b64(p[0]), len = b64(p[1]), ls = b64(p[2]), le = b64(p[3]);
            if (sp < 0 || len < 0 || len >= SFX_STEPS || ls < 0 || ls > SFX_STEPS || le < 0 || le > SFX_STEPS) {
                snprintf(err, (size_t)errlen, "%s: sfx %d starts '%.4s': speed 1-64, length 1-%d and loop 0-%d, less one for speed and length", fn, k, p, SFX_STEPS, SFX_STEPS);
                ok = false;
                break;
            }
            x->speed = (uint8_t)(sp + 1);
            x->length = (uint8_t)(len + 1);
            x->loop_start = (uint8_t)ls;
            x->loop_end = (uint8_t)le;
            for (int i = 0; i < SFX_STEPS; i++) {
                const char *q = p + 4 + i * 4;
                int pi = b64(q[0]), wv = b64(q[1]), vol = b64(q[2]), fx = b64(q[3]);
                if (pi < 0 || wv < 0 || wv > WAVE_NOISE || vol < 0 || vol > 7 || fx < 0 || fx >= NFX) {
                    snprintf(err, (size_t)errlen, "%s: sfx %d step %d is '%.4s': pitch, wave 0-g, volume 0-7, effect 0-%d", fn, k, i, q, NFX - 1);
                    ok = false;
                    break;
                }
                x->steps[i] = (SfxStep){(uint8_t)pi, (uint8_t)wv, (uint8_t)vol, (uint8_t)fx};
            }
        }
    }
    free(s);
    return ok;
}

static bool sfx_is_default(const Sfx *s) {
    Sfx d;
    synth_default_sfx(&d);
    return !memcmp(&d, s, sizeof d);
}

static bool write_sound(const char *path, const SoundBank *b, char *err, int errlen) {
    int ns = 1;                      /* up to the last sound effect that isn't empty */
    for (int k = 0; k < NSFX; k++) if (!sfx_is_default(&b->sfx[k])) ns = k + 1;
    char tmp[PATH_MAX + 16];
    FILE *f = save_begin(path, tmp, sizeof tmp, err, errlen);
    if (!f) return false;
    fprintf(f, "%c%c\n", ge_pixel_chars[NWAVES - 1], ge_pixel_chars[ns - 1]);
    for (int w = 0; w < NWAVES; w++) {
        for (int i = 0; i < WAVE_LEN; i++) fputc(ge_pixel_chars[b->waves[w][i] & 15], f);
        fputc('\n', f);
    }
    for (int k = 0; k < ns; k++) {
        const Sfx *x = &b->sfx[k];
        fprintf(f, "%c%c%c%c", ge_pixel_chars[(x->speed - 1) & 63], ge_pixel_chars[(x->length - 1) & 63],
                ge_pixel_chars[x->loop_start & 63], ge_pixel_chars[x->loop_end & 63]);
        for (int i = 0; i < SFX_STEPS; i++) {
            const SfxStep *st = &x->steps[i];
            fputc(ge_pixel_chars[st->pitch & 63], f);
            fputc(ge_pixel_chars[st->wave > WAVE_NOISE ? WAVE_NOISE : st->wave], f);
            fputc(ge_pixel_chars[st->vol & 7], f);
            fputc(ge_pixel_chars[st->fx % NFX], f);
        }
        fputc('\n', f);
    }
    return save_end(f, tmp, path, err, errlen);
}

static bool parse_music(const char *path, Song *song, char *err, int errlen) {
    size_t n;
    char *s = read_packed(path, &n);
    if (!s) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = file_part(path);
    bool ok = false;
    int sp = n >= 4 ? b64(s[0]) : -1, no = n >= 4 ? b64(s[1]) + 1 : 0;
    int np = n >= 4 ? b64(s[2]) + 1 : 0, nt = n >= 4 ? b64(s[3]) + 1 : 0;
    size_t want = 4 + (size_t)no * 2 + (size_t)np * PATTERN_ROWS * (size_t)nt * CELL_CHARS;
    bool lens = n == want + (size_t)np;          /* a line of pattern lengths at the end, when one is short */
    if (sp < 0 || no < 1 || np < 1 || nt < 1)
        snprintf(err, (size_t)errlen, "%s: starts with its speed and its numbers of order entries, patterns and tracks (four characters 0-9 a-z A-Z + *)", fn);
    else if (np > NPATTERNS || nt > NTRACKS)
        snprintf(err, (size_t)errlen, "%s: has %d patterns of %d tracks; a song has at most %d of %d", fn, np, nt, NPATTERNS, NTRACKS);
    else if (n != want && !lens)
        snprintf(err, (size_t)errlen, "%s: %d order entries and %d patterns of %d tracks are %zu characters (%zu with the patterns' lengths), this one has %zu",
                 fn, no, np, nt, want, want + (size_t)np, n);
    else {
        ok = true;
        song->song_speed = (uint8_t)(sp + 1);
        const char *p = s + 4;
        for (int i = 0; i < no; i++, p += 2) {
            OrderEntry *e = &song->order[i];
            int kind = b64(p[0]), v = b64(p[1]);
            if (p[0] == '.' && p[1] == '.') { e->kind = ORDER_STOP; e->value = 0; continue; }
            if ((kind == 0 && v >= 0 && v < NPATTERNS) || (kind == 1 && v >= 0 && v < ORDER_LEN)) {
                e->kind = kind == 0 ? ORDER_PATTERN : ORDER_JUMP;
                e->value = (uint8_t)v;
                continue;
            }
            snprintf(err, (size_t)errlen, "%s: order entry %d is '%.2s', not 0N (pattern 0-%d), 1N (jump to entry N) or ..", fn, i, p, NPATTERNS - 1);
            ok = false;
            break;
        }
        for (int pt = 0; pt < np && ok; pt++)
            for (int r = 0; r < PATTERN_ROWS && ok; r++)
                for (int t = 0; t < nt; t++, p += CELL_CHARS) {
                    Cell *c = &song->patterns[pt].cells[r][t];
                    memset(c, 0, sizeof *c);
                    c->pitch = 24;
                    if (!strncmp(p, "....", 4)) { c->kind = CELL_EMPTY; continue; }
                    if (!strncmp(p, "====", 4)) { c->kind = CELL_OFF; continue; }
                    int pi = b64(p[0]), wv = b64(p[1]), vol = b64(p[2]), fx = b64(p[3]);
                    if (pi < 0 || wv < 0 || wv > WAVE_NOISE || vol < 0 || vol > 7 || fx < 0 || fx >= NFX) {
                        snprintf(err, (size_t)errlen, "%s: pattern %d row %d track %d is '%.4s': pitch, wave 0-g, volume 0-7, effect 0-%d, or .... or ====",
                                 fn, pt, r, t + 1, p, NFX - 1);
                        ok = false;
                        break;
                    }
                    *c = (Cell){CELL_NOTE, (uint8_t)pi, (uint8_t)wv, (uint8_t)vol, (uint8_t)fx};
                }
        for (int pt = 0; pt < np && ok; pt++, p += lens) {
            int len = lens ? b64(*p) + 1 : PATTERN_ROWS;
            if (len < 1 || len > PATTERN_ROWS) {
                snprintf(err, (size_t)errlen, "%s: pattern %d's length is '%c', not 0-%c (1 to %d rows)", fn, pt, *p,
                         ge_pixel_chars[PATTERN_ROWS - 1], PATTERN_ROWS);
                ok = false;
                break;
            }
            song->patterns[pt].length = (uint8_t)len;
        }
    }
    free(s);
    return ok;
}

static int pattern_tracks(const Song *song, int p) {      /* tracks in use: up to the last one with anything in it */
    int used = 0;
    for (int r = 0; r < PATTERN_ROWS; r++)
        for (int t = 0; t < NTRACKS; t++)
            if (song->patterns[p].cells[r][t].kind != CELL_EMPTY && t + 1 > used) used = t + 1;
    return used;
}

static bool write_music(const char *path, const Song *song, char *err, int errlen) {
    int no = ORDER_LEN;                          /* order entries up to the last one that isn't a stop */
    while (no > 1 && song->order[no - 1].kind == ORDER_STOP) no--;
    int np = 1, nt = 1;                          /* patterns up to the last used one, as wide as the widest */
    for (int p = 0; p < NPATTERNS; p++) {
        int t = pattern_tracks(song, p);
        if (t || pattern_len(&song->patterns[p]) < PATTERN_ROWS) np = p + 1;
        if (t > nt) nt = t;
    }
    bool lens = false;                           /* the lengths line only when a pattern ends early */
    for (int p = 0; p < np; p++) if (pattern_len(&song->patterns[p]) < PATTERN_ROWS) lens = true;
    char tmp[PATH_MAX + 16];
    FILE *f = save_begin(path, tmp, sizeof tmp, err, errlen);
    if (!f) return false;
    fprintf(f, "%c%c%c%c\n", ge_pixel_chars[(song->song_speed - 1) & 63], ge_pixel_chars[no - 1],
            ge_pixel_chars[np - 1], ge_pixel_chars[nt - 1]);
    for (int i = 0; i < no; i++) {
        const OrderEntry *e = &song->order[i];
        if (e->kind == ORDER_STOP) fputs("..", f);
        else { fputc(e->kind == ORDER_JUMP ? '1' : '0', f); fputc(ge_pixel_chars[e->value & 63], f); }
    }
    fputc('\n', f);
    for (int p = 0; p < np; p++) {
        fputc('\n', f);                          /* a blank line before each pattern */
        for (int r = 0; r < PATTERN_ROWS; r++) {
            for (int t = 0; t < nt; t++) {
                const Cell *c = &song->patterns[p].cells[r][t];
                if (c->kind == CELL_EMPTY) fputs("....", f);
                else if (c->kind == CELL_OFF) fputs("====", f);
                else {
                    fputc(ge_pixel_chars[c->pitch & 63], f);
                    fputc(ge_pixel_chars[c->wave > WAVE_NOISE ? WAVE_NOISE : c->wave], f);
                    fputc(ge_pixel_chars[c->vol & 7], f);
                    fputc(ge_pixel_chars[c->fx % NFX], f);
                }
            }
            fputc('\n', f);
        }
    }
    if (lens) {                                  /* each pattern's length - 1 */
        fputc('\n', f);
        for (int p = 0; p < np; p++) fputc(ge_pixel_chars[pattern_len(&song->patterns[p]) - 1], f);
        fputc('\n', f);
    }
    return save_end(f, tmp, path, err, errlen);
}

static int row_chars(int w) { return (w + 5) / 6; }   /* a glyph row: 6 pixels to a character */

static bool parse_font(const char *path, Font *f, char *err, int errlen) {
    size_t n;
    char *s = read_packed(path, &n);
    if (!s) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = file_part(path);
    bool ok = false;
    int w = n >= 2 ? b64(s[0]) + 1 : 0, h = n >= 2 ? b64(s[1]) + 1 : 0;
    size_t want = 2 + (size_t)NGLYPHS * (size_t)h * (size_t)row_chars(w);
    if (w < 1 || h < 1) snprintf(err, (size_t)errlen, "%s: starts with its character size (two characters 0-9 a-z A-Z + *)", fn);
    else if (w < FONT_MIN || h < FONT_MIN || w > FONT_MAX || h > FONT_MAX)
        snprintf(err, (size_t)errlen, "%s: is %d x %d; a font is %d x %d to %d x %d", fn, w, h, FONT_MIN, FONT_MIN, FONT_MAX, FONT_MAX);
    else if (n != want) snprintf(err, (size_t)errlen, "%s: %d glyphs of %d x %d are %zu characters, this one has %zu", fn, NGLYPHS, w, h, want, n);
    else {
        ok = true;
        memset(f, 0, sizeof *f);
        f->w = (uint8_t)w;
        f->h = (uint8_t)h;
        const char *p = s + 2;
        for (int g = 0; g < NGLYPHS && ok; g++)
            for (int r = 0; r < h && ok; r++)
                for (int k = 0; k < row_chars(w); k++, p++) {
                    int v = b64(*p);
                    if (v < 0) {
                        snprintf(err, (size_t)errlen, "%s: character %d row %d has '%c'", fn, g + FONT_FIRST, r, *p);
                        ok = false;
                        break;
                    }
                    for (int b = 0; b < 6; b++)              /* the character's top bit is its leftmost pixel */
                        if ((v & (32 >> b)) && k * 6 + b < w) f->g[g][r] |= 1u << (k * 6 + b);
                }
    }
    free(s);
    return ok;
}

static bool write_font(const char *path, const Font *f, char *err, int errlen) {
    char tmp[PATH_MAX + 16];
    FILE *out = save_begin(path, tmp, sizeof tmp, err, errlen);
    if (!out) return false;
    fprintf(out, "%c%c\n", ge_pixel_chars[f->w - 1], ge_pixel_chars[f->h - 1]);
    for (int g = 0; g < NGLYPHS; g++) {
        for (int r = 0; r < f->h; r++)
            for (int k = 0; k < row_chars(f->w); k++) {
                int v = 0;
                for (int b = 0; b < 6; b++)
                    if (k * 6 + b < f->w && (f->g[g][r] >> (k * 6 + b) & 1)) v |= 32 >> b;
                fputc(ge_pixel_chars[v], out);
            }
        fputc('\n', out);
    }
    return save_end(out, tmp, path, err, errlen);
}

/* write a resource in its .g64 form */
static bool write_file(int kind, const char *path, const void *data, char *err, int errlen) {
    switch (kind) {
        case RES_TILES: return write_tileset(path, data, err, errlen);
        case RES_MAP: return write_map(path, data, err, errlen);
        case RES_SFX: return write_sound(path, data, err, errlen);
        case RES_FONT: return write_font(path, data, err, errlen);
        default: return write_music(path, data, err, errlen);
    }
}

/* ---------------------------------------------------------------- resources in memory */

bool map_resize(TileMap *m, int w, int h) {
    if (w < 1 || h < 1 || w > MAP_MAX_W || h > MAP_MAX_H) return false;
    if (m->cells && m->w == w && m->h == h) return true;
    uint16_t *c = malloc(sizeof *c * (size_t)w * (size_t)h);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) c[y * w + x] = map_cell(m->cells ? m : NULL, x, y);
    free(m->cells);
    m->cells = c;
    m->w = w;
    m->h = h;
    return true;
}

void *res_new(int kind) {
    switch (kind) {
        case RES_TILES: { Tileset *t = malloc(sizeof *t); memset(t->t, CLEAR, sizeof t->t); return t; }
        case RES_MAP: {
            TileMap *m = calloc(1, sizeof *m);
            map_resize(m, 4 * SCREEN_CELLS_W, 4 * SCREEN_CELLS_H);
            return m;
        }
        case RES_SFX: { SoundBank *b = calloc(1, sizeof *b); synth_defaults(b); return b; }
        case RES_FONT: { Font *f = malloc(sizeof *f); memcpy(f, &sys_font, sizeof *f); return f; }   /* a new font starts as the built-in one */
        default: { Song *s = calloc(1, sizeof *s); music_defaults(s); return s; }
    }
}

void *res_copy(int kind, const void *p) {
    switch (kind) {
        case RES_TILES: { Tileset *t = malloc(sizeof *t); memcpy(t, p, sizeof *t); return t; }
        case RES_MAP: {
            const TileMap *from = p;
            TileMap *m = calloc(1, sizeof *m);
            m->w = from->w;
            m->h = from->h;
            m->cells = malloc(sizeof *m->cells * (size_t)m->w * (size_t)m->h);
            memcpy(m->cells, from->cells, sizeof *m->cells * (size_t)m->w * (size_t)m->h);
            return m;
        }
        case RES_SFX: { SoundBank *b = malloc(sizeof *b); memcpy(b, p, sizeof *b); return b; }
        case RES_FONT: { Font *f = malloc(sizeof *f); memcpy(f, p, sizeof *f); return f; }
        default: { Song *s = malloc(sizeof *s); memcpy(s, p, sizeof *s); return s; }
    }
}

void res_free(int kind, void *p) {
    if (!p) return;
    if (kind == RES_MAP) free(((TileMap *)p)->cells);
    if (kind == RES_SFX || kind == RES_MUSIC) {
        audio_lock();
        synth_forget(p);
        audio_unlock();
    }
    free(p);
}

bool res_name_ok(const char *name) { return cart_valid_name(name) && strlen(name) < 48; }

void res_path(int kind, const char *name, char *out, size_t n) {
    snprintf(out, n, "%s/%s/%s%s", cart.dir, res_dir[kind], name, res_ext[kind]);
}

static bool is_file(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISREG(st.st_mode); }

static bool parse_file(int kind, const char *path, void *data, char *err, int errlen) {
    switch (kind) {
        case RES_TILES: return parse_tileset(path, data, err, errlen);
        case RES_MAP: return parse_map(path, data, err, errlen);
        case RES_SFX: return parse_sound(path, data, err, errlen);
        case RES_FONT: return parse_font(path, data, err, errlen);
        default: return parse_music(path, data, err, errlen);
    }
}

/* the old text form of a sound bank or song: sfx/NAME.txt, music/NAME.txt (from the
 * cart folder). False for tilesets and maps, which never had one in their folder. */
static bool legacy_txt(int kind, const char *name, char *rel, size_t n) {
    if (kind != RES_SFX && kind != RES_MUSIC) return false;
    snprintf(rel, n, "%s/%s.txt", res_dir[kind], name);
    return true;
}

static void cart_file(const char *rel, char *out, size_t n) { snprintf(out, n, "%s/%s", cart.dir, rel); }

/* ---------------------------------------------------------------- documents */

void docs_reset(void) {
    cur_tiles = NULL;
    cur_map = NULL;
    cur_snd = NULL;
    cur_song = NULL;
    cur_font = NULL;
    for (int i = 0; i < cart.ndocs; i++) {
        res_free(cart.docs[i]->kind, cart.docs[i]->data);
        free(cart.docs[i]);
    }
    cart.ndocs = 0;
    memset(cart.active, 0, sizeof cart.active);
    tiles_doc_changed();      /* the editors' undo histories were for the old files */
    map_doc_changed();
    sound_doc_changed();
    music_doc_changed();
    font_doc_changed();
}

Doc *doc_find(int kind, const char *name) {
    for (int i = 0; i < cart.ndocs; i++)
        if (cart.docs[i]->kind == kind && !strcmp(cart.docs[i]->name, name)) return cart.docs[i];
    return NULL;
}

static Doc *add_doc(int kind, const char *name, void *data, char *err, int errlen) {
    if (cart.ndocs == MAX_DOCS) {
        snprintf(err, (size_t)errlen, "too many files open (%d)", MAX_DOCS);
        res_free(kind, data);
        return NULL;
    }
    Doc *d = calloc(1, sizeof *d);
    d->kind = kind;
    snprintf(d->name, sizeof d->name, "%s", name);
    d->data = data;
    cart.docs[cart.ndocs++] = d;
    return d;
}

Doc *doc_open(int kind, const char *name, char *err, int errlen) {
    Doc *d = doc_find(kind, name);
    if (d) return d;
    if (!res_name_ok(name)) { snprintf(err, (size_t)errlen, "'%s' is not a file name (letters, digits, - and _)", name); return NULL; }
    char path[PATH_MAX + 128];
    res_path(kind, name, path, sizeof path);
    char rel[80];
    const char *legacy = NULL;
    if (!is_file(path) && legacy_txt(kind, name, rel, sizeof rel)) {   /* sound or music still in text: sfx/NAME.txt */
        cart_file(rel, path, sizeof path);
        if (is_file(path)) legacy = rel;
    }
    if (!legacy && !is_file(path) && legacy_file[kind] && !strcmp(name, res_default[kind])) {   /* an old cart: tiles.txt ... */
        snprintf(path, sizeof path, "%s/%s", cart.dir, legacy_file[kind]);
        if (is_file(path)) legacy = legacy_file[kind];
    }
    if (!legacy) res_path(kind, name, path, sizeof path);
    if (!is_file(path)) { snprintf(err, (size_t)errlen, "no %s/%s%s", res_dir[kind], name, res_ext[kind]); return NULL; }
    void *data = res_new(kind);
    bool ok = !legacy ? parse_file(kind, path, data, err, errlen)
            : kind == RES_TILES ? res_parse_legacy_tiles(path, data, err, errlen)
            : kind == RES_MAP ? res_parse_legacy_map(path, data, err, errlen)
            : kind == RES_SFX ? res_parse_legacy_sound(path, data, err, errlen)
            : res_parse_legacy_music(path, data, err, errlen);
    if (!ok) { res_free(kind, data); return NULL; }
    d = add_doc(kind, name, data, err, errlen);
    if (d && legacy) {
        snprintf(d->legacy, sizeof d->legacy, "%s", legacy);
        d->dirty = true;           /* SAVE writes it in the new place */
    }
    return d;
}

Doc *doc_create(int kind, const char *name, char *err, int errlen) {
    if (!res_name_ok(name)) { snprintf(err, (size_t)errlen, "'%s' is not a file name (letters, digits, - and _)", name); return NULL; }
    if (doc_find(kind, name)) { snprintf(err, (size_t)errlen, "%s/%s%s is already open", res_dir[kind], name, res_ext[kind]); return NULL; }
    char path[PATH_MAX + 128];
    res_path(kind, name, path, sizeof path);
    if (is_file(path)) { snprintf(err, (size_t)errlen, "%s/%s%s already exists", res_dir[kind], name, res_ext[kind]); return NULL; }
    Doc *d = add_doc(kind, name, res_new(kind), err, errlen);
    if (d) d->dirty = true;
    return d;
}

bool docs_load_defaults(char *err, int errlen) {
    for (int k = 0; k < NRES; k++) {
        char e[512];
        Doc *d = doc_open(k, res_default[k], e, sizeof e);
        if (d) {
            if (d->legacy[0] && !opt.player)
                fprintf(stderr, "gooberengine: read the old %s; SAVE writes it as %s/%s%s\n", d->legacy, res_dir[k], d->name, res_ext[k]);
            continue;
        }
        if (strncmp(e, "no ", 3)) { snprintf(err, (size_t)errlen, "%s", e); return false; }   /* a broken file */
    }
    return true;
}

Doc *doc_active(int kind) {
    if (cart.active[kind]) return cart.active[kind];
    char err[256];
    Doc *d = doc_find(kind, res_default[kind]);
    if (!d) {          /* no default-named file: the cart's first one, else an empty one (not on disk yet) */
        const char *names[MAX_DOCS * 2];
        int n = docs_list(kind, names, MAX_DOCS * 2);
        for (int i = 0; i < n && !d; i++) d = doc_open(kind, names[i], err, sizeof err);
    }
    if (!d) d = add_doc(kind, res_default[kind], res_new(kind), err, sizeof err);
    if (!d && cart.ndocs) d = cart.docs[0];
    cart.active[kind] = d;
    return d;
}

void doc_set_active(Doc *d) { cart.active[d->kind] = d; }

void doc_touch(int kind) { doc_active(kind)->dirty = true; }

bool doc_save(Doc *d, char *err, int errlen) {
    if (cart.untitled) { snprintf(err, (size_t)errlen, "untitled cart: type SAVE name in the shell to keep it"); return false; }
    char dir[PATH_MAX + 64], path[PATH_MAX + 128];
    snprintf(dir, sizeof dir, "%s/%s", cart.dir, res_dir[d->kind]);
    mkdir(dir, 0755);
    res_path(d->kind, d->name, path, sizeof path);
    if (!write_file(d->kind, path, d->data, err, errlen)) return false;
    d->dirty = false;
    if (d->legacy[0]) {          /* the old file is replaced: keep it as *.old, out of the way */
        char from[PATH_MAX + 128], to[PATH_MAX + 144];
        cart_file(d->legacy, from, sizeof from);
        snprintf(to, sizeof to, "%s.old", from);
        rename(from, to);
        d->legacy[0] = 0;
    }
    return true;
}

static bool file_exists(const char *p) { struct stat st; return stat(p, &st) == 0; }

/* rename a file of the cart: the open document and the file on disk (if it's there yet) */
bool doc_rename(int kind, const char *name, const char *to, char *err, int errlen) {
    if (!res_name_ok(to)) { snprintf(err, (size_t)errlen, "'%s' is not a file name (letters, digits, - and _)", to); return false; }
    if (!strcmp(name, to)) return true;
    char from_p[PATH_MAX + 128], to_p[PATH_MAX + 128], from_t[PATH_MAX + 128], to_t[PATH_MAX + 128];
    char from_rel[80], to_rel[80];
    res_path(kind, name, from_p, sizeof from_p);
    res_path(kind, to, to_p, sizeof to_p);
    bool txt = legacy_txt(kind, name, from_rel, sizeof from_rel) && legacy_txt(kind, to, to_rel, sizeof to_rel);
    if (txt) { cart_file(from_rel, from_t, sizeof from_t); cart_file(to_rel, to_t, sizeof to_t); }
    if (doc_find(kind, to) || file_exists(to_p) || (txt && file_exists(to_t))) {
        snprintf(err, (size_t)errlen, "%s/%s%s already exists", res_dir[kind], to, res_ext[kind]);
        return false;
    }
    Doc *d = doc_find(kind, name);
    if (file_exists(from_p) && rename(from_p, to_p) != 0) { snprintf(err, (size_t)errlen, "could not rename %s", from_p); return false; }
    if (txt && file_exists(from_t) && rename(from_t, to_t) != 0) { snprintf(err, (size_t)errlen, "could not rename %s", from_t); return false; }
    cart_slot_rename(kind, name, to);      /* before the doc's name changes: an old cart's slot 0 is found by it */
    if (d) {
        snprintf(d->name, sizeof d->name, "%s", to);
        if (txt && !strcmp(d->legacy, from_rel)) snprintf(d->legacy, sizeof d->legacy, "%s", to_rel);
    }
    return true;
}

/* delete a file of the cart: from disk and from the editors */
bool doc_delete(int kind, const char *name, char *err, int errlen) {
    char path[PATH_MAX + 128];
    cart_slot_forget(kind, name);
    res_path(kind, name, path, sizeof path);
    if (file_exists(path) && remove(path) != 0) { snprintf(err, (size_t)errlen, "could not delete %s", path); return false; }
    char rel[80], txt[PATH_MAX + 128];
    if (legacy_txt(kind, name, rel, sizeof rel)) {     /* the old text form goes too, or it would come back */
        cart_file(rel, txt, sizeof txt);
        if (file_exists(txt)) remove(txt);
    }
    for (int i = 0; i < cart.ndocs; i++) {
        Doc *d = cart.docs[i];
        if (d->kind != kind || strcmp(d->name, name)) continue;
        if (d->legacy[0]) {          /* an old cart's file: it goes too, or it would come back */
            char old[PATH_MAX + 128];
            cart_file(d->legacy, old, sizeof old);
            remove(old);
        }
        bool was_active = cart.active[kind] == d;
        if (was_active) cart.active[kind] = NULL;
        memmove(cart.docs + i, cart.docs + i + 1, sizeof cart.docs[0] * (size_t)(cart.ndocs - i - 1));
        cart.ndocs--;
        if (was_active) {             /* the editor moves to another file before this one is freed */
            switch (kind) {
                case RES_TILES: cur_tiles = NULL; tiles_doc_changed(); break;
                case RES_MAP: cur_map = NULL; map_doc_changed(); break;
                case RES_SFX: audio_lock(); cur_snd = NULL; audio_unlock(); sound_doc_changed(); break;
                case RES_FONT: cur_font = NULL; font_doc_changed(); break;
                default: audio_lock(); cur_song = NULL; audio_unlock(); music_doc_changed(); break;
            }
        }
        res_free(kind, d->data);
        free(d);
        docs_bind();
        break;
    }
    return true;
}

bool docs_dirty(int kind) {
    for (int i = 0; i < cart.ndocs; i++)
        if ((kind < 0 || cart.docs[i]->kind == kind) && cart.docs[i]->dirty) return true;
    return false;
}

bool docs_save_all(int kind, char *msg, int msglen) {
    msg[0] = 0;
    bool ok = true;
    for (int i = 0; i < cart.ndocs; i++) {
        Doc *d = cart.docs[i];
        if ((kind >= 0 && d->kind != kind) || !d->dirty) continue;
        char err[512];
        size_t n = strlen(msg);
        if (doc_save(d, err, sizeof err))
            snprintf(msg + n, (size_t)msglen - n, "%s%s/%s%s", n ? ", " : "saved ", res_dir[d->kind], d->name, res_ext[d->kind]);
        else {
            snprintf(msg, (size_t)msglen, "%s", err);
            ok = false;
            break;
        }
    }
    return ok;
}

/* the window closed with unsaved files: keep them beside the cart as NAME.g64.unsaved ... */
void docs_emergency_save(void) {
    if (!cart.loaded) return;
    for (int i = 0; i < cart.ndocs; i++) {
        Doc *d = cart.docs[i];
        if (!d->dirty || d->legacy[0]) continue;
        char dir[PATH_MAX + 64], path[PATH_MAX + 128], err[512];
        snprintf(dir, sizeof dir, "%s/%s", cart.dir, res_dir[d->kind]);
        mkdir(dir, 0755);
        snprintf(path, sizeof path, "%s/%s%s.unsaved", dir, d->name, res_ext[d->kind]);
        if (write_file(d->kind, path, d->data, err, sizeof err)) fprintf(stderr, "gooberengine: unsaved %s kept in %s\n", res_word[d->kind], path);
    }
}

void docs_bind(void) {
    if (in_cart || !cart.loaded) return;
    cur_tiles = doc_active(RES_TILES)->data;
    cur_map = doc_active(RES_MAP)->data;
    audio_lock();
    cur_snd = doc_active(RES_SFX)->data;
    cur_song = doc_active(RES_MUSIC)->data;
    audio_unlock();
    cur_font = doc_active(RES_FONT)->data;
}

static int cmp_str(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }

/* the files of a kind: on disk, or open (a new one may not be saved yet) */
int docs_list(int kind, const char **names, int max) {
    static char store[NRES][MAX_DOCS * 2][48];
    int n = 0;
    for (int i = 0; i < cart.ndocs && n < max && n < MAX_DOCS * 2; i++)
        if (cart.docs[i]->kind == kind) { snprintf(store[kind][n], 48, "%s", cart.docs[i]->name); names[n] = store[kind][n]; n++; }
    char dir[PATH_MAX + 64];
    snprintf(dir, sizeof dir, "%s/%s", cart.dir, res_dir[kind]);
    DIR *dp = opendir(dir);
    struct dirent *e;
    bool txt_too = kind == RES_SFX || kind == RES_MUSIC;    /* not converted yet: sfx/NAME.txt */
    while (dp && (e = readdir(dp)) && n < max && n < MAX_DOCS * 2) {
        size_t len = strlen(e->d_name);
        const char *ext = len > 4 && !strcmp(e->d_name + len - 4, res_ext[kind]) ? res_ext[kind]
                        : txt_too && len > 4 && !strcmp(e->d_name + len - 4, ".txt") ? ".txt" : NULL;
        size_t el = ext ? strlen(ext) : 0;
        if (!ext || len - el >= 48) continue;
        char stem[48];
        snprintf(stem, sizeof stem, "%.*s", (int)(len - el), e->d_name);
        if (!res_name_ok(stem) || doc_find(kind, stem)) continue;
        bool dup = false;                                    /* NAME.g64 and NAME.txt: one file */
        for (int i = 0; i < n && !dup; i++) dup = !strcmp(names[i], stem);
        if (dup) continue;
        snprintf(store[kind][n], 48, "%s", stem);
        names[n] = store[kind][n];
        n++;
    }
    if (dp) closedir(dp);
    qsort(names, (size_t)n, sizeof names[0], cmp_str);
    return n;
}

/* ---------------------------------------------------------------- the run-time slots */

static struct { void *slot[NRES][NSLOTS]; int cur[NRES]; } rt;

static void rt_point(int kind) {
    void *p = rt.slot[kind][rt.cur[kind]];
    switch (kind) {
        case RES_TILES: cur_tiles = p; break;
        case RES_MAP: cur_map = p; break;
        case RES_SFX: audio_lock(); cur_snd = p; audio_unlock(); break;
        case RES_FONT: cur_font = p; break;
        default: audio_lock(); cur_song = p; audio_unlock(); break;
    }
}

/* every slot the cart fills by itself (the file tree's numbers, cart.txt's
 * slot lines) gets its file; slot 0 is never empty, so there's something to draw on */
void rt_begin(void) {
    rt_end();
    for (int k = 0; k < NRES; k++) {
        for (int s = 0; s < NSLOTS; s++) {
            const char *name = cart_slot_name(k, s);
            char err[512], msg[600];
            if (name && !rt_load(k, s, name, err, sizeof err)) {
                snprintf(msg, sizeof msg, "%s slot %d: %s", res_word[k], s, err);
                shell_print(msg, COL_ORANGE);
                if (opt.headless) fprintf(stderr, "%s\n", msg);
            }
        }
        if (!rt.slot[k][0]) rt.slot[k][0] = res_new(k);
        rt.cur[k] = 0;
        rt_point(k);
    }
}

void rt_end(void) {
    for (int k = 0; k < NRES; k++) {
        for (int s = 0; s < NSLOTS; s++) {
            res_free(k, rt.slot[k][s]);
            rt.slot[k][s] = NULL;
        }
        rt.cur[k] = 0;
        rt_point(k);
    }
    docs_bind();
}

/* what the editors have open (unsaved edits included) wins over the file on disk,
 * so Ctrl+R runs what you see */
bool rt_load(int kind, int slot, const char *name, char *err, int errlen) {
    if (slot < 0 || slot >= NSLOTS) { snprintf(err, (size_t)errlen, "slot %d out of range 0-%d", slot, NSLOTS - 1); return false; }
    void *data = NULL;
    if (name) {
        Doc *d = doc_open(kind, name, err, errlen);
        if (!d) return false;
        data = res_copy(kind, d->data);
    }
    void *old = rt.slot[kind][slot];
    rt.slot[kind][slot] = data;
    if (slot == rt.cur[kind]) rt_point(kind);
    res_free(kind, old);
    return true;
}

bool rt_swap(int kind, int slot, char *err, int errlen) {
    if (slot < 0 || slot >= NSLOTS) { snprintf(err, (size_t)errlen, "slot %d out of range 0-%d", slot, NSLOTS - 1); return false; }
    if (!rt.slot[kind][slot]) { snprintf(err, (size_t)errlen, "slot %d is empty (load a %s into it first)", slot, res_word[kind]); return false; }
    rt.cur[kind] = slot;
    rt_point(kind);
    return true;
}

int rt_current(int kind) { return rt.cur[kind]; }
