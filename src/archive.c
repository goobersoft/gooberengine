/* Zipped carts: a cart folder in one .zip, .tar.gz (.tgz) or plain .tar file.
 *
 * An archive is for carrying a cart around. LOAD unpacks it into a cart folder
 * beside it in the carts folder (rocket.zip -> rocket/), and from then on that
 * folder is the cart: SAVE writes the folder, not the archive. LOAD on an
 * archive whose folder is already there just loads the folder.
 *
 * The cart may sit at the top of the archive (main.gb, tiles/tileset.g64 ...) or
 * in one folder inside it (rocket/main.gb ...), which is what zipping a folder
 * from a file manager gives. Only the cart's own files come out (see
 * bundled_name below): no other paths, so nothing outside the cart folder can
 * be written.
 *
 * EXPORT zips the cart folder the same way: its own files, inside a folder of
 * the cart's name (rocket.zip holds rocket/main.gb ...). Exported games carry the
 * same zip bytes (see export.c).
 *
 * DIR and BROWSE show an archive's title, author and version without unpacking
 * it. They read its cart.txt once and keep it in a cache, cartcache.txt beside the
 * save-data folder (or $GOOBERENGINE_CACHE), one line per archive:
 *
 *   /path/to/rocket.zip <TAB> size <TAB> mtime <TAB> 1 <TAB> Rocket <TAB> goober <TAB> 1.0
 *
 * A line is used while the archive's size and time are unchanged; the 1 says it
 * has a main.gb. Lines for archives that are gone are dropped on the next write. */
#include "engine.h"
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_ARCHIVE (64u << 20)    /* the archive file */
#define MAX_MEMBER (16u << 20)     /* one file in it, unpacked */
#define MAX_MEMBERS 1024

static const char *const exts[] = {".tar.gz", ".tgz", ".zip", ".tar"};
#define NEXTS (int)(sizeof exts / sizeof exts[0])

static size_t ext_len(const char *file) {
    size_t n = strlen(file);
    for (int i = 0; i < NEXTS; i++) {
        size_t e = strlen(exts[i]);
        if (n > e && !strcasecmp(file + n - e, exts[i])) return e;
    }
    return 0;
}

bool archive_name(const char *file) { return ext_len(file) > 0; }

static bool read_all(const char *path, uint8_t **data, size_t *len, char *err, int errlen) {
    FILE *f = fopen(path, "rb");
    if (!f) { snprintf(err, (size_t)errlen, "cannot open %s", path); return false; }
    size_t cap = 65536, n = 0;
    uint8_t *b = malloc(cap);
    size_t got;
    while ((got = fread(b + n, 1, cap - n, f)) > 0) {
        n += got;
        if (n > MAX_ARCHIVE) { free(b); fclose(f); snprintf(err, (size_t)errlen, "%s is too big for a cart", path); return false; }
        if (n == cap) b = realloc(b, cap *= 2);
    }
    fclose(f);
    *data = b;
    *len = n;
    return true;
}

/* ---- the cart's own files ---- */

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

/* a cart's own files: .gb and .txt at the top, and the files in tiles/, maps/,
 * sfx/, music/ and fonts/ (.g64, or an old .txt sound or song). Not *.unsaved or
 * *.old leftovers, and not save data, which belongs to the player, not the cart.
 * "main.gb", "cart.txt", "tiles/tileset.g64", "sfx/sfx.g64" ... */
static bool bundled_name(const char *n) {
    const char *slash = strchr(n, '/');
    if (!slash) return plain_name(n, strlen(n)) && (has_ext(n, ".gb") || has_ext(n, ".txt"));
    if (strchr(slash + 1, '/') || !plain_name(slash + 1, strlen(slash + 1))) return false;
    for (int k = 0; k < NRES; k++)
        if ((size_t)(slash - n) == strlen(res_dir[k]) && !strncmp(n, res_dir[k], (size_t)(slash - n)))
            return has_ext(slash + 1, res_ext[k]) || ((k == RES_SFX || k == RES_MUSIC) && has_ext(slash + 1, ".txt"));
    return false;
}

