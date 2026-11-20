/* Folder cartridges.
 *
 *   mycart/
 *     main.gb      the program (may `include` other .gb files next to it)
 *     cart.txt     title = ..., author = ..., version = ...       (optional)
 *     tiles/       tilesets, NAME.g64                              (optional)
 *     maps/        maps, NAME.g64                                  (optional)
 *     sfx/         sound banks, NAME.txt                           (optional)
 *     music/       songs, NAME.txt                                 (optional)
 *
 * The cart's code is compiled by the GooberBASIC compiler and run on its VM;
 * the console API it calls is the native table in api.c. */
#include "engine.h"
#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "vm.h"
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <strings.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>

Cart cart;
bool in_cart;

/* default pixel alphabet: character i is palette index i; '.' is transparent */
const char ge_pixel_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+*";

static bool is_dir(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISDIR(st.st_mode); }
static bool is_file(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISREG(st.st_mode); }

static char root[PATH_MAX];

void carts_root_set(const char *dir) { snprintf(root, sizeof root, "%s", dir); }

const char *carts_root(void) {
    if (root[0]) return root;
    const char *env = getenv("GOOBERENGINE_CARTS");
    if (env && is_dir(env)) { snprintf(root, sizeof root, "%s", env); return root; }
    if (is_dir("carts") && realpath("carts", root)) return root;
    char *base = SDL_GetBasePath();     /* folder of the executable */
    if (base) {
        snprintf(root, sizeof root, "%scarts", base);
        SDL_free(base);
        if (is_dir(root)) return root;
    }
    snprintf(root, sizeof root, ".");
    return root;
}

/* "name" -> carts/name ; a path to a folder (or to its main.gb) is used as is */
static bool resolve(const char *arg, char *out, size_t n) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof tmp, "%s", arg);
    size_t len = strlen(tmp);
    while (len > 1 && tmp[len - 1] == '/') tmp[--len] = 0;
    if (len > 8 && !strcmp(tmp + len - 8, "/main.gb")) tmp[len - 8] = 0;
    char cand[PATH_MAX + 16];
    snprintf(cand, sizeof cand, "%s/main.gb", tmp);
    if (!is_file(cand)) {
        snprintf(tmp, sizeof tmp, "%s/%s", carts_root(), arg);
        snprintf(cand, sizeof cand, "%s/main.gb", tmp);
        if (!is_file(cand)) return false;
    }
    if (!realpath(tmp, out)) snprintf(out, n, "%s", tmp);
    return true;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) *--e = 0;
    return s;
}

static FILE *open_in(const char *dir, const char *file) {
    char p[PATH_MAX + 64];
    snprintf(p, sizeof p, "%s/%s", dir, file);
    return fopen(p, "r");
}

static void load_meta(void) {
    FILE *f = open_in(cart.dir, "cart.txt");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        char *s = trim(line);
        if (!*s || *s == '#') continue;
        char *eq = strchr(s, '=');
        if (!eq) continue;
        *eq = 0;
        char *k = trim(s), *v = trim(eq + 1);
        if (!strcasecmp(k, "title")) snprintf(cart.title, sizeof cart.title, "%s", v);
        else if (!strcasecmp(k, "author")) snprintf(cart.author, sizeof cart.author, "%s", v);
        else if (!strcasecmp(k, "version")) snprintf(cart.version, sizeof cart.version, "%s", v);
    }
    fclose(f);
}

/* comment lines are kept so saving from the editors writes them back */
static char *append_line(char *buf, const char *s) {
    size_t a = buf ? strlen(buf) : 0, b = strlen(s);
    buf = realloc(buf, a + b + 2);
    memcpy(buf + a, s, b);
    buf[a + b] = '\n';
    buf[a + b + 1] = 0;
    return buf;
}

/* ---- the old formats, read once and converted (SAVE writes the new files) ----
 * tiles.txt:
 *   key r 48        (optional: make 'r' mean palette index 48 in this file)
 *   tile 1
 *   ..rrrrrr..      (10 rows of 10 characters follow)
 * Tile numbers stay the same, so sprite(5, ...) still draws the same tile. */
