/* Resource files, the documents the editors work on, and the slots a cart
 * loads them into while it runs.
 *
 * Files (in the cart folder):
 *   tiles/NAME.g64   tileset    maps/NAME.g64   map
 *   sfx/NAME.txt     sound bank music/NAME.txt  song
 *
 * The .g64 files are base-64 text. Each character of the alphabet
 *   0-9 a-z A-Z + *
 * stands for a number 0-63 (in that order: the same characters stand for the
 * same colours as in the old tiles.txt). Whitespace is ignored, so a file can
 * be broken into lines; GooberEngine writes one row per line.
 *
 *   tileset   X Y then X*10 x Y*10 pixel characters, row by row
 *             X, Y: width and height in pixels as (digit + 1) * 10, so 10-640;
 *             a tileset holds at most 400 x 240 (960 tiles, 40 per row).
 *             A pixel is a colour 0-63, or '.' for transparent.
 *   map       X Y then two characters per cell, row by row
 *             X, Y: width and height in screens as digit + 1, at most 10
 *             (a screen is 40 x 24 cells). A cell is a tile number hi*64 + lo,
 *             or ".." for no tile. Tile 0 is a tile like any other.
 *
 * Old carts kept one of each at the top level (tiles.txt, map.txt, sfx.txt,
 * music.txt). Those are read as the default-named file and written to the new
 * place by the next SAVE, which renames the old file to *.old. */
#include "engine.h"
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char *const res_dir[NRES] = {"tiles", "maps", "sfx", "music"};
const char *const res_ext[NRES] = {".g64", ".g64", ".txt", ".txt"};
const char *const res_default[NRES] = {"tileset", "map", "sfx", "music"};
const char *const res_word[NRES] = {"tileset", "map", "sound", "music"};
static const char *const legacy_file[NRES] = {"tiles.txt", "map.txt", "sfx.txt", "music.txt"};

Tileset *cur_tiles;
TileMap *cur_map;
SoundBank *cur_snd;
Song *cur_song;

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
                    snprintf(err, (size_t)errlen, "%s: '%c' at x %d y %d is not a colour (0-9 a-z A-Z + *, or . for clear)", fn, c, x, y);
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
        case RES_SFX: {
            SoundBank *b = malloc(sizeof *b);
            memcpy(b, p, sizeof *b);
            b->header = NULL;
            memset(b->note, 0, sizeof b->note);
            return b;
        }
        default: {
            Song *s = malloc(sizeof *s);
            memcpy(s, p, sizeof *s);
            s->header = NULL;
            memset(s->note, 0, sizeof s->note);
            return s;
        }
    }
}

void res_free(int kind, void *p) {
    if (!p) return;
    if (kind == RES_MAP) free(((TileMap *)p)->cells);
    if (kind == RES_SFX) {
        SoundBank *b = p;
        free(b->header);
        for (int i = 0; i < NSFX; i++) free(b->note[i]);
    }
    if (kind == RES_MUSIC) {
        Song *s = p;
        free(s->header);
        for (int i = 0; i < NPATTERNS; i++) free(s->note[i]);
    }
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
        case RES_SFX: return res_parse_sound(path, data, err, errlen);
        default: return res_parse_music(path, data, err, errlen);
    }
}

/* ---------------------------------------------------------------- documents */

