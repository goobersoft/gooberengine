/* Standalone games.
 *
 * An exported game is a prebuilt "player" runtime (GooberEngine without the
 * shell and editors, built once by `make runtimes`) with the cart packed in.
 * Nothing is compiled at export time, so any GooberEngine - including the web
 * one - can export to every target.
 *
 * The packed cart is the cart's .gbcart text (its source and data files; the
 * player compiles main.gb when it starts, which takes milliseconds).
 *
 *   Linux / Windows: the runtime executable, then the .gbcart bytes, then an
 *   80-byte trailer: the cart name (64 bytes, zero padded), the .gbcart length
 *   (8 bytes, little-endian) and the magic "GOOBERPK". Executables ignore
 *   bytes after their end, so the result is still an ordinary program.
 *
 *   Web: the runtime's index.html with the .gbcart embedded (base64) in a
 *   <script> tag, next to player.js and player.wasm: three files for any static
 *   host, also written as a .zip ready for upload.
 */
#include "engine.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define PACK_MAGIC "GOOBERPK"
#define PACK_NAME 64
#define PACK_TRAILER (PACK_NAME + 8 + 8)

/* ---- platform bits declared in compat.h ---- */

#ifdef _WIN32
int ge_rename(const char *from, const char *to) {
    return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) ? 0 : -1;
}
#endif

bool ge_exe_path(char *out, size_t n) {
#if defined(_WIN32)
    DWORD r = GetModuleFileNameA(NULL, out, (DWORD)n);
    return r > 0 && r < n;
#elif defined(__EMSCRIPTEN__)
    (void)out; (void)n;
    return false;
#else
    ssize_t r = readlink("/proc/self/exe", out, n - 1);
    if (r <= 0) return false;
    out[r] = 0;
    return true;
#endif
}

const char *ge_temp_dir(void) {
    static char dir[PATH_MAX];
#if defined(_WIN32)
    char base[MAX_PATH];
    if (!GetTempPathA(sizeof base, base)) return NULL;
    snprintf(dir, sizeof dir, "%sgooberengine-%lu-%lu", base, (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetTickCount());
    return _mkdir(dir) == 0 ? dir : NULL;
#elif defined(__EMSCRIPTEN__)
    snprintf(dir, sizeof dir, "/tmp/gooberengine-cart");
    mkdir(dir, 0755);
    return dir;
#else
    const char *t = getenv("TMPDIR");
    snprintf(dir, sizeof dir, "%s/gooberengine-XXXXXX", t && *t ? t : "/tmp");
    return mkdtemp(dir);
#endif
}

/* ---- small file helpers ---- */

static bool read_file(const char *path, char **data, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return false; }
    *data = malloc((size_t)n + 1);
    bool ok = fread(*data, 1, (size_t)n, f) == (size_t)n;
    fclose(f);
    if (!ok) { free(*data); return false; }
    (*data)[n] = 0;
    *len = (size_t)n;
    return true;
}

typedef struct { const void *p; size_t n; } Part;

static bool write_parts(const char *path, const Part *parts, int nparts, bool exec) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = true;
    for (int i = 0; i < nparts; i++) ok = ok && fwrite(parts[i].p, 1, parts[i].n, f) == parts[i].n;
    ok = fclose(f) == 0 && ok;
#ifndef _WIN32
    if (ok && exec) chmod(path, 0755);
#else
    (void)exec;
#endif
    return ok;
}

static void mkdirs(const char *path) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof tmp, "%s", path);
    for (char *p = tmp + 1; *p; p++)
        if (*p == '/') { *p = 0; mkdir(tmp, 0755); *p = '/'; }
    mkdir(tmp, 0755);
}

static bool is_dir(const char *p) { struct stat st; return stat(p, &st) == 0 && S_ISDIR(st.st_mode); }