bool res_parse_legacy_tiles(const char *path, Tileset *ts, char *err, int errlen) {
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    int key[256];
    for (int i = 0; i < 256; i++) key[i] = -1;
    for (int i = 0; ge_pixel_chars[i]; i++) key[(unsigned char)ge_pixel_chars[i]] = i;
    key['.'] = CLEAR;
    char line[512];
    int lineno = 0, cur = -1, row = 0;
    bool ok = true;
    while (ok && fgets(line, sizeof line, f)) {
        lineno++;
        char *s = trim(line);
        if (cur >= 0) {
            if (!*s || *s == '#') continue;
            for (int i = 0; i < TILE; i++) {
                int c = i < (int)strlen(s) ? key[(unsigned char)s[i]] : CLEAR;
                if (c < 0) {
                    snprintf(err, (size_t)errlen, "tiles.txt:%d: '%c' is not a colour (add a key line)", lineno, s[i]);
                    ok = false;
                    break;
                }
                ts->t[cur][row * TILE + i] = (uint8_t)c;
            }
            if (++row == TILE) cur = -1;
            continue;
        }
        if (!*s || *s == '#') continue;
        char ch;
        int n;
        if (sscanf(s, "tile %d", &n) == 1) {
            if (n < 0 || n >= 256) { snprintf(err, (size_t)errlen, "tiles.txt:%d: tile %d out of range 0-255", lineno, n); ok = false; }
            cur = n;
            row = 0;
        } else if (sscanf(s, "key %c %d", &ch, &n) == 2 && n >= 0 && n < NCOLORS && ch != '.') {
            key[(unsigned char)ch] = n;
        } else {
            snprintf(err, (size_t)errlen, "tiles.txt:%d: expected 'tile N' or 'key C N'", lineno);
            ok = false;
        }
    }
    fclose(f);
    return ok;
}

/* map.txt: "row Y" then hex tile numbers for x = 0, 1, 2 ... on a 160 x 96 map,
 * where tile 0 meant "nothing here" */
bool res_parse_legacy_map(const char *path, TileMap *m, char *err, int errlen) {
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    map_resize(m, 160, 96);
    static char line[160 * 4 + 64];
    int lineno = 0;
    while (fgets(line, sizeof line, f)) {
        lineno++;
        char *s = trim(line);
        if (*s == '#' || !*s || !strncmp(s, "size", 4)) continue;
        int y, used;
        if (sscanf(s, "row %d%n", &y, &used) != 1 || y < 0 || y >= m->h) {
            snprintf(err, (size_t)errlen, "map.txt:%d: expected 'row Y ...' with Y in 0-%d", lineno, m->h - 1);
            fclose(f);
            return false;
        }
        char *p = s + used;
        for (int x = 0; x < m->w; x++) {
            char *end;
            long v = strtol(p, &end, 16);
            if (end == p) break;
            v &= 0xFF;
            m->cells[y * m->w + x] = v ? (uint16_t)v : MAP_EMPTY;
            p = end;
        }
    }
    fclose(f);
    return true;
}

static FILE *open_for_save(const char *path, char *tmp, size_t n, char *err, int errlen) {
    snprintf(tmp, n, "%s.saving", path);
    FILE *f = fopen(tmp, "w");
    if (!f) snprintf(err, (size_t)errlen, "could not write %s", tmp);
    return f;
}

static bool finish_save(FILE *f, const char *tmp, const char *path, char *err, int errlen) {
    bool ok = fclose(f) == 0 && rename(tmp, path) == 0;
    if (!ok) snprintf(err, (size_t)errlen, "could not write %s", path);
    return ok;
}

static const char *base_name(const char *path) {
    const char *s = strrchr(path, '/');
    return s ? s + 1 : path;
}

/* ---- sfx/NAME.txt (and the old top-level sfx.txt) ----
 *   wave 3 0123456789abcdeffedcba9876543210     32 samples, one hex digit each
 *   sfx 0 speed 8 length 4 loop 0 0
 *     C-4 0 7 .  E-4 0 7 .  G-4 3 5 v  --- 0 0 .
 * A step is: note (C-4, D#5 ...), wave (0-f, or n for noise), volume 0-7,
 * effect (. none, s slide, v vibrato, d drop, < fade in, > fade out, a arpeggio). */
static const char *const NOTE_NAMES[12] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

void pitch_name(int pitch, char out[4]) {
    int m = pitch + 36;
    snprintf(out, 4, "%s%d", NOTE_NAMES[m % 12], m / 12 - 1);
}

static int parse_pitch(const char *t) {
    for (int i = 0; i < 12; i++)
        if (!strncasecmp(t, NOTE_NAMES[i], 2) && isdigit((unsigned char)t[2]) && !t[3]) {
            int p = (t[2] - '0' + 1) * 12 + i - 36;
            return p >= 0 && p < NPITCH ? p : -1;
        }
    return -1;
}

