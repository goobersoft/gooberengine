/* Cart labels and one-file carts.
 *
 * label.txt is the cart's picture for the BROWSE screen: the screen at a
 * quarter size (100 x 60), one character per pixel in the default tiles.txt
 * alphabet. Press F8 while a cart runs to make one.
 *
 * A .gbcart file is a whole cart folder in one text file, for sharing:
 *
 *   GOOBERCART 1
 *   file cart.txt
 *   |title = Seedling            every line of a file, behind a |
 *   file main.gb
 *   |...
 *   end
 *
 * Only the cart's own text files are bundled: .gb and .txt at the top, and
 * the files in tiles/, maps/ (.g64), sfx/ and music/ (.txt), named with their
 * folder ("file tiles/tileset.g64"). Not *.unsaved or *.old. Save data is
 * not bundled either: it belongs to the player, not the cart. */
#include "engine.h"
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool label_taken;

static int pixel_index(char c) {
    const char *p = c ? strchr(ge_pixel_chars, c) : NULL;
    return p ? (int)(p - ge_pixel_chars) : -1;
}

bool label_read(const char *dir, uint8_t out[LABEL_H][LABEL_W]) {
    char path[PATH_MAX + 16];
    snprintf(path, sizeof path, "%s/label.txt", dir);
    FILE *f = fopen(path, "r");
    if (!f) return false;
    char line[256];
    int y = 0;
    memset(out, 0, LABEL_H * LABEL_W);
    while (y < LABEL_H && fgets(line, sizeof line, f)) {
        if (line[0] == '#') continue;
        for (int x = 0; x < LABEL_W; x++) {
            int c = x < (int)strlen(line) ? pixel_index(line[x]) : -1;
            out[y][x] = (uint8_t)(c < 0 ? 0 : c);
        }
        y++;
    }
    fclose(f);
    return y > 0;
}

bool label_capture(char *err, int errlen) {
    char path[PATH_MAX + 16];
    snprintf(path, sizeof path, "%s/label.txt", cart.dir);
    FILE *f = fopen(path, "w");
    if (!f) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    fprintf(f, "# %s label: the screen at a quarter size, one character per pixel (made with F8)\n", cart.title);
    for (int y = 0; y < LABEL_H; y++) {
        for (int x = 0; x < LABEL_W; x++) {
            uint8_t c = fb[y * 4 + 1][x * 4 + 1];
            cart.label[y][x] = c;
            fputc(ge_pixel_chars[c & 63], f);
        }
        fputc('\n', f);
    }
    cart.has_label = true;
    if (fclose(f) != 0) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    return true;
}

/* ---- .gbcart ---- */

static bool plain_name(const char *n, size_t len) {
    if (!len || len > 64 || n[0] == '.') return false;
    for (size_t i = 0; i < len; i++)
        if (!isalnum((unsigned char)n[i]) && n[i] != '_' && n[i] != '-' && n[i] != '.') return false;
    return true;
}

static bool has_ext(const char *n, const char *ext) {
    size_t a = strlen(n), b = strlen(ext);
    return a > b && !strcmp(n + a - b, ext);
}

/* "main.gb", "cart.txt", "tiles/tileset.g64", "sfx/sfx.txt" ... */
static bool bundled_name(const char *n) {
    const char *slash = strchr(n, '/');
    if (!slash) return plain_name(n, strlen(n)) && (has_ext(n, ".gb") || has_ext(n, ".txt"));
    if (strchr(slash + 1, '/') || !plain_name(slash + 1, strlen(slash + 1))) return false;
    for (int k = 0; k < NRES; k++)
        if ((size_t)(slash - n) == strlen(res_dir[k]) && !strncmp(n, res_dir[k], (size_t)(slash - n)))
            return has_ext(slash + 1, res_ext[k]);
    return false;
}

static int cmp_names(const void *a, const void *b) {
    const char *x = *(char *const *)a, *y = *(char *const *)b;
    /* cart.txt first, then main.gb, then the rest in order: a stable, readable file */
    int rx = !strcmp(x, "cart.txt") ? 0 : !strcmp(x, "main.gb") ? 1 : 2;
    int ry = !strcmp(y, "cart.txt") ? 0 : !strcmp(y, "main.gb") ? 1 : 2;
    return rx != ry ? rx - ry : strcmp(x, y);
}