void docs_reset(void) {
    cur_tiles = NULL;
    cur_map = NULL;
    cur_snd = NULL;
    cur_song = NULL;
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
    const char *legacy = NULL;
    if (!is_file(path) && !strcmp(name, res_default[kind])) {   /* an old cart: tiles.txt ... */
        snprintf(path, sizeof path, "%s/%s", cart.dir, legacy_file[kind]);
        if (is_file(path)) legacy = legacy_file[kind];
    }
    if (!is_file(path)) { snprintf(err, (size_t)errlen, "no %s/%s%s", res_dir[kind], name, res_ext[kind]); return NULL; }
    void *data = res_new(kind);
    bool ok = legacy && kind == RES_TILES ? res_parse_legacy_tiles(path, data, err, errlen)
            : legacy && kind == RES_MAP ? res_parse_legacy_map(path, data, err, errlen)
            : parse_file(kind, path, data, err, errlen);
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
    bool ok = d->kind == RES_TILES ? write_tileset(path, d->data, err, errlen)
            : d->kind == RES_MAP ? write_map(path, d->data, err, errlen)
            : d->kind == RES_SFX ? res_write_sound(path, d->data, err, errlen)
            : res_write_music(path, d->data, err, errlen);
    if (!ok) return false;
    d->dirty = false;
    if (d->legacy[0]) {          /* the old file is replaced: keep it as *.old, out of the way */
        char from[PATH_MAX + 64], to[PATH_MAX + 80];
        snprintf(from, sizeof from, "%s/%s", cart.dir, d->legacy);
        snprintf(to, sizeof to, "%s/%s.old", cart.dir, d->legacy);
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
    char from_p[PATH_MAX + 128], to_p[PATH_MAX + 128];
    res_path(kind, name, from_p, sizeof from_p);
    res_path(kind, to, to_p, sizeof to_p);
    if (doc_find(kind, to) || file_exists(to_p)) { snprintf(err, (size_t)errlen, "%s/%s%s already exists", res_dir[kind], to, res_ext[kind]); return false; }
    Doc *d = doc_find(kind, name);
    if (file_exists(from_p) && rename(from_p, to_p) != 0) { snprintf(err, (size_t)errlen, "could not rename %s", from_p); return false; }
    if (d) snprintf(d->name, sizeof d->name, "%s", to);
    return true;
}

/* delete a file of the cart: from disk and from the editors */
bool doc_delete(int kind, const char *name, char *err, int errlen) {
    char path[PATH_MAX + 128];
    res_path(kind, name, path, sizeof path);
    if (file_exists(path) && remove(path) != 0) { snprintf(err, (size_t)errlen, "could not delete %s", path); return false; }
    for (int i = 0; i < cart.ndocs; i++) {
        Doc *d = cart.docs[i];
        if (d->kind != kind || strcmp(d->name, name)) continue;
        if (d->legacy[0]) {          /* an old cart's file: it goes too, or it would come back */
            char old[PATH_MAX + 64];
            snprintf(old, sizeof old, "%s/%s", cart.dir, d->legacy);
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
        bool ok = d->kind == RES_TILES ? write_tileset(path, d->data, err, sizeof err)
                : d->kind == RES_MAP ? write_map(path, d->data, err, sizeof err)
                : d->kind == RES_SFX ? res_write_sound(path, d->data, err, sizeof err)
                : res_write_music(path, d->data, err, sizeof err);
        if (ok) fprintf(stderr, "gooberengine: unsaved %s kept in %s\n", res_word[d->kind], path);
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
    size_t el = strlen(res_ext[kind]);
    while (dp && (e = readdir(dp)) && n < max && n < MAX_DOCS * 2) {
        size_t len = strlen(e->d_name);
        if (len <= el || strcmp(e->d_name + len - el, res_ext[kind]) || len - el >= 48) continue;
        char stem[48];
        snprintf(stem, sizeof stem, "%.*s", (int)(len - el), e->d_name);
        if (!res_name_ok(stem) || doc_find(kind, stem)) continue;
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
        default: audio_lock(); cur_song = p; audio_unlock(); break;
    }
}

/* slot 0 of each kind starts with the default-named file, or an empty one */
void rt_begin(void) {
    rt_end();
    for (int k = 0; k < NRES; k++) {
        char err[256];
        Doc *d = doc_find(k, res_default[k]);
        if (!d) d = doc_open(k, res_default[k], err, sizeof err);
        rt.slot[k][0] = d ? res_copy(k, d->data) : res_new(k);   /* no file: an empty one to draw on */
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