bool res_parse_sound(const char *path, SoundBank *bank, char *err, int errlen) {
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = base_name(path);
    char line[1024];
    int lineno = 0, cur = -1, nstep = 0;
    char *pending = NULL;
    bool ok = true, seen_sfx = false;
#define FAIL(fmt, ...) do { snprintf(err, (size_t)errlen, "%s:%d: " fmt, fn, lineno, __VA_ARGS__); ok = false; goto done; } while (0)
    while (fgets(line, sizeof line, f)) {
        lineno++;
        char *s = trim(line);
        if (!*s) {      /* a blank line ends the comments at the top of the file */
            if (pending && !seen_sfx) { bank->header = append_line(bank->header, trim(pending)); free(pending); pending = NULL; }
            continue;
        }
        if (*s == '#') { pending = append_line(pending, s); continue; }
        int n, a, b, c, d;
        char hex[64];
        if (sscanf(s, "wave %d %63s", &n, hex) == 2) {
            if (n < 0 || n >= NWAVES || strlen(hex) != WAVE_LEN) FAIL("wave N then %d hex digits", WAVE_LEN);
            for (int i = 0; i < WAVE_LEN; i++) {
                if (!isxdigit((unsigned char)hex[i])) FAIL("'%c' is not a hex digit", hex[i]);
                bank->waves[n][i] = (uint8_t)(isdigit((unsigned char)hex[i]) ? hex[i] - '0' : tolower((unsigned char)hex[i]) - 'a' + 10);
            }
            if (pending) { bank->header = append_line(bank->header, trim(pending)); free(pending); pending = NULL; }
            cur = -1;
        } else if (sscanf(s, "sfx %d speed %d length %d loop %d %d", &n, &a, &b, &c, &d) == 5) {
            if (n < 0 || n >= NSFX) FAIL("sfx %d out of range 0-%d", n, NSFX - 1);
            if (a < 1 || a > 64 || b < 1 || b > SFX_STEPS || c < 0 || d < 0 || c > SFX_STEPS || d > SFX_STEPS)
                FAIL("speed is 1-64, length 1-%d, loop 0-%d", SFX_STEPS, SFX_STEPS);
            Sfx *x = &bank->sfx[n];
            x->speed = (uint8_t)a; x->length = (uint8_t)b; x->loop_start = (uint8_t)c; x->loop_end = (uint8_t)d;
            free(bank->note[n]);
            bank->note[n] = pending;
            pending = NULL;
            cur = n;
            nstep = 0;
            seen_sfx = true;
        } else if (cur >= 0) {               /* a line of steps */
            char *save, *tok = strtok_r(s, " \t", &save);
            while (tok) {
                char *tw = strtok_r(NULL, " \t", &save), *tv = tw ? strtok_r(NULL, " \t", &save) : NULL;
                char *tf = tv ? strtok_r(NULL, " \t", &save) : NULL;
                if (!tf) FAIL("%s", "a step is note, wave, volume, effect");
                if (nstep >= SFX_STEPS) FAIL("more than %d steps", SFX_STEPS);
                SfxStep *st = &bank->sfx[cur].steps[nstep++];
                int p = strcmp(tok, "---") ? parse_pitch(tok) : 24;
                const char *fx = strlen(tf) == 1 ? strchr(fx_chars, tf[0]) : NULL;
                int wv = (tw[0] == 'n' || tw[0] == 'N') && !tw[1] ? WAVE_NOISE
                         : isxdigit((unsigned char)tw[0]) && !tw[1] ? (int)strtol(tw, NULL, 16) : -1;
                if (p < 0) FAIL("'%s' is not a note (C-2 .. D#7)", tok);
                if (wv < 0) FAIL("wave '%s' is not 0-f or n", tw);
                if (strlen(tv) != 1 || tv[0] < '0' || tv[0] > '7') FAIL("volume '%s' is not 0-7", tv);
                if (!fx || !tf[0]) FAIL("effect '%s' is not one of %s", tf, fx_chars);
                st->pitch = (uint8_t)p;
                st->wave = (uint8_t)wv;
                st->vol = (uint8_t)(tv[0] - '0');
                st->fx = (uint8_t)(fx - fx_chars);
                tok = strtok_r(NULL, " \t", &save);
            }
        } else FAIL("%s", "expected 'wave N ...' or 'sfx N speed S length L loop A B'");
    }
done:
#undef FAIL
    if (pending) {
        if (!bank->header) bank->header = pending;
        else free(pending);
    }
    fclose(f);
    return ok;
}

static bool sfx_is_default(const Sfx *s) {
    Sfx d;
    synth_default_sfx(&d);
    return !memcmp(&d, s, sizeof d);
}