static int cmp_names(const void *a, const void *b) {
    const char *x = *(char *const *)a, *y = *(char *const *)b;
    /* cart.txt first, then main.gb, then the rest in order: a stable, readable file */
    int rx = !strcmp(x, "cart.txt") ? 0 : !strcmp(x, "main.gb") ? 1 : 2;
    int ry = !strcmp(y, "cart.txt") ? 0 : !strcmp(y, "main.gb") ? 1 : 2;
    return rx != ry ? rx - ry : strcmp(x, y);
}

bool cart_bundled_name(const char *n) { return bundled_name(n); }

/* the names of a cart folder's own files, sorted (cart.txt, main.gb, the rest); strdup'd */
int cart_file_list(const char *dir, char **names, int max) {
    DIR *d = opendir(dir);
    if (!d) return -1;
    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < max)
        if (bundled_name(e->d_name)) names[n++] = strdup(e->d_name);
    closedir(d);
    for (int k = 0; k < NRES; k++) {           /* the resource folders */
        char sub[PATH_MAX + 64];
        snprintf(sub, sizeof sub, "%s/%s", dir, res_dir[k]);
        DIR *sd = opendir(sub);
        while (sd && (e = readdir(sd)) && n < max) {
            char rel[160];
            snprintf(rel, sizeof rel, "%s/%s", res_dir[k], e->d_name);
            if (bundled_name(rel)) names[n++] = strdup(rel);
        }
        if (sd) closedir(sd);
    }
    qsort(names, (size_t)n, sizeof names[0], cmp_names);
    return n;
}

/* ---- reading: every file in the archive, in memory ---- */

typedef struct { char name[256]; uint8_t *data; size_t len; } Member;
typedef struct { Member *m; int n; } Members;

static void members_free(Members *ms) {
    for (int i = 0; i < ms->n; i++) free(ms->m[i].data);
    free(ms->m);
    ms->m = NULL;
    ms->n = 0;
}

static void add(Members *ms, const char *name, size_t nlen, uint8_t *data, size_t len) {
    if (ms->n == MAX_MEMBERS) { free(data); return; }
    if (!ms->m) ms->m = calloc(MAX_MEMBERS, sizeof *ms->m);
    Member *m = &ms->m[ms->n++];
    if (nlen >= sizeof m->name) nlen = sizeof m->name - 1;
    memcpy(m->name, name, nlen);
    m->name[nlen] = 0;
    for (char *p = m->name; *p; p++) if (*p == '\\') *p = '/';     /* zips made on Windows */
    m->data = data;
    m->len = len;
}