/* a folder and the files in it, plus `depth` levels of sub-folders (the unpack folder holds a cart, which holds tiles/ ...) */
static void remove_tree(const char *dir, int depth) {
    DIR *d = opendir(dir);
    struct dirent *e;
    while (d && (e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        char p[PATH_MAX + 256];
        snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
        if (depth > 0 && is_dir(p)) remove_tree(p, depth - 1);
        else remove(p);
    }
    if (d) closedir(d);
    rmdir(dir);
}

/* ---- runtimes ---- */

static const char *runtimes_dir(void) {
    static char dir[PATH_MAX];
#ifdef __EMSCRIPTEN__
    snprintf(dir, sizeof dir, "/runtimes");
#else
    const char *env = getenv("GOOBERENGINE_RUNTIMES");
    if (env && *env) { snprintf(dir, sizeof dir, "%s", env); return dir; }
    char *base = SDL_GetBasePath();      /* next to the gooberengine executable */
    if (base) {
        snprintf(dir, sizeof dir, "%sruntimes", base);
        SDL_free(base);
        if (is_dir(dir)) return dir;
    }
    snprintf(dir, sizeof dir, "runtimes");
#endif
    return dir;
}

static bool load_runtime(const char *rel, char **data, size_t *len, ExportLog log) {
    char path[PATH_MAX + 64];
    snprintf(path, sizeof path, "%s/%s", runtimes_dir(), rel);
    if (read_file(path, data, len)) return true;
#ifdef __EMSCRIPTEN__
    /* the web engine fetches runtimes from its own server the first time they're needed */
    char url[PATH_MAX + 64];
    snprintf(url, sizeof url, "runtimes/%s", rel);
    mkdirs("/runtimes/web");
    log("fetching the player runtime...", false);
    emscripten_wget(url, path);
    if (read_file(path, data, len) && *len > 0) return true;
#endif
    char msg[PATH_MAX + 160];
    snprintf(msg, sizeof msg, "missing player runtime %s (build them with: make runtimes)", path);
    log(msg, true);
    return false;
}

/* ---- the .zip writer (stored, no compression: the web files are what they are) ---- */

static void le16(uint8_t *p, unsigned v) { p[0] = v & 255; p[1] = (v >> 8) & 255; }
static void le32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (v >> (8 * i)) & 255; }

static bool write_zip(const char *path, const char *const *names, char *const *datas, const size_t *lens, int n) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    uint8_t central[8][46 + 128];
    size_t central_len[8];
    uint32_t offset = 0;
    bool ok = true;
    for (int i = 0; i < n && i < 8; i++) {
        size_t nl = strlen(names[i]);
        uint32_t crc = ge_crc32(datas[i], lens[i]);
        uint8_t h[30] = {0};
        le32(h, 0x04034b50);
        le16(h + 4, 20);                 /* version needed: 2.0 */
        le16(h + 12, 0x21);              /* date: 1980-01-01 (reproducible) */
        le32(h + 14, crc);
        le32(h + 18, (uint32_t)lens[i]);
        le32(h + 22, (uint32_t)lens[i]);
        le16(h + 26, (unsigned)nl);
        ok = ok && fwrite(h, 1, 30, f) == 30 && fwrite(names[i], 1, nl, f) == nl &&
             fwrite(datas[i], 1, lens[i], f) == lens[i];
        uint8_t *c = central[i];
        memset(c, 0, 46);
        le32(c, 0x02014b50);
        le16(c + 4, 20);
        le16(c + 6, 20);
        le16(c + 14, 0x21);
        le32(c + 16, crc);
        le32(c + 20, (uint32_t)lens[i]);
        le32(c + 24, (uint32_t)lens[i]);
        le16(c + 28, (unsigned)nl);
        le32(c + 42, offset);
        memcpy(c + 46, names[i], nl);
        central_len[i] = 46 + nl;
        offset += (uint32_t)(30 + nl + lens[i]);
    }
    uint32_t cd_size = 0;
    for (int i = 0; i < n; i++) { ok = ok && fwrite(central[i], 1, central_len[i], f) == central_len[i]; cd_size += (uint32_t)central_len[i]; }
    uint8_t end[22] = {0};
    le32(end, 0x06054b50);
    le16(end + 8, (unsigned)n);
    le16(end + 10, (unsigned)n);
    le32(end + 12, cd_size);
    le32(end + 16, offset);
    ok = ok && fwrite(end, 1, 22, f) == 22;
    return fclose(f) == 0 && ok;
}

/* ---- export ---- */

static char *base64(const char *data, size_t n) {
    static const char t[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *out = malloc((n + 2) / 3 * 4 + 1), *o = out;
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < n; i += 3) {
        uint32_t v = (uint32_t)p[i] << 16 | (i + 1 < n ? (uint32_t)p[i + 1] << 8 : 0) | (i + 2 < n ? p[i + 2] : 0);
        *o++ = t[v >> 18 & 63];
        *o++ = t[v >> 12 & 63];
        *o++ = i + 1 < n ? t[v >> 6 & 63] : '=';
        *o++ = i + 2 < n ? t[v & 63] : '=';
    }
    *o = 0;
    return out;
}