bool res_write_sound(const char *path, const SoundBank *bank, char *err, int errlen) {
    char tmp[PATH_MAX + 32];
    FILE *f = open_for_save(path, tmp, sizeof tmp, err, errlen);
    if (!f) return false;
    if (bank->header) fputs(bank->header, f);
    else fprintf(f, "# %s sound: waves that differ from the defaults, then sound effects\n", cart.title);
    for (int w = 0; w < NWAVES; w++) {
        uint8_t d[WAVE_LEN];
        synth_default_wave(w, d);
        if (!memcmp(d, bank->waves[w], WAVE_LEN)) continue;
        fprintf(f, "wave %d ", w);
        for (int i = 0; i < WAVE_LEN; i++) fputc("0123456789abcdef"[bank->waves[w][i] & 15], f);
        fputc('\n', f);
    }
    for (int n = 0; n < NSFX; n++) {
        if (sfx_is_default(&bank->sfx[n])) continue;
        const Sfx *x = &bank->sfx[n];
        fputc('\n', f);
        if (bank->note[n]) fputs(bank->note[n], f);
        fprintf(f, "sfx %d speed %d length %d loop %d %d", n, x->speed, x->length, x->loop_start, x->loop_end);
        for (int k = 0; k < x->length; k++) {
            const SfxStep *st = &x->steps[k];
            char name[4];
            pitch_name(st->pitch, name);
            if (!st->vol && st->pitch == 24) strcpy(name, "---");     /* a plain rest */
            fprintf(f, "%s%s %c %d %c", k % 8 == 0 ? "\n  " : "  ", name,
                    st->wave >= NWAVES ? 'n' : "0123456789abcdef"[st->wave], st->vol, fx_chars[st->fx % NFX]);
        }
        fputc('\n', f);
    }
    return finish_save(f, tmp, path, err, errlen);
}

/* ---- music/NAME.txt (and the old top-level music.txt) ----
 *   song speed 8                 ticks per row
 *   order 0 1 0 2 >1             patterns to play; >N jumps to entry N, . stops
 *   pattern 0 tracks 2           then 32 rows, one cell per track:
 *     C-5 1 4 .  C-3 3 6 >       note wave volume effect (as in sfx.txt)
 *     ... . . .  === . . .       ... = nothing new (the note carries on), === = note off */
static bool parse_cell(char **save, Cell *c) {
    char *tn = strtok_r(NULL, " \t", save), *tw = tn ? strtok_r(NULL, " \t", save) : NULL;
    char *tv = tw ? strtok_r(NULL, " \t", save) : NULL, *tf = tv ? strtok_r(NULL, " \t", save) : NULL;
    if (!tf) return false;
    memset(c, 0, sizeof *c);
    c->pitch = 24;
    if (!strcmp(tn, "...")) { c->kind = CELL_EMPTY; return true; }
    if (!strcmp(tn, "===")) { c->kind = CELL_OFF; return true; }
    int p = parse_pitch(tn);
    int wv = (tw[0] == 'n' || tw[0] == 'N') && !tw[1] ? WAVE_NOISE : isxdigit((unsigned char)tw[0]) && !tw[1] ? (int)strtol(tw, NULL, 16) : -1;
    const char *fx = strlen(tf) == 1 && tf[0] ? strchr(fx_chars, tf[0]) : NULL;
    if (p < 0 || wv < 0 || strlen(tv) != 1 || tv[0] < '0' || tv[0] > '7' || !fx) return false;
    c->kind = CELL_NOTE;
    c->pitch = (uint8_t)p;
    c->wave = (uint8_t)wv;
    c->vol = (uint8_t)(tv[0] - '0');
    c->fx = (uint8_t)(fx - fx_chars);
    return true;
}

bool res_parse_music(const char *path, Song *song, char *err, int errlen) {
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, (size_t)errlen, "cannot read %s", path); return false; }
    const char *fn = base_name(path);
    char line[1024];
    int lineno = 0, cur = -1, row = 0, tracks = 0;
    char *pending = NULL;
    bool ok = true, seen = false;