/* the cart folder's text files as one .gbcart text, malloc'd */
typedef struct { char *s; size_t n, cap; } Buf;
static void buf_put(Buf *b, const char *s, size_t n) {
    if (b->n + n + 1 > b->cap) {
        b->cap = (b->n + n + 1) * 2;
        b->s = realloc(b->s, b->cap);
    }
    memcpy(b->s + b->n, s, n);
    b->n += n;
    b->s[b->n] = 0;
}
static void buf_puts(Buf *b, const char *s) { buf_put(b, s, strlen(s)); }

char *cart_pack(size_t *len, char *err, int errlen) {
    if (!cart.loaded) { snprintf(err, (size_t)errlen, "no cart loaded"); return NULL; }
    DIR *d = opendir(cart.dir);
    if (!d) { snprintf(err, (size_t)errlen, "cannot read %s", cart.dir); return NULL; }
    char *names[512];
    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < 512)
        if (bundled_name(e->d_name)) names[n++] = strdup(e->d_name);
    closedir(d);
    for (int k = 0; k < NRES; k++) {           /* the resource folders */
        char sub[PATH_MAX + 64];
        snprintf(sub, sizeof sub, "%s/%s", cart.dir, res_dir[k]);
        DIR *sd = opendir(sub);
        while (sd && (e = readdir(sd)) && n < 512) {
            char rel[160];
            snprintf(rel, sizeof rel, "%s/%s", res_dir[k], e->d_name);
            if (bundled_name(rel)) names[n++] = strdup(rel);
        }
        if (sd) closedir(sd);
    }
    qsort(names, (size_t)n, sizeof names[0], cmp_names);
    Buf b = {0};
    buf_puts(&b, "GOOBERCART 1\n");
    for (int i = 0; i < n; i++) {
        char p[PATH_MAX + 80];
        snprintf(p, sizeof p, "%s/%s", cart.dir, names[i]);
        FILE *in = fopen(p, "rb");
        if (!in) continue;
        buf_puts(&b, "file ");
        buf_puts(&b, names[i]);
        buf_puts(&b, "\n");
        char line[4096];
        bool at_start = true;
        while (fgets(line, sizeof line, in)) {
            size_t ln = strlen(line);
            if (ln > 1 && line[ln - 2] == '\r' && line[ln - 1] == '\n') { line[ln - 2] = '\n'; line[--ln] = 0; }
            if (at_start) buf_puts(&b, "|");
            buf_put(&b, line, ln);
            at_start = ln && line[ln - 1] == '\n';
        }
        if (!at_start) buf_puts(&b, "\n");
        fclose(in);
    }
    buf_puts(&b, "end\n");
    for (int i = 0; i < n; i++) free(names[i]);
    *len = b.n;
    return b.s;
}

bool cart_export(const char *path, char *out_path, int outlen, char *err, int errlen) {
    char def[PATH_MAX + 160];
    if (!path || !*path) {
        snprintf(def, sizeof def, "%s/%s.gbcart", carts_root(), cart.name);
        path = def;
    }
    size_t len;
    char *pack = cart_pack(&len, err, errlen);
    if (!pack) return false;
    char tmp[PATH_MAX + 200];
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    FILE *out = fopen(tmp, "wb");
    bool ok = out && fwrite(pack, 1, len, out) == len;
    if (out) ok = fclose(out) == 0 && ok && rename(tmp, path) == 0;
    free(pack);
    if (!ok) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    snprintf(out_path, (size_t)outlen, "%s", path);
    return true;
}