/* the template with every `key` replaced by `value` (keys: __GE_TITLE__ ...) */
static char *fill_template(const char *tpl, const char *const *keys, const char *const *values, int n) {
    size_t cap = strlen(tpl) + 1;
    for (int i = 0; i < n; i++) cap += strlen(values[i]) * 4;    /* each key appears a few times at most */
    char *out = malloc(cap), *o = out;
    while (*tpl) {
        int k = -1;
        for (int i = 0; i < n && k < 0; i++) if (!strncmp(tpl, keys[i], strlen(keys[i]))) k = i;
        if (k < 0) { *o++ = *tpl++; continue; }
        size_t vl = strlen(values[k]);
        if ((size_t)(o - out) + vl + strlen(tpl) + 1 > cap) {
            size_t at = (size_t)(o - out);
            cap = (at + vl + strlen(tpl) + 1) * 2;
            out = realloc(out, cap);
            o = out + at;
        }
        memcpy(o, values[k], vl);
        o += vl;
        tpl += strlen(keys[k]);
    }
    *o = 0;
    return out;
}

static void html_escape(const char *s, char *out, size_t n) {
    size_t o = 0;
    for (; *s && o + 7 < n; s++) {
        const char *r = *s == '<' ? "&lt;" : *s == '>' ? "&gt;" : *s == '&' ? "&amp;" : *s == '"' ? "&quot;" : NULL;
        if (r) { memcpy(out + o, r, strlen(r)); o += strlen(r); }
        else out[o++] = *s;
    }
    out[o] = 0;
}

static bool export_native(const char *rt_name, const char *out_path, const char *pack, size_t len, ExportLog log) {
    char *rt;
    size_t rt_len;
    if (!load_runtime(rt_name, &rt, &rt_len, log)) return false;
    uint8_t trailer[PACK_TRAILER] = {0};
    snprintf((char *)trailer, PACK_NAME, "%s", cart.name);
    for (int i = 0; i < 8; i++) trailer[PACK_NAME + i] = (uint8_t)((uint64_t)len >> (8 * i));
    memcpy(trailer + PACK_NAME + 8, PACK_MAGIC, 8);
    Part parts[] = {{rt, rt_len}, {pack, len}, {trailer, sizeof trailer}};
    bool ok = write_parts(out_path, parts, 3, true);
    free(rt);
    char msg[PATH_MAX + 64];
    snprintf(msg, sizeof msg, ok ? "wrote %s" : "could not write %s", out_path);
    log(msg, !ok);
    if (ok) web_offer_download(out_path);
    return ok;
}

static bool export_web(const char *out_dir, const char *pack, size_t len, ExportLog log) {
    static const char *files[] = {"index.html", "player.js", "player.wasm"};
    char *data[3] = {0};
    size_t lens[3];
    bool ok = true;
    for (int i = 0; i < 3 && ok; i++) {
        char rel[64];
        snprintf(rel, sizeof rel, "web/%s", files[i]);
        ok = load_runtime(rel, &data[i], &lens[i], log);
    }
    if (ok) {
        char title[512], *b64 = base64(pack, len);
        html_escape(cart.title, title, sizeof title);
        const char *keys[] = {"__GE_TITLE__", "__GE_CART_NAME__", "__GE_CART__"};
        const char *values[] = {title, cart.name, b64};
        char *html = fill_template(data[0], keys, values, 3);
        free(data[0]);
        free(b64);
        data[0] = html;
        lens[0] = strlen(html);
        char zip[PATH_MAX + 64];
        snprintf(zip, sizeof zip, "%s/%s-web.zip", out_dir, cart.name);
#ifndef __EMSCRIPTEN__
        /* the three files, ready to copy to a web server */
        char dir[PATH_MAX + 64];
        snprintf(dir, sizeof dir, "%s/%s-web", out_dir, cart.name);
        mkdirs(dir);
        for (int i = 0; i < 3 && ok; i++) {
            char p[PATH_MAX + 96];
            snprintf(p, sizeof p, "%s/%s", dir, files[i]);
            Part part = {data[i], lens[i]};
            ok = write_parts(p, &part, 1, false);
        }
        char msg[PATH_MAX + 64];
        snprintf(msg, sizeof msg, ok ? "wrote %s/ (index.html, player.js, player.wasm)" : "could not write %s", dir);
        log(msg, !ok);
#endif
        /* ...and as one .zip (what itch.io and friends want; the web version downloads it) */
        if (ok) {
            ok = write_zip(zip, files, data, lens, 3);
            char msg2[PATH_MAX + 64];
            snprintf(msg2, sizeof msg2, ok ? "wrote %s" : "could not write %s", zip);
            log(msg2, !ok);
            if (ok) web_offer_download(zip);
        }
    }
    for (int i = 0; i < 3; i++) free(data[i]);
    return ok;
}