#define FAIL(fmt, ...) do { snprintf(err, (size_t)errlen, "%s:%d: " fmt, fn, lineno, __VA_ARGS__); ok = false; goto done; } while (0)
    while (fgets(line, sizeof line, f)) {
        lineno++;
        char *s = trim(line);
        if (!*s) {
            if (pending && !seen) { song->header = append_line(song->header, trim(pending)); free(pending); pending = NULL; }
            continue;
        }
        if (*s == '#') { pending = append_line(pending, s); continue; }
        int n, t;
        if (cur >= 0) {                              /* a row of the pattern */
            char *save = s;                          /* parse_cell tokenises from here */
            for (int k = 0; k < tracks; k++)
                if (!parse_cell(&save, &song->patterns[cur].cells[row][k]))
                    FAIL("%s", "a cell is note wave volume effect, or ... . . . or === . . .");
            if (++row == PATTERN_ROWS) cur = -1;
        } else if (sscanf(s, "song speed %d", &n) == 1) {
            if (n < 1 || n > 64) FAIL("%s", "speed is 1-64");
            song->song_speed = (uint8_t)n;
            seen = true;
        } else if (!strncmp(s, "order", 5) && (!s[5] || isspace((unsigned char)s[5]))) {
            char *save, *tok = strtok_r(s + 5, " \t", &save);
            for (int i = 0; i < ORDER_LEN; i++) { song->order[i].kind = ORDER_STOP; song->order[i].value = 0; }
            for (int i = 0; tok; i++, tok = strtok_r(NULL, " \t", &save)) {
                if (i >= ORDER_LEN) FAIL("more than %d order entries", ORDER_LEN);
                OrderEntry *e = &song->order[i];
                char *end;
                if (!strcmp(tok, ".")) e->kind = ORDER_STOP;
                else if (tok[0] == '>' && (n = (int)strtol(tok + 1, &end, 10), end != tok + 1 && !*end) && n >= 0 && n < ORDER_LEN) {
                    e->kind = ORDER_JUMP;
                    e->value = (uint8_t)n;
                } else if ((n = (int)strtol(tok, &end, 10), end != tok && !*end) && n >= 0 && n < NPATTERNS) {
                    e->kind = ORDER_PATTERN;
                    e->value = (uint8_t)n;
                } else FAIL("'%s' is not a pattern (0-%d), >entry or .", tok, NPATTERNS - 1);
            }
            seen = true;
        } else if (sscanf(s, "pattern %d tracks %d", &n, &t) == 2) {
            if (n < 0 || n >= NPATTERNS) FAIL("pattern %d out of range 0-%d", n, NPATTERNS - 1);
            if (t < 1 || t > NTRACKS) FAIL("tracks is 1-%d", NTRACKS);
            free(song->note[n]);
            song->note[n] = pending;
            pending = NULL;
            cur = n;
            row = 0;
            tracks = t;
            seen = true;
        } else FAIL("%s", "expected 'song speed', 'order' or 'pattern N tracks T'");
    }
    if (cur >= 0) FAIL("pattern %d has %d of its %d rows", cur, row, PATTERN_ROWS);
done:
#undef FAIL
    if (pending) {
        if (!song->header) song->header = pending;
        else free(pending);
    }
    fclose(f);
    return ok;
}

static int pattern_tracks(const Song *song, int p) {      /* tracks in use: up to the last one with anything in it */
    int used = 0;
    for (int r = 0; r < PATTERN_ROWS; r++)
        for (int t = 0; t < NTRACKS; t++)
            if (song->patterns[p].cells[r][t].kind != CELL_EMPTY && t + 1 > used) used = t + 1;
    return used;
}

bool res_write_music(const char *path, const Song *song, char *err, int errlen) {
    char tmp[PATH_MAX + 32];
    FILE *f = open_for_save(path, tmp, sizeof tmp, err, errlen);
    if (!f) return false;
    if (song->header) fputs(song->header, f);
    else fprintf(f, "# %s music: the order to play patterns in, then the patterns\n", cart.title);
    fprintf(f, "\nsong speed %d\norder", song->song_speed);
    int last = ORDER_LEN - 1;
    while (last > 0 && song->order[last].kind == ORDER_STOP) last--;
    for (int i = 0; i <= last; i++) {
        const OrderEntry *e = &song->order[i];
        if (e->kind == ORDER_PATTERN) fprintf(f, " %d", e->value);
        else if (e->kind == ORDER_JUMP) fprintf(f, " >%d", e->value);
        else fprintf(f, " .");
    }
    fputc('\n', f);
    for (int p = 0; p < NPATTERNS; p++) {
        int tracks = pattern_tracks(song, p);
        if (!tracks) continue;
        fputc('\n', f);
        if (song->note[p]) fputs(song->note[p], f);
        fprintf(f, "pattern %d tracks %d\n", p, tracks);
        for (int r = 0; r < PATTERN_ROWS; r++) {
            for (int t = 0; t < tracks; t++) {
                const Cell *c = &song->patterns[p].cells[r][t];
                fputs("  ", f);
                if (c->kind == CELL_EMPTY) fputs("... . . .", f);
                else if (c->kind == CELL_OFF) fputs("=== . . .", f);
                else {
                    char name[4];
                    pitch_name(c->pitch, name);
                    fprintf(f, "%s %c %d %c", name, c->wave >= NWAVES ? 'n' : "0123456789abcdef"[c->wave], c->vol, fx_chars[c->fx % NFX]);
                }
            }
            fputc('\n', f);
        }
    }
    return finish_save(f, tmp, path, err, errlen);
}

