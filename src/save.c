/* Save data: a little key = value store for each cart (high scores, settings,
 * progress). It lives outside the cart folder, so playing a cart never changes
 * the cart itself:
 *
 *   $GOOBERENGINE_SAVES/<cart>.txt, else $XDG_DATA_HOME/gooberengine/saves/<cart>.txt,
 *   else ~/.local/share/gooberengine/saves/<cart>.txt   (Windows: %APPDATA%/GooberEngine/saves)
 *
 * Every save_data writes the file straight away, so nothing is lost if the
 * window is closed. Values are text; numbers convert both ways automatically. */
#include "engine.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_KEYS 256
#define MAX_KEY 64
#define MAX_VALUE 1024

typedef struct { char key[MAX_KEY + 1]; char *value; } Entry;

static void mkdirs(const char *path) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof tmp, "%s", path);
    for (char *p = tmp + 1; *p; p++)
        if (*p == '/') { *p = 0; mkdir(tmp, 0755); *p = '/'; }
    mkdir(tmp, 0755);
}

const char *save_dir(void) {
    static char dir[PATH_MAX];
    const char *env = getenv("GOOBERENGINE_SAVES"), *xdg = getenv("XDG_DATA_HOME"), *home = getenv("HOME");
    if (env && *env) snprintf(dir, sizeof dir, "%s", env);
    else if (xdg && *xdg) snprintf(dir, sizeof dir, "%s/gooberengine/saves", xdg);
#ifdef _WIN32
    else if (getenv("APPDATA")) snprintf(dir, sizeof dir, "%s/GooberEngine/saves", getenv("APPDATA"));
#endif
    else snprintf(dir, sizeof dir, "%s/.local/share/gooberengine/saves", home ? home : ".");
    return dir;
}

static void save_path(char *out, size_t n) { snprintf(out, n, "%s/%s.txt", save_dir(), cart.name); }

bool save_key_ok(const char *k) {
    size_t n = strlen(k);
    if (!n || n > MAX_KEY) return false;
    for (; *k; k++)
        if (!isalnum((unsigned char)*k) && *k != '_' && *k != '-' && *k != '.') return false;
    return true;
}

/* values are stored on one line: ~n for a line break, ~~ for ~ */
static char *unescape(const char *s) {
    char *out = malloc(strlen(s) + 1), *o = out;
    for (; *s; s++) {
        if (*s == '~' && s[1] == 'n') { *o++ = '\n'; s++; }
        else if (*s == '~' && s[1] == '~') { *o++ = '~'; s++; }
        else *o++ = *s;
    }
    *o = 0;
    return out;
}

static int read_all(Entry *e) {
    char path[PATH_MAX + 80];
    save_path(path, sizeof path);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    int n = 0;
    static char line[MAX_KEY + MAX_VALUE * 2 + 16];
    while (n < MAX_KEYS && fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        char *eq = strstr(line, " = ");
        if (!eq || line[0] == '#') continue;
        *eq = 0;
        if (!save_key_ok(line)) continue;
        snprintf(e[n].key, sizeof e[n].key, "%s", line);
        e[n].value = unescape(eq + 3);
        n++;
    }
    fclose(f);
    return n;
}

static void free_all(Entry *e, int n) { for (int i = 0; i < n; i++) free(e[i].value); }

/* returns a malloc'd copy of the value, or NULL if the key isn't saved */
char *save_load(const char *key) {
    static Entry e[MAX_KEYS];
    int n = read_all(e);
    char *r = NULL;
    for (int i = 0; i < n && !r; i++) if (!strcmp(e[i].key, key)) r = strdup(e[i].value);
    free_all(e, n);
    return r;
}

/* stores key = value ("" removes the key); returns an error message or NULL */
const char *save_store(const char *key, const char *value) {
    if (strlen(value) > MAX_VALUE) return "value is longer than 1024 characters";
    static Entry e[MAX_KEYS];
    int n = read_all(e), at = -1;
    for (int i = 0; i < n; i++) if (!strcmp(e[i].key, key)) at = i;
    if (at < 0 && *value) {
        if (n == MAX_KEYS) { free_all(e, n); return "a cart can save at most 256 keys"; }
        at = n++;
        snprintf(e[at].key, sizeof e[at].key, "%s", key);
        e[at].value = NULL;
    }
    if (at >= 0) { free(e[at].value); e[at].value = strdup(value); }

    mkdirs(save_dir());
    char path[PATH_MAX + 80], tmp[PATH_MAX + 96];
    save_path(path, sizeof path);
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    FILE *f = fopen(tmp, "w");
    if (!f) { free_all(e, n); return "could not write the save file"; }
    fprintf(f, "# save data for %s\n", cart.title);
    for (int i = 0; i < n; i++) {
        if (!*e[i].value) continue;
        fprintf(f, "%s = ", e[i].key);
        for (const char *p = e[i].value; *p; p++) {
            if (*p == '\n') fputs("~n", f);
            else if (*p == '~') fputs("~~", f);
            else if (*p != '\r') fputc(*p, f);
        }
        fputc('\n', f);
    }
    bool ok = fclose(f) == 0 && rename(tmp, path) == 0;
    free_all(e, n);
    return ok ? NULL : "could not write the save file";
}