static unsigned rd16(const uint8_t *p) { return p[0] | p[1] << 8; }
static uint32_t rd32(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

static bool read_zip(const uint8_t *f, size_t n, Members *ms, char *err, int errlen) {
    /* the end record: the last 22 bytes, unless a comment follows it */
    size_t eocd = SIZE_MAX;
    for (size_t i = n >= 22 ? n - 22 : 0, stop = n > 22 + 65535 ? n - 22 - 65535 : 0; n >= 22; i--) {
        if (rd32(f + i) == 0x06054b50) { eocd = i; break; }
        if (i == stop) break;
    }
    if (eocd == SIZE_MAX) { snprintf(err, (size_t)errlen, "not a zip file (no central directory)"); return false; }
    unsigned count = rd16(f + eocd + 10);
    size_t p = rd32(f + eocd + 16);
    for (unsigned k = 0; k < count; k++) {
        if (p + 46 > n || rd32(f + p) != 0x02014b50) { snprintf(err, (size_t)errlen, "the zip's directory is damaged"); return false; }
        unsigned flags = rd16(f + p + 8), method = rd16(f + p + 10);
        uint32_t crc = rd32(f + p + 16), csize = rd32(f + p + 20), usize = rd32(f + p + 24);
        unsigned nlen = rd16(f + p + 28), xlen = rd16(f + p + 30), clen = rd16(f + p + 32);
        size_t local = rd32(f + p + 42);
        const char *name = (const char *)f + p + 46;
        if (p + 46 + nlen > n) { snprintf(err, (size_t)errlen, "the zip's directory is damaged"); return false; }
        p += 46 + nlen + xlen + clen;
        if (nlen && name[nlen - 1] == '/') continue;                   /* a folder */
        if (flags & 1) { snprintf(err, (size_t)errlen, "the zip is encrypted"); return false; }
        if (csize == 0xffffffff || usize == 0xffffffff || usize > MAX_MEMBER) {
            snprintf(err, (size_t)errlen, "%.*s is too big for a cart", (int)nlen, name);
            return false;
        }
        if (local + 30 > n || rd32(f + local) != 0x04034b50) { snprintf(err, (size_t)errlen, "the zip is damaged"); return false; }
        size_t data = local + 30 + rd16(f + local + 26) + rd16(f + local + 28);
        if (data + csize > n) { snprintf(err, (size_t)errlen, "the zip is cut short"); return false; }
        uint8_t *out;
        size_t olen;
        if (method == 0) {
            out = malloc(csize ? csize : 1);
            memcpy(out, f + data, csize);
            olen = csize;
        } else if (method == 8) {
            const char *why = "";
            out = ge_inflate(f + data, csize, MAX_MEMBER, &olen, NULL, &why);
            if (!out) { snprintf(err, (size_t)errlen, "%.*s: %s", (int)nlen, name, why); return false; }
        } else {
            snprintf(err, (size_t)errlen, "%.*s uses a compression GooberEngine can't read (method %u; use deflate)", (int)nlen, name, method);
            return false;
        }
        if (olen != usize || ge_crc32(out, olen) != crc) {
            free(out);
            snprintf(err, (size_t)errlen, "%.*s is damaged (CRC)", (int)nlen, name);
            return false;
        }
        add(ms, name, nlen, out, olen);
    }
    return true;
}

static size_t octal(const uint8_t *p, int n) {
    size_t v = 0;
    for (int i = 0; i < n && p[i]; i++)
        if (p[i] >= '0' && p[i] <= '7') v = v * 8 + (size_t)(p[i] - '0');
    return v;
}

static bool read_tar(const uint8_t *f, size_t n, Members *ms, char *err, int errlen) {
    size_t p = 0;
    while (p + 512 <= n) {
        const uint8_t *h = f + p;
        if (!h[0]) break;                                   /* the zero blocks at the end */
        unsigned sum = 0;
        for (int i = 0; i < 512; i++) sum += (i >= 148 && i < 156) ? ' ' : h[i];
        if (sum != octal(h + 148, 8)) { snprintf(err, (size_t)errlen, "not a tar file (bad header)"); return false; }
        size_t size = octal(h + 124, 12);
        char type = (char)h[156];
        p += 512;
        if (p + size > n) { snprintf(err, (size_t)errlen, "the tar is cut short"); return false; }
        if (type == '0' || type == 0) {
            if (size > MAX_MEMBER) { snprintf(err, (size_t)errlen, "%.100s is too big for a cart", (const char *)h); return false; }
            char name[256];
            size_t nl = strnlen((const char *)h, 100);
            if (!memcmp(h + 257, "ustar", 5) && h[345])        /* a long name: prefix/name */
                snprintf(name, sizeof name, "%.*s/%.*s", (int)strnlen((const char *)h + 345, 155), (const char *)h + 345, (int)nl, (const char *)h);
            else snprintf(name, sizeof name, "%.*s", (int)nl, (const char *)h);
            uint8_t *data = malloc(size ? size : 1);
            memcpy(data, f + p, size);
            add(ms, name, strlen(name), data, size);
        }
        p += (size + 511) / 512 * 512;
    }
    return true;
}

static bool read_archive(const char *file, Members *ms, char *err, int errlen) {
    uint8_t *f;
    size_t n;
    if (!read_all(file, &f, &n, err, errlen)) return false;
    bool ok;
    char why[300] = "";
    if (n >= 4 && rd32(f) == 0x04034b50) ok = read_zip(f, n, ms, why, sizeof why);
    else if (n >= 4 && rd32(f) == 0x06054b50) { ok = false; snprintf(why, sizeof why, "the zip is empty"); }
    else if (n >= 2 && f[0] == 0x1f && f[1] == 0x8b) {
        const char *e = "";
        size_t tn;
        uint8_t *t = ge_gunzip(f, n, MAX_ARCHIVE, &tn, &e);
        if (t) { ok = read_tar(t, tn, ms, why, sizeof why); free(t); }
        else { ok = false; snprintf(why, sizeof why, "%s", e); }
    } else ok = read_tar(f, n, ms, why, sizeof why);
    free(f);
    if (!ok) { members_free(ms); snprintf(err, (size_t)errlen, "%s: %s", file, why); }
    return ok;
}

/* where the cart is in the archive: "" or "folder/", the shallowest main.gb */
static bool cart_prefix(const Members *ms, char *prefix, size_t n) {
    int best = -1;
    for (int i = 0; i < ms->n; i++) {
        const char *nm = ms->m[i].name, *slash = strchr(nm, '/');
        if (!strcmp(nm, "main.gb")) { best = i; break; }
        if (slash && !strcmp(slash, "/main.gb") && slash > nm && best < 0) best = i;
    }
    if (best < 0) return false;
    snprintf(prefix, n, "%.*s", (int)(strlen(ms->m[best].name) - 7), ms->m[best].name);
    return true;
}

/* the cart's file `rel` in the archive, or NULL */
static const Member *member(const Members *ms, const char *prefix, const char *rel) {
    size_t pl = strlen(prefix);
    for (int i = 0; i < ms->n; i++)
        if (!strncmp(ms->m[i].name, prefix, pl) && !strcmp(ms->m[i].name + pl, rel)) return &ms->m[i];
    return NULL;
}

static void parse_stub(const char *text, size_t len, CartStub *st) {
    const char *p = text, *end = text + len;
    while (p < end) {
        const char *nl = memchr(p, '\n', (size_t)(end - p));
        if (!nl) nl = end;
        char line[512];
        snprintf(line, sizeof line, "%.*s", (int)(nl - p), p);
        p = nl + 1;
        char *s = line;
        while (isspace((unsigned char)*s)) s++;
        char *eq = strchr(s, '=');
        if (*s == '#' || !eq) continue;
        char *k = s, *v = eq + 1, *ke = eq;
        while (ke > k && isspace((unsigned char)ke[-1])) ke--;
        *ke = 0;
        while (isspace((unsigned char)*v)) v++;
        char *ve = v + strlen(v);
        while (ve > v && isspace((unsigned char)ve[-1])) *--ve = 0;
        for (char *c = v; *c; c++) if (*c == '\t') *c = ' ';    /* the cache is tab separated */
        if (!strcasecmp(k, "title")) snprintf(st->title, sizeof st->title, "%s", v);
        else if (!strcasecmp(k, "author")) snprintf(st->author, sizeof st->author, "%s", v);
        else if (!strcasecmp(k, "version")) snprintf(st->version, sizeof st->version, "%s", v);
    }
}

/* ---- the stub cache ---- */

typedef struct { char *path; long long size, mtime; CartStub st; } CacheLine;
static CacheLine *cache;
static int ncache;
static bool cache_read;

static void cache_file(char *out, size_t n) {
    const char *env = getenv("GOOBERENGINE_CACHE");
    if (env && *env) { snprintf(out, n, "%s", env); return; }
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", save_dir());
    char *slash = strrchr(dir, '/');
    if (slash && slash != dir) *slash = 0;
    snprintf(out, n, "%s/cartcache.txt", dir);
}

static void cache_load(void) {
    if (cache_read) return;
    cache_read = true;
    char path[PATH_MAX];
    cache_file(path, sizeof path);
    FILE *f = fopen(path, "r");
    char line[2048];
    while (f && fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        char *field[7] = {0};
        int nf = 0;
        for (char *s = line; nf < 7; nf++) {
            field[nf] = s;
            char *tab = strchr(s, '\t');
            if (!tab) { nf++; break; }
            *tab = 0;
            s = tab + 1;
        }
        if (nf < 4) continue;
        cache = realloc(cache, sizeof *cache * (size_t)(ncache + 1));
        CacheLine *c = &cache[ncache++];
        memset(c, 0, sizeof *c);
        c->path = strdup(field[0]);
        c->size = atoll(field[1]);
        c->mtime = atoll(field[2]);
        c->st.ok = field[3][0] == '1';
        if (nf > 4) snprintf(c->st.title, sizeof c->st.title, "%s", field[4]);
        if (nf > 5) snprintf(c->st.author, sizeof c->st.author, "%s", field[5]);
        if (nf > 6) snprintf(c->st.version, sizeof c->st.version, "%s", field[6]);
    }
    if (f) fclose(f);
}

static void cache_save(void) {
    char path[PATH_MAX], tmp[PATH_MAX + 16];
    cache_file(path, sizeof path);
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", path);
    char *slash = strrchr(dir, '/');
    if (slash && slash != dir) { *slash = 0; mkdir(dir, 0755); }
    FILE *f = fopen(tmp, "w");
    if (!f) return;                 /* no cache: DIR just reads the archives again next time */
    for (int i = 0; i < ncache; i++) {
        struct stat s;
        if (stat(cache[i].path, &s) != 0) continue;       /* the archive is gone */
        fprintf(f, "%s\t%lld\t%lld\t%d\t%s\t%s\t%s\n", cache[i].path, cache[i].size, cache[i].mtime, cache[i].st.ok,
                cache[i].st.title, cache[i].st.author, cache[i].st.version);
    }
    if (fclose(f) == 0) rename(tmp, path);
    else remove(tmp);
}

bool archive_stub(const char *file, CartStub *st) {
    memset(st, 0, sizeof *st);
    char abs[PATH_MAX];
    if (!realpath(file, abs)) return false;
    struct stat s;
    if (stat(abs, &s) != 0) return false;
    cache_load();
    for (int i = 0; i < ncache; i++)
        if (!strcmp(cache[i].path, abs) && cache[i].size == (long long)s.st_size && cache[i].mtime == (long long)s.st_mtime) {
            *st = cache[i].st;
            return st->ok;
        }
    /* not cached, or the archive changed: read it */
    memset(st, 0, sizeof *st);
    Members ms = {0};
    char err[512], prefix[256];
    if (read_archive(abs, &ms, err, sizeof err) && cart_prefix(&ms, prefix, sizeof prefix)) {
        st->ok = true;
        const Member *m = member(&ms, prefix, "cart.txt");
        if (m) parse_stub((const char *)m->data, m->len, st);
    }
    members_free(&ms);
    CacheLine *c = NULL;
    for (int i = 0; i < ncache && !c; i++) if (!strcmp(cache[i].path, abs)) c = &cache[i];
    if (!c) {
        cache = realloc(cache, sizeof *cache * (size_t)(ncache + 1));
        c = &cache[ncache++];
        c->path = strdup(abs);
    }
    c->size = (long long)s.st_size;
    c->mtime = (long long)s.st_mtime;
    c->st = *st;
    cache_save();
    return st->ok;
}

/* ---- names ---- */

/* games/Rocket Game (1).zip -> Rocket_Game: a name a cart folder can have */
void archive_cart_name(const char *file, char *out, size_t n) {
    const char *slash = strrchr(file, '/');
    const char *base = slash ? slash + 1 : file;
    size_t len = strlen(base) - ext_len(base);
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%.*s", (int)len, base);
    char *paren = strstr(tmp, " (");          /* a browser's "rocket (1).zip" */
    if (paren && paren[2] && isdigit((unsigned char)paren[2])) *paren = 0;
    size_t o = 0;
    for (const char *p = tmp; *p && o + 1 < n; p++)
        out[o++] = (isalnum((unsigned char)*p) || *p == '-' || *p == '_') ? *p : '_';
    while (o && out[o - 1] == '_') o--;
    out[o] = 0;
    if (!o) snprintf(out, n, "cart");
}

/* LOAD rocket: no folder rocket/, but rocket.zip (or .tar.gz ...) is there */
bool archive_find(const char *name, char *out, size_t n) {
    struct stat s;
    if (archive_name(name)) {
        if (stat(name, &s) == 0 && S_ISREG(s.st_mode)) { snprintf(out, n, "%s", name); return true; }
        snprintf(out, n, "%s/%s", carts_root(), name);
        return stat(out, &s) == 0 && S_ISREG(s.st_mode);
    }
    for (int i = 0; i < NEXTS; i++) {
        snprintf(out, n, "%s/%s%s", carts_root(), name, exts[i]);
        if (stat(out, &s) == 0 && S_ISREG(s.st_mode)) return true;
    }
    return false;
}

/* ---- unpacking ---- */

uint8_t *archive_member(const char *file, const char *name, size_t *len, char *err, int errlen) {
    Members ms = {0};
    if (!read_archive(file, &ms, err, errlen)) return NULL;
    uint8_t *data = NULL;
    for (int i = 0; i < ms.n; i++) {
        const char *n = ms.m[i].name;
        if (!strncmp(n, "./", 2)) n += 2;
        if (strcmp(n, name)) continue;
        *len = ms.m[i].len;
        data = realloc(ms.m[i].data, *len + 1);     /* zero-ended, so text can be used as a string */
        if (!data) break;
        data[*len] = 0;
        ms.m[i].data = NULL;        /* the caller has it now */
        break;
    }
    members_free(&ms);
    if (!data) snprintf(err, (size_t)errlen, "%s has no %s", file, name);
    return data;
}

bool archive_unpack(const char *file, const char *name, char *out_dir, int outlen, char *err, int errlen) {
    char base[128];
    if (!name || !*name) { archive_cart_name(file, base, sizeof base); name = base; }
    for (const char *p = name; *p; p++)
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-') {
            snprintf(err, (size_t)errlen, "a cart name uses letters, digits, - and _ only (IMPORT file name)");
            return false;
        }
    char dir[PATH_MAX + 160];
    snprintf(dir, sizeof dir, "%s/%s", carts_root(), name);
    struct stat s;
    if (stat(dir, &s) == 0) { snprintf(err, (size_t)errlen, "cart '%s' already exists: IMPORT %s othername", name, file); return false; }
    Members ms = {0};
    if (!read_archive(file, &ms, err, errlen)) return false;
    char prefix[256];
    if (!cart_prefix(&ms, prefix, sizeof prefix)) {
        members_free(&ms);
        snprintf(err, (size_t)errlen, "%s has no main.gb (at the top, or in one folder)", file);
        return false;
    }
    if (mkdir(dir, 0755) != 0) { members_free(&ms); snprintf(err, (size_t)errlen, "could not create %s", dir); return false; }
    size_t pl = strlen(prefix);
    char *written[MAX_MEMBERS];
    int nw = 0;
    bool ok = true;
    for (int i = 0; i < ms.n && ok; i++) {
        const Member *m = &ms.m[i];
        if (strncmp(m->name, prefix, pl) || !cart_bundled_name(m->name + pl)) continue;  /* not one of the cart's files */
        const char *rel = m->name + pl, *slash = strchr(rel, '/');
        char p[PATH_MAX + 512];
        if (slash) {
            snprintf(p, sizeof p, "%s/%.*s", dir, (int)(slash - rel), rel);
            mkdir(p, 0755);
        }
        snprintf(p, sizeof p, "%s/%s", dir, rel);
        FILE *f = fopen(p, "wb");
        ok = f && fwrite(m->data, 1, m->len, f) == m->len;
        if (f) ok = fclose(f) == 0 && ok;
        if (f) written[nw++] = strdup(p);
        if (!ok) snprintf(err, (size_t)errlen, "could not write %s", p);
    }
    members_free(&ms);
    if (!ok) {   /* don't leave half a cart behind */
        for (int i = 0; i < nw; i++) remove(written[i]);
        for (int k = 0; k < NRES; k++) {
            char p[PATH_MAX + 240];
            snprintf(p, sizeof p, "%s/%s", dir, res_dir[k]);
            rmdir(p);
        }
        rmdir(dir);
    }
    for (int i = 0; i < nw; i++) free(written[i]);
    if (ok) snprintf(out_dir, (size_t)outlen, "%s", dir);
    return ok;
}