bool cart_load(const char *arg, char *err, int errlen) {
    char dir[PATH_MAX];
    if (!resolve(arg, dir, sizeof dir)) {
        snprintf(err, (size_t)errlen, "no cart '%s' (a folder with main.gb)", arg);
        return false;
    }
    docs_reset();
    memset(&cart, 0, sizeof cart);
    snprintf(cart.dir, sizeof cart.dir, "%s", dir);
    const char *slash = strrchr(dir, '/');
    snprintf(cart.name, sizeof cart.name, "%s", slash ? slash + 1 : dir);
    snprintf(cart.title, sizeof cart.title, "%s", cart.name);
    load_meta();
    cart.has_label = label_read(cart.dir, cart.label);
    if (!docs_load_defaults(err, errlen)) { docs_reset(); return false; }
    cart.loaded = true;
    return true;
}

/* ---- compile and run ---- */

static char diag[4096];
static void collect_diag(const char *text) {
    size_t n = strlen(diag);
    snprintf(diag + n, sizeof diag - n, "%s", text);
}

char cart_err_file[256];
int cart_err_line;
char cart_err_msg[256];

/* "main.gb:12: runtime error: something" -> file, line and message */
static void parse_error(const char *s) {
    const char *c1 = strchr(s, ':');
    if (!c1) return;
    char *end;
    long line = strtol(c1 + 1, &end, 10);
    if (end == c1 + 1 || *end != ':') return;
    snprintf(cart_err_file, sizeof cart_err_file, "%.*s", (int)(c1 - s), s);
    cart_err_line = (int)line;
    const char *msg = end + 1;
    while (*msg == ' ') msg++;
    if (!strncmp(msg, "runtime error: ", 15)) msg += 15;
    else if (!strncmp(msg, "error: ", 7)) msg += 7;
    snprintf(cart_err_msg, sizeof cart_err_msg, "%s", msg);
}

static void report_diag(void) {
    /* drop the cart folder from file names: "/home/x/carts/foo/main.gb:3:" -> "main.gb:3:" */
    char *line = strtok(diag, "\n");
    size_t dl = strlen(cart.dir);
    bool first = true;
    while (line) {
        char out[1024], *o = out;
        for (const char *p = line; *p && o < out + sizeof out - 1;) {
            if (!strncmp(p, cart.dir, dl) && p[dl] == '/') { p += dl + 1; continue; }
            *o++ = *p++;
        }
        *o = 0;
        if (first) parse_error(out);
        first = false;
        shell_print(out, COL_RED);
        line = strtok(NULL, "\n");
    }
    diag[0] = 0;
}

bool cart_valid_name(const char *s) {
    if (!*s || strlen(s) > 64) return false;
    for (; *s; s++)
        if (!isalnum((unsigned char)*s) && *s != '_' && *s != '-') return false;
    return true;
}

/* writes a starter cart.txt and main.gb into dir */
static bool write_starter(const char *dir, const char *name, char *err, int errlen) {
    char path[PATH_MAX + 100];
    snprintf(path, sizeof path, "%s/cart.txt", dir);
    FILE *f = fopen(path, "w");
    if (f) { fprintf(f, "title = %s\nauthor =\nversion = 1\n", name); fclose(f); }
    snprintf(path, sizeof path, "%s/main.gb", dir);
    f = fopen(path, "w");
    if (!f) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    fprintf(f,
            "' %s - a new GooberEngine cart\n"
            "strict\n"
            "\n"
            "local x:float = SCREEN_W / 2, y:float = SCREEN_H / 2\n"
            "\n"
            "repeat\n"
            "  if btn(BTN_LEFT) then x :- 2\n"
            "  if btn(BTN_RIGHT) then x :+ 2\n"
            "  if btn(BTN_UP) then y :- 2\n"
            "  if btn(BTN_DOWN) then y :+ 2\n"
            "\n"
            "  cls(COL_NAVY)\n"
            "  text(\"hello from %s\", 8, 8, COL_WHITE)\n"
            "  circle_fill(x, y, 8, COL_YELLOW)\n"
            "  flip()\n"
            "forever\n",
            name, name);
    fclose(f);
    return true;
}