bool cart_import(const char *file, const char *name, char *out_dir, int outlen, char *err, int errlen) {
    FILE *in = fopen(file, "rb");     /* binary both ways: the cart's files keep \n line ends everywhere */
    if (!in) { snprintf(err, (size_t)errlen, "cannot open %s", file); return false; }
    char line[4096];
    if (!fgets(line, sizeof line, in) || strncmp(line, "GOOBERCART 1", 12)) {
        fclose(in);
        snprintf(err, (size_t)errlen, "%s is not a .gbcart file", file);
        return false;
    }
    char base[128];
    if (!name || !*name) {      /* the cart takes the file's name: games/rocket.gbcart -> rocket */
        const char *slash = strrchr(file, '/');
        snprintf(base, sizeof base, "%s", slash ? slash + 1 : file);
        char *dot = strstr(base, ".gbcart");
        if (dot) *dot = 0;
        name = base;
    }
    for (const char *p = name; *p; p++)
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-') {
            fclose(in);
            snprintf(err, (size_t)errlen, "a cart name uses letters, digits, - and _ only (IMPORT file name)");
            return false;
        }
    char dir[PATH_MAX + 160];
    snprintf(dir, sizeof dir, "%s/%s", carts_root(), name);
    struct stat st;
    if (stat(dir, &st) == 0) {
        fclose(in);
        snprintf(err, (size_t)errlen, "cart '%s' already exists: IMPORT %s othername", name, file);
        return false;
    }
    if (mkdir(dir, 0755) != 0) { fclose(in); snprintf(err, (size_t)errlen, "could not create %s", dir); return false; }
    FILE *out = NULL;
    bool ok = true, ended = false, has_main = false, mid = false;
    int lineno = 1;
    while (fgets(line, sizeof line, in)) {
        size_t got = strlen(line);
        bool was_mid = mid;
        mid = got && line[got - 1] != '\n';        /* a line longer than the buffer: the rest follows */
        if (was_mid) { if (out) fputs(line, out); continue; }
        lineno++;
        if (line[0] == '|') {
            if (!out) { ok = false; snprintf(err, (size_t)errlen, "%s:%d: a | line before any 'file' line", file, lineno); break; }
            size_t ln = strlen(line);
            if (ln > 1 && line[ln - 2] == '\r') { line[ln - 2] = '\n'; line[ln - 1] = 0; }
            fputs(line + 1, out);
            continue;
        }
        line[strcspn(line, "\r\n")] = 0;
        if (out) { fclose(out); out = NULL; }
        if (!strcmp(line, "end")) { ended = true; break; }
        if (strncmp(line, "file ", 5) || !bundled_name(line + 5)) {
            ok = false;
            snprintf(err, (size_t)errlen, "%s:%d: expected 'file NAME' (a .gb, .txt or .g64 name) or 'end'", file, lineno);
            break;
        }
        if (!strcmp(line + 5, "main.gb")) has_main = true;
        char p[PATH_MAX + 240];
        const char *slash = strchr(line + 5, '/');
        if (slash) {                             /* tiles/... : make the folder first */
            snprintf(p, sizeof p, "%s/%.*s", dir, (int)(slash - (line + 5)), line + 5);
            mkdir(p, 0755);
        }
        snprintf(p, sizeof p, "%s/%s", dir, line + 5);
        out = fopen(p, "wb");
        if (!out) { ok = false; snprintf(err, (size_t)errlen, "could not write %s", p); break; }
    }
    if (out) fclose(out);
    fclose(in);
    if (ok && !ended) { ok = false; snprintf(err, (size_t)errlen, "%s is cut short (no 'end' line)", file); }
    if (ok && !has_main) { ok = false; snprintf(err, (size_t)errlen, "%s has no main.gb", file); }
    if (!ok) {   /* don't leave half a cart behind */
        for (int k = 0; k < NRES; k++) {
            char sub[PATH_MAX + 240];
            snprintf(sub, sizeof sub, "%s/%s", dir, res_dir[k]);
            DIR *sd = opendir(sub);
            struct dirent *se;
            while (sd && (se = readdir(sd))) {
                if (se->d_name[0] == '.') continue;
                char p[PATH_MAX + 500];
                snprintf(p, sizeof p, "%s/%s", sub, se->d_name);
                remove(p);
            }
            if (sd) closedir(sd);
            rmdir(sub);
        }
        DIR *d = opendir(dir);
        struct dirent *e;
        while (d && (e = readdir(d))) {
            if (e->d_name[0] == '.') continue;
            char p[PATH_MAX + 240];
            snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
            remove(p);
        }
        if (d) closedir(d);
        rmdir(dir);
        return false;
    }
    snprintf(out_dir, (size_t)outlen, "%s", dir);
    return true;
}