/* ---- writing ---- */

typedef struct { uint8_t *s; size_t n, cap; } Bytes;
static void put(Bytes *b, const void *p, size_t n) {
    if (b->n + n > b->cap) { b->cap = (b->n + n) * 2; b->s = realloc(b->s, b->cap); }
    memcpy(b->s + b->n, p, n);
    b->n += n;
}
static void put16(Bytes *b, unsigned v) { uint8_t x[2] = {(uint8_t)v, (uint8_t)(v >> 8)}; put(b, x, 2); }
static void put32(Bytes *b, uint32_t v) { uint8_t x[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)}; put(b, x, 4); }

static void zip_add(Bytes *z, Bytes *cd, const char *name, const uint8_t *data, size_t len) {
    size_t dn;
    uint8_t *d = ge_deflate(data, len, &dn);
    bool deflated = dn < len;
    const uint8_t *body = deflated ? d : data;
    size_t blen = deflated ? dn : len;
    uint32_t crc = ge_crc32(data, len), offset = (uint32_t)z->n;
    unsigned nl = (unsigned)strlen(name);
    put32(z, 0x04034b50); put16(z, 20); put16(z, 0); put16(z, deflated ? 8 : 0);
    put16(z, 0); put16(z, 0x21);                  /* 1980-01-01: the same cart gives the same bytes */
    put32(z, crc); put32(z, (uint32_t)blen); put32(z, (uint32_t)len); put16(z, nl); put16(z, 0);
    put(z, name, nl);
    put(z, body, blen);
    put32(cd, 0x02014b50); put16(cd, 0x031e); put16(cd, 20); put16(cd, 0); put16(cd, deflated ? 8 : 0);
    put16(cd, 0); put16(cd, 0x21);
    put32(cd, crc); put32(cd, (uint32_t)blen); put32(cd, (uint32_t)len); put16(cd, nl);
    put16(cd, 0); put16(cd, 0); put16(cd, 0); put16(cd, 0);
    put32(cd, 0100644u << 16);                    /* a plain file, rw-r--r-- */
    put32(cd, offset);
    put(cd, name, nl);
    free(d);
}