bool cart_create(const char *name, char *err, int errlen) {
    if (!cart_valid_name(name)) {
        snprintf(err, (size_t)errlen, "a cart name uses letters, digits, - and _ only");
        return false;
    }
    char dir[PATH_MAX + 80];
    snprintf(dir, sizeof dir, "%s/%s", carts_root(), name);
    if (is_dir(dir)) { snprintf(err, (size_t)errlen, "cart '%s' already exists (LOAD it)", name); return false; }
    if (mkdir(dir, 0755) != 0) { snprintf(err, (size_t)errlen, "could not create %s", dir); return false; }
    if (!write_starter(dir, name, err, errlen)) return false;
    return cart_load(dir, err, errlen);
}

/* ---- the untitled cart ----
 * GooberEngine boots into a cart with no name. It lives in a scratch folder
 * outside the carts folder (running needs main.gb on disk), nothing in it
 * counts as saved, and SAVE name copies it into the carts folder. */

static char scratch[PATH_MAX];

static void remove_dir(const char *dir) {      /* the scratch folder holds plain files only */
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.' && (!e->d_name[1] || (e->d_name[1] == '.' && !e->d_name[2]))) continue;
        char p[PATH_MAX + 260];
        snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
        remove(p);
    }
    closedir(d);
    rmdir(dir);
}

void cart_discard_scratch(void) {
    if (!scratch[0]) return;
    remove_dir(scratch);
    scratch[0] = 0;
}

const char *cart_scratch_dir(void) { return scratch; }

bool cart_new_untitled(char *err, int errlen) {
    cart_discard_scratch();
    const char *tmp = getenv("TMPDIR");
    snprintf(scratch, sizeof scratch, "%s/gooberengine-untitled-%ld", tmp && *tmp ? tmp : "/tmp", (long)getpid());
    remove_dir(scratch);                       /* a leftover from an earlier run with the same pid */
    if (mkdir(scratch, 0700) != 0) {
        snprintf(err, (size_t)errlen, "could not create %s", scratch);
        scratch[0] = 0;
        return false;
    }
    if (!write_starter(scratch, "untitled", err, errlen) || !cart_load(scratch, err, errlen)) return false;
    snprintf(cart.name, sizeof cart.name, "untitled");
    cart.untitled = true;
    return true;
}

static bool copy_file(const char *from, const char *to) {
    FILE *in = fopen(from, "rb"), *out = in ? fopen(to, "wb") : NULL;
    bool ok = in && out;
    char buf[8192];
    size_t n;
    while (ok && (n = fread(buf, 1, sizeof buf, in)) > 0) ok = fwrite(buf, 1, n, out) == n;
    if (in) fclose(in);
    if (out && fclose(out) != 0) ok = false;
    return ok;
}

/* SAVE name on the untitled cart: give it a folder in the carts folder. The
 * caller then saves the edits into it (the scratch copy may be behind). */
bool cart_exists(const char *name) {
    char dir[PATH_MAX + 80];
    snprintf(dir, sizeof dir, "%s/%s", carts_root(), name);
    return cart_valid_name(name) && is_dir(dir);
}

static void remove_tree(const char *dir, int depth) {   /* a cart folder and its resource subfolders */
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.' && (!e->d_name[1] || (e->d_name[1] == '.' && !e->d_name[2]))) continue;
        char p[PATH_MAX + 260];
        snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
        if (depth > 0 && is_dir(p)) remove_tree(p, depth - 1);
        else remove(p);
    }
    closedir(d);
    rmdir(dir);
}