bool game_export(const char *targets, const char *outdir, ExportLog log) {
    if (!cart.loaded) { log("no cart loaded", true); return false; }
    bool linux_ = false, windows = false, web = false;
    char words[512], out[PATH_MAX];
    snprintf(words, sizeof words, "%s", targets ? targets : "");
    out[0] = 0;
    if (outdir) snprintf(out, sizeof out, "%s", outdir);
    for (char *w = strtok(words, " \t"); w; w = strtok(NULL, " \t")) {
        if (!strcasecmp(w, "linux")) linux_ = true;
        else if (!strcasecmp(w, "windows")) windows = true;
        else if (!strcasecmp(w, "web")) web = true;
        else if (!strcasecmp(w, "all")) linux_ = windows = web = true;
        else if (!out[0]) snprintf(out, sizeof out, "%s", w);      /* a folder to write to */
        else { log("EXPORT takes LINUX, WINDOWS, WEB or ALL, then a folder (optional)", true); return false; }
    }
    if (!linux_ && !windows && !web) { log("export to what? LINUX, WINDOWS, WEB or ALL", true); return false; }
    if (!out[0]) {
#ifdef __EMSCRIPTEN__
        snprintf(out, sizeof out, "/tmp/exports/%s", cart.name);
#else
        snprintf(out, sizeof out, "exports/%s", cart.name);
#endif
    }
    mkdirs(out);
    char err[256];
    size_t len;
    char *pack = cart_pack(&len, err, sizeof err);
    if (!pack) { log(err, true); return false; }
    bool ok = true;
    char path[PATH_MAX + 96];
    if (linux_) {
        snprintf(path, sizeof path, "%s/%s-linux-x86_64", out, cart.name);
        ok = export_native("player-linux-x86_64", path, pack, len, log) && ok;
    }
    if (windows) {
        snprintf(path, sizeof path, "%s/%s-windows.exe", out, cart.name);
        ok = export_native("player-windows-x86_64.exe", path, pack, len, log) && ok;
    }
    if (web) ok = export_web(out, pack, len, log) && ok;
    free(pack);
    return ok;
}

/* ---- the player: what an exported game runs ---- */

/* the .gbcart packed onto the end of this executable */
static bool read_own_pack(char *name, char **pack, size_t *len) {
    char exe[PATH_MAX];
    if (!ge_exe_path(exe, sizeof exe)) return false;
    FILE *f = fopen(exe, "rb");
    if (!f) return false;
    uint8_t t[PACK_TRAILER];
    bool ok = fseek(f, -PACK_TRAILER, SEEK_END) == 0 && fread(t, 1, PACK_TRAILER, f) == PACK_TRAILER &&
              !memcmp(t + PACK_NAME + 8, PACK_MAGIC, 8);
    uint64_t n = 0;
    for (int i = 0; ok && i < 8; i++) n |= (uint64_t)t[PACK_NAME + i] << (8 * i);
    ok = ok && n > 0 && n < (64u << 20) && fseek(f, -(long)(PACK_TRAILER + n), SEEK_END) == 0;
    if (ok) {
        *pack = malloc((size_t)n + 1);
        ok = fread(*pack, 1, (size_t)n, f) == n;
        if (!ok) free(*pack);
        else { (*pack)[n] = 0; *len = (size_t)n; }
    }
    fclose(f);
    if (ok) { memcpy(name, t, PACK_NAME); name[PACK_NAME] = 0; }
    return ok;
}

static void player_fail(const char *msg) {
    fprintf(stderr, "%s\n", msg);
#if !defined(__EMSCRIPTEN__)
    if (!opt.headless) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GooberEngine", msg, NULL);
#endif
}

static bool valid_cart_name(const char *s) {
    if (!*s) return false;
    for (; *s; s++) if (!isalnum((unsigned char)*s) && *s != '_' && *s != '-') return false;
    return true;
}