static void tar_add(Bytes *t, const char *name, const uint8_t *data, size_t len) {
    uint8_t h[512] = {0};
    snprintf((char *)h, 100, "%s", name);
    memcpy(h + 100, "0000644", 7);
    memcpy(h + 108, "0000000", 7);
    memcpy(h + 116, "0000000", 7);
    snprintf((char *)h + 124, 12, "%011o", (unsigned)len);
    memcpy(h + 136, "00000000000", 11);           /* mtime 0: reproducible */
    h[156] = '0';
    memcpy(h + 257, "ustar", 6);
    memcpy(h + 263, "00", 2);
    memset(h + 148, ' ', 8);
    unsigned sum = 0;
    for (int i = 0; i < 512; i++) sum += h[i];
    snprintf((char *)h + 148, 8, "%06o", sum);
    put(t, h, 512);
    put(t, data, len);
    static const uint8_t zero[512];
    if (len % 512) put(t, zero, 512 - len % 512);
}

/* the cart folder `dir` as archive bytes, its files inside a folder `top`
 * (top/main.gb ...): a .zip, or with tgz a .tar.gz. malloc'd */
uint8_t *archive_pack(const char *dir, const char *top, bool tgz, size_t *len, char *err, int errlen) {
    char *names[512];
    int n = cart_file_list(dir, names, 512);
    if (n < 0) { snprintf(err, (size_t)errlen, "cannot read %s", dir); return NULL; }
    Bytes body = {0}, cd = {0};
    bool ok = true;
    for (int i = 0; i < n && ok; i++) {
        char p[PATH_MAX + 200], name[300];
        snprintf(p, sizeof p, "%s/%s", dir, names[i]);
        snprintf(name, sizeof name, "%s/%s", top, names[i]);
        uint8_t *data;
        size_t dlen;
        if (!read_all(p, &data, &dlen, err, errlen)) { ok = false; break; }
        if (tgz) tar_add(&body, name, data, dlen);
        else zip_add(&body, &cd, name, data, dlen);
        free(data);
    }
    for (int i = 0; i < n; i++) free(names[i]);
    if (!ok) { free(body.s); free(cd.s); return NULL; }
    uint8_t *out;
    if (!tgz) {
        uint32_t cd_at = (uint32_t)body.n, cd_len = (uint32_t)cd.n;
        put(&body, cd.s, cd.n);
        put32(&body, 0x06054b50); put16(&body, 0); put16(&body, 0);
        put16(&body, (unsigned)n); put16(&body, (unsigned)n);
        put32(&body, cd_len); put32(&body, cd_at); put16(&body, 0);
        out = body.s;
        *len = body.n;
    } else {
        static const uint8_t zero[1024];
        put(&body, zero, sizeof zero);            /* two zero blocks end a tar */
        out = ge_gzip(body.s, body.n, len);
        free(body.s);
    }
    free(cd.s);
    return out;
}