bool cart_save_as(const char *name, bool overwrite, char *err, int errlen) {
    if (!cart.untitled) { snprintf(err, (size_t)errlen, "this cart already has a name"); return false; }
    if (!cart_valid_name(name)) {
        snprintf(err, (size_t)errlen, "a cart name uses letters, digits, - and _ only");
        return false;
    }
    char dir[PATH_MAX + 80];
    snprintf(dir, sizeof dir, "%s/%s", carts_root(), name);
    if (is_dir(dir)) {
        if (!overwrite) { snprintf(err, (size_t)errlen, "cart '%s' already exists: pick another name", name); return false; }
        remove_tree(dir, 2);                    /* the old cart goes, so none of its files linger in the new one */
        if (is_dir(dir)) { snprintf(err, (size_t)errlen, "could not remove the old %s", dir); return false; }
    }
    if (mkdir(dir, 0755) != 0) { snprintf(err, (size_t)errlen, "could not create %s", dir); return false; }
    DIR *d = opendir(cart.dir);
    struct dirent *e;
    while (d && (e = readdir(d))) {
        char from[PATH_MAX + 260], to[PATH_MAX + 340];
        snprintf(from, sizeof from, "%s/%s", cart.dir, e->d_name);
        if (e->d_name[0] == '.' || !is_file(from)) continue;
        if (!strcmp(e->d_name, "cart.txt")) continue;          /* written fresh below, with the new title */
        snprintf(to, sizeof to, "%s/%s", dir, e->d_name);
        if (!copy_file(from, to)) {
            closedir(d);
            snprintf(err, (size_t)errlen, "could not write %s", to);
            return false;
        }
    }
    if (d) closedir(d);
    if (!strcmp(cart.title, "untitled")) snprintf(cart.title, sizeof cart.title, "%s", name);
    char path[PATH_MAX + 100];
    snprintf(path, sizeof path, "%s/cart.txt", dir);
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "title = %s\nauthor = %s\nversion = %s\n", cart.title, cart.author, cart.version[0] ? cart.version : "1");
        fclose(f);
    }
    char *real = realpath(dir, NULL);
    snprintf(cart.dir, sizeof cart.dir, "%s", real ? real : dir);
    free(real);
    snprintf(cart.name, sizeof cart.name, "%s", name);
    cart.untitled = false;
    cart_discard_scratch();
    return true;
}

static jmp_buf compile_jmp;

static bool compile_cart(Program *prog) {
    static TokenList toks;
    memset(&toks, 0, sizeof toks);
    memset(prog, 0, sizeof *prog);
    char path[PATH_MAX + 16];
    snprintf(path, sizeof path, "%s/main.gb", cart.dir);
    gb_error_jmp = &compile_jmp;
    if (setjmp(compile_jmp)) {
        gb_error_jmp = NULL;
        return false;
    }
    lex_file(path, &toks, prog);
    compile_program(&toks, prog);
    gb_error_jmp = NULL;
    return true;
}

int cart_check(const char *arg) {
    char err[512];
    if (!cart_load(arg, err, sizeof err)) { fprintf(stderr, "%s\n", err); return 1; }
    static Program prog;
    if (!compile_cart(&prog)) return 1;    /* errors already went to stderr */
    printf("%s: OK\n", cart.name);
    return 0;
}

bool cart_flipped;      /* set by flip(): the cart drew at least one frame */
bool cart_stopped;      /* Esc or window close */

int cart_run(void) {
    if (!cart.loaded) { shell_print("no cart loaded: try LOAD name", COL_RED); return 1; }
    static Program prog;    /* static: setjmp/longjmp inside */
    diag[0] = 0;
    cart_err_file[0] = 0;
    cart_err_line = 0;
    cart_err_msg[0] = 0;
    gb_diag_hook = collect_diag;
    if (!compile_cart(&prog)) {
        report_diag();
        gb_diag_hook = NULL;
        return 1;
    }
    static VM vm;
    vm_init(&vm, &prog);
    int vx = view_x, vw = view_w;      /* the cart has the whole screen, file tree or not */
    view_x = 0;
    view_w = SCREEN_W;
    draw_reset();
    draw_cls(0);
    cart_flipped = cart_stopped = label_taken = false;
    /* the cart runs on copies of its files in slots (slot 0: the default ones),
     * so what it changes (map_set, tile_pset) lasts only for the run and the
     * editors never save what the game did */
    rt_begin();
    dot_limit = DOT_LIMIT_DEFAULT;
    dots_used = 0;
    ds.count_dots = true;
    in_cart = true;
    int code = vm_run(&vm);
    in_cart = false;
    ds.count_dots = false;
    audio_lock();
    synth_stop(-1);             /* a cart's sounds end with it */
    synth_music_stop();
    audio_unlock();
    rt_end();
    vm_free(&vm);
    gb_diag_hook = NULL;
    view_x = vx;
    view_w = vw;
    if (code != 0 && !cart_stopped) { report_diag(); return 1; }
    diag[0] = 0;
    if (cart_stopped) return 2;
    /* a cart that drew once and ended: keep its picture up until a key is pressed */
    if (!cart_flipped && !in.quit) {
        view_x = 0;
        view_w = SCREEN_W;
        draw_reset();
        draw_rect(0, SCREEN_H - FONT_H, SCREEN_W, FONT_H, COL_BLACK, true);
        draw_text("cart ended - press any key", 2, SCREEN_H - FONT_H, COL_GREY);
        for (;;) {
            platform_frame();
            if (in.quit || opt.headless || in.escape || in.text[0] || in.key_hit[SDL_SCANCODE_RETURN]) break;
        }
        view_x = vx;
        view_w = vw;
    }
    return 0;
}