/* the cart's error, on screen, until the window is closed */
static void error_screen(void) {
    while (!in.quit) {
        draw_reset();
        draw_cls(COL_NAVY);
        draw_text("The game stopped with an error:", 4, 4, COL_YELLOW);
        int y = 4 + FONT_H * 2;
        const char *s = player_errors;
        while (*s && y < SCREEN_H - FONT_H) {
            char line[TEXT_COLS + 1];
            int n = 0;
            while (s[n] && s[n] != '\n' && n < TEXT_COLS - 1) n++;
            snprintf(line, sizeof line, "%.*s", n, s);
            draw_text(line, 4, y, COL_WHITE);
            y += FONT_H;
            s += n;
            if (*s == '\n') s++;
        }
        platform_frame();
    }
}

static void player_usage(void) {
    fprintf(stderr,
            "%s - made with GooberEngine %s\n"
            "  --scale N       window scale (default 3)\n"
            "  --headless      no window (with --frames, --shot, --type, --wav: for testing)\n"
            "  --cart file     play this .gbcart instead of the packed one\n",
            cart.title[0] ? cart.title : "a GooberEngine game", GE_VERSION);
}

int player_main(int argc, char **argv) {
    opt.player = true;
    const char *cart_file = NULL;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        bool more = i + 1 < argc;
        if (!strcmp(a, "--scale") && more) opt.scale = atoi(argv[++i]);
        else if (!strcmp(a, "--frames") && more) opt.max_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--shot") && more) opt.shot = argv[++i];
        else if (!strcmp(a, "--headless")) opt.headless = true;
        else if (!strcmp(a, "--type") && more) opt.script = argv[++i];
        else if (!strcmp(a, "--wav") && more) opt.wav = argv[++i];
        else if (!strcmp(a, "--cart") && more) cart_file = argv[++i];
        else if (!strcmp(a, "--version")) { printf("GooberEngine player %s\n", GE_VERSION); return 0; }
        else if (!strcmp(a, "-h") || !strcmp(a, "--help")) { player_usage(); return 0; }
        /* anything else (Windows and desktops pass odd things) is ignored */
    }
    if (opt.scale < 1) opt.scale = 1;
    if (opt.scale > 8) opt.scale = 8;
    if (opt.headless && opt.max_frames <= 0) opt.max_frames = 60;

    char name[PACK_NAME + 1] = "";
    char *pack = NULL;
    size_t len = 0;
    if (cart_file) {
        if (!read_file(cart_file, &pack, &len)) { player_fail("could not read the cart file"); return 1; }
        const char *slash = strrchr(cart_file, '/'), *bs = strrchr(cart_file, '\\');
        if (bs && (!slash || bs > slash)) slash = bs;
        snprintf(name, sizeof name, "%s", slash ? slash + 1 : cart_file);
        char *dot = strstr(name, ".gbcart");
        if (dot) *dot = 0;
    } else if (!read_own_pack(name, &pack, &len)) {
        player_fail("This is the GooberEngine player runtime with no game packed in it.\n"
                    "Export a cart from GooberEngine (EXPORT LINUX / WINDOWS / WEB) to make a game.");
        return 1;
    }
    if (!valid_cart_name(name)) snprintf(name, sizeof name, "game");

    /* unpack into a private folder, which the cart loader reads like any cart */
    const char *tmp = ge_temp_dir();
    if (!tmp) { player_fail("could not make a temporary folder for the game"); free(pack); return 1; }
    carts_root_set(tmp);
    char file[PATH_MAX + 96], dir[PATH_MAX + 96], err[512] = "";
    snprintf(file, sizeof file, "%s/%s.gbcart", tmp, name);
    Part part = {pack, len};
    bool ok = write_parts(file, &part, 1, false);
    free(pack);
    ok = ok && cart_import(file, name, dir, sizeof dir, err, sizeof err);
    remove(file);
    if (!ok || !cart_load(dir, err, sizeof err)) {
        player_fail(err[0] ? err : "could not unpack the game");
        remove_tree(tmp, 2);
        return 1;
    }

    if (!platform_init()) { remove_tree(tmp, 2); return 1; }
    platform_title(cart.title);
    while (!in.quit) {
        long start = frame_count;
        player_errors[0] = 0;
        int r = cart_run();
        input_consume();
        if (in.quit) break;
        if (r == 1) { error_screen(); break; }
        if (opt.headless && frame_count == start) break;   /* nothing would ever advance */
        /* the cart ended: start it again, like switching a console off and on */
    }
    platform_shutdown();
    remove_tree(tmp, 2);
    return 0;
}