/* EXPORT [file]: zip the loaded cart. No file: carts/<name>.zip; a name without
 * .zip gets it (EXPORT ~/rocket -> ~/rocket.zip); .tar.gz / .tgz gzip a tar instead */
bool cart_export(const char *path, char *out_path, int outlen, char *err, int errlen) {
    if (!cart.loaded) { snprintf(err, (size_t)errlen, "no cart loaded"); return false; }
    char file[PATH_MAX + 160];
    size_t pl = path ? strlen(path) : 0;
    if (pl > 7 && !strcasecmp(path + pl - 7, ".gbcart")) {
        snprintf(err, (size_t)errlen, "carts are shared as .zip now: EXPORT %.*s.zip", (int)(pl - 7), path);
        return false;
    }
    if (!pl) snprintf(file, sizeof file, "%s/%s.zip", carts_root(), cart.name);
    else if (!archive_name(path)) snprintf(file, sizeof file, "%s.zip", path);
    else if (!strcasecmp(path + strlen(path) - 4, ".tar")) snprintf(file, sizeof file, "%s.gz", path);   /* x.tar -> x.tar.gz */
    else snprintf(file, sizeof file, "%s", path);
    size_t fl = strlen(file);
    bool tgz = fl > 4 && strcasecmp(file + fl - 4, ".zip");
    size_t len;
    uint8_t *out = archive_pack(cart.dir, cart.name, tgz, &len, err, errlen);
    if (!out) return false;
    char tmp[PATH_MAX + 200];
    snprintf(tmp, sizeof tmp, "%s.saving", file);
    FILE *f = fopen(tmp, "wb");
    bool ok = f && fwrite(out, 1, len, f) == len;
    if (f) ok = fclose(f) == 0 && ok && rename(tmp, file) == 0;
    if (!ok) remove(tmp);
    free(out);
    if (!ok) { snprintf(err, (size_t)errlen, "could not write %s", file); return false; }
    snprintf(out_path, (size_t)outlen, "%s", file);
    return true;
}
