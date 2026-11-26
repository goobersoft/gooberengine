/* The built-in code editor: edits the loaded cart's .gb files (main.gb, and
 * the files it includes, picked in the file tree). Each open file keeps its
 * own buffer, cursor and undo history.
 *
 * The syntax colors come from the GooberBASIC compiler itself: keywords
 * from the lexer's table, commands from the native tables (core + console
 * API), type names and intrinsics from the compiler, constants from the
 * console's constant table, so the editor never drifts from the language. */
#include "engine.h"
#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "vm.h"
#include <SDL2/SDL_scancode.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <strings.h>

#define BAR_H FONT_H
#define TEXT_Y (BAR_H)
#define ROWS ((SCREEN_H - 2 * BAR_H) / FONT_H)
#define GUTTER 28
#define COLS ((UI_W - GUTTER) / FONT_W)
#define MAX_FILES 16
#define MAX_UNDO 100
#define INDENT 2

/* colors */
/* the editor's chrome follows the theme (UI_RGB); the syntax colors below don't */
#define C_BG UI_RGB(0, 0, 0)
#define C_CURLINE (ui_theme == THEME_GREY ? RGB6(1, 1, 1) : UI_RGB(0, 0, 1))   /* grey has no shade between black and dark grey */
#define C_SEL (ui_theme == THEME_GREY ? RGB6(2, 2, 2) : UI_RGB(1, 1, 2))   /* one step above C_CURLINE, which grey has taken */
#define C_ERRLINE RGB6(1, 0, 0)
#define C_BAR UI_RGB(0, 1, 2)
#define C_GUTTER UI_RGB(1, 1, 2)
#define C_TEXT RGB6(3, 3, 3)
#define C_KEYWORD RGB6(3, 2, 0)
#define C_TYPE RGB6(3, 1, 3)
#define C_COMMAND RGB6(1, 2, 3)
#define C_CONST RGB6(0, 3, 3)
#define C_NUMBER RGB6(3, 1, 1)
#define C_STRING RGB6(2, 3, 0)
#define C_COMMENT RGB6(1, 2, 1)
#define C_PUNCT RGB6(2, 2, 3)

typedef struct { char *s; int len, cap; } Line;
typedef struct { char *text; int cy, cx; } Snap;

typedef struct {
    Line *lines;
    int n, cap;
    int cy, cx, want_x;          /* cursor (want_x: column to aim for on up/down) */
    bool sel;
    int ay, ax;                  /* selection anchor; the other end is the cursor */
    int top, left;               /* scroll */
    bool follow;                 /* scroll to the cursor after this frame's input */
    bool modified;
    char path[PATH_MAX];
    Snap undo[MAX_UNDO], redo[MAX_UNDO];
    int nundo, nredo, last_kind, last_kind_line;
    bool *rem_at;                /* rem-block state at the start of each line */
    int rem_cap;
    bool rem_dirty;
    char status[512];
    int status_color;
    long status_until;
    bool status_open;            /* the full status text is shown in a panel (click the bar to open) */
    int err_line;                /* 1-based, 0 = none */
    int prompt;                  /* 0 none, 1 find, 2 go to line */
    char prompt_text[64];
    char find_text[64];
    long blink_from;
    bool dragging;
    bool fresh_newline;          /* the last thing done was Enter (auto indent: Backspace takes it back) */
} EState;
static EState E;
static EState parked[MAX_FILES];     /* the other open files */
static int nparked;

/* ---------------------------------------------------------------- buffer */

static void line_reserve(Line *l, int need) {
    if (need + 1 <= l->cap) return;
    int cap = l->cap ? l->cap : 16;
    while (cap < need + 1) cap *= 2;
    l->s = realloc(l->s, (size_t)cap);
    l->cap = cap;
}

static void line_set(Line *l, const char *s, int n) {
    line_reserve(l, n);
    memcpy(l->s, s, (size_t)n);
    l->len = n;
    l->s[n] = 0;
}

static void line_insert(Line *l, int at, const char *s, int n) {
    line_reserve(l, l->len + n);
    memmove(l->s + at + n, l->s + at, (size_t)(l->len - at + 1));
    memcpy(l->s + at, s, (size_t)n);
    l->len += n;
}

static void line_delete(Line *l, int at, int n) {
    memmove(l->s + at, l->s + at + n, (size_t)(l->len - at - n + 1));
    l->len -= n;
}

static void lines_insert(int at, const char *s, int n) {
    if (E.n == E.cap) {
        E.cap = E.cap ? E.cap * 2 : 64;
        E.lines = realloc(E.lines, sizeof(Line) * (size_t)E.cap);
    }
    memmove(E.lines + at + 1, E.lines + at, sizeof(Line) * (size_t)(E.n - at));
    memset(&E.lines[at], 0, sizeof(Line));
    line_set(&E.lines[at], s, n);
    E.n++;
    E.rem_dirty = true;
}

static void lines_remove(int at) {
    free(E.lines[at].s);
    memmove(E.lines + at, E.lines + at + 1, sizeof(Line) * (size_t)(E.n - at - 1));
    E.n--;
    E.rem_dirty = true;
}

static void clear_buffer(void) {
    for (int i = 0; i < E.n; i++) free(E.lines[i].s);
    E.n = 0;
}

/* replaces the buffer with text (tabs become spaces, \r is dropped) */
static void set_text(const char *t) {
    clear_buffer();
    const char *p = t;
    char buf[4096];
    for (;;) {
        int n = 0;
        while (*p && *p != '\n') {
            if (*p == '\t') { do buf[n++] = ' '; while (n % INDENT && n < (int)sizeof buf - 1); }
            else if (*p != '\r' && n < (int)sizeof buf - 1) buf[n++] = *p;
            p++;
        }
        lines_insert(E.n, buf, n);
        if (!*p) break;
        p++;
        if (!*p) break;          /* the file's final newline doesn't start a new line */
    }
    if (E.n == 0) lines_insert(0, "", 0);
    E.rem_dirty = true;
}

/* text between two positions (y0,x0) <= (y1,x1); malloc'd */
static char *get_range(int y0, int x0, int y1, int x1) {
    size_t total = 1;
    for (int y = y0; y <= y1; y++) total += (size_t)E.lines[y].len + 1;
    char *out = malloc(total), *o = out;
    for (int y = y0; y <= y1; y++) {
        int a = y == y0 ? x0 : 0, b = y == y1 ? x1 : E.lines[y].len;
        memcpy(o, E.lines[y].s + a, (size_t)(b - a));
        o += b - a;
        if (y < y1) *o++ = '\n';
    }
    *o = 0;
    return out;
}

static char *get_all(void) { return get_range(0, 0, E.n - 1, E.lines[E.n - 1].len); }

static void delete_range(int y0, int x0, int y1, int x1) {
    if (y0 == y1) { line_delete(&E.lines[y0], x0, x1 - x0); E.rem_dirty = true; return; }
    Line *a = &E.lines[y0], *b = &E.lines[y1];
    a->len = x0;
    a->s[x0] = 0;
    line_insert(a, x0, b->s + x1, b->len - x1);
    for (int y = y1; y > y0; y--) lines_remove(y);
}

/* inserts at the cursor and moves the cursor to the end of the inserted text */
static void insert_text(const char *s) {
    for (;;) {
        const char *nl = strchr(s, '\n');
        int n = nl ? (int)(nl - s) : (int)strlen(s);
        char clean[4096];
        int m = 0;
        for (int i = 0; i < n && m < (int)sizeof clean - INDENT; i++) {
            if (s[i] == '\t') { do clean[m++] = ' '; while ((E.cx + m) % INDENT); }
            else if (s[i] != '\r') clean[m++] = s[i];
        }
        line_insert(&E.lines[E.cy], E.cx, clean, m);
        E.cx += m;
        if (!nl) break;
        Line *l = &E.lines[E.cy];
        lines_insert(E.cy + 1, l->s + E.cx, l->len - E.cx);
        l = &E.lines[E.cy];
        l->len = E.cx;
        l->s[E.cx] = 0;
        E.cy++;
        E.cx = 0;
        s = nl + 1;
    }
    E.rem_dirty = true;
}

/* ---------------------------------------------------------------- selection, undo */

static int pos_cmp(int y0, int x0, int y1, int x1) { return y0 != y1 ? y0 - y1 : x0 - x1; }

static bool sel_range(int *y0, int *x0, int *y1, int *x1) {
    if (!E.sel || (E.ay == E.cy && E.ax == E.cx)) return false;
    if (pos_cmp(E.ay, E.ax, E.cy, E.cx) < 0) { *y0 = E.ay; *x0 = E.ax; *y1 = E.cy; *x1 = E.cx; }
    else { *y0 = E.cy; *x0 = E.cx; *y1 = E.ay; *x1 = E.ax; }
    return true;
}

static void free_snaps(Snap *s, int *n) {
    for (int i = 0; i < *n; i++) free(s[i].text);
    *n = 0;
}

static void push_snap(Snap *stack, int *n) {
    if (*n == MAX_UNDO) {
        free(stack[0].text);
        memmove(stack, stack + 1, sizeof(Snap) * (MAX_UNDO - 1));
        (*n)--;
    }
    stack[*n] = (Snap){get_all(), E.cy, E.cx};
    (*n)++;
}

/* call before every change. Typing (kind 1) and deleting (kind 2) on one
 * line group into a single undo step. */
enum { EDIT_OTHER = 0, EDIT_TYPE = 1, EDIT_DELETE = 2 };
static void before_edit(int kind) {
    E.err_line = 0;
    E.status_open = false;
    /* a click leaves an empty selection as the anchor for shift + click; an
     * edit drops it, or the anchor stays put while the cursor moves away and
     * it turns into a real selection, possibly past the end of the line */
    if (E.sel && E.ay == E.cy && E.ax == E.cx) E.sel = false;
    if (kind == EDIT_OTHER || kind != E.last_kind || E.cy != E.last_kind_line) push_snap(E.undo, &E.nundo);
    free_snaps(E.redo, &E.nredo);
    E.last_kind = kind;
    E.last_kind_line = E.cy;
    E.modified = true;
    E.follow = true;
}

static void restore(Snap *from, int *nfrom, Snap *to, int *nto) {
    if (!*nfrom) return;
    push_snap(to, nto);
    Snap s = from[--*nfrom];
    set_text(s.text);
    free(s.text);
    E.cy = s.cy < E.n ? s.cy : E.n - 1;
    E.cx = s.cx <= E.lines[E.cy].len ? s.cx : E.lines[E.cy].len;
    E.sel = false;
    E.last_kind = -1;
    E.modified = true;
    E.follow = true;
}

static bool delete_selection(void) {
    int y0, x0, y1, x1;
    if (!sel_range(&y0, &x0, &y1, &x1)) { E.sel = false; return false; }
    delete_range(y0, x0, y1, x1);
    E.cy = y0;
    E.cx = x0;
    E.sel = false;
    return true;
}

/* ---------------------------------------------------------------- status */

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.status, sizeof E.status, fmt, ap);
    va_end(ap);
    E.status_color = color;
    E.status_until = frame_count + 4 * FPS;
    E.status_open = false;
    if (opt.headless) printf("[editor] %s\n", E.status);
}

/* where the status text starts in the bottom bar, and whether it is cut off there */
#define STATUS_END (UI_W - STATUS_CLOSE_W - 2)     /* it ends just left of its close button */
static bool status_shown(void) { return E.status[0] && frame_count < E.status_until; }
static int status_x(void) {
    int sxp = STATUS_END - text_width(E.status);
    return sxp < 150 ? 150 : sxp;
}
static bool status_cut(void) { return status_x() + text_width(E.status) > STATUS_END; }

/* splits the status into lines of at most cols characters, breaking at spaces
 * where it can. Returns how many lines; starts[i]/lens[i] give each one. */
static int status_wrap(int cols, int max, int *starts, int *lens) {
    int n = 0, i = 0, len = (int)strlen(E.status);
    while (i < len && n < max) {
        int end = i + cols < len ? i + cols : len;
        const char *nl = memchr(E.status + i, '\n', (size_t)(end - i));
        if (nl) end = (int)(nl - E.status);
        else if (end < len) {
            int b = end;
            while (b > i && E.status[b] != ' ') b--;
            if (b > i) end = b;
        }
        starts[n] = i; lens[n] = end - i; n++;
        i = end;
        while (i < len && (E.status[i] == ' ' || E.status[i] == '\n')) i++;
    }
    return n;
}

/* ---------------------------------------------------------------- files */


static bool read_file_text(const char *path, char **out) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)n + 1);
    size_t got = fread(buf, 1, (size_t)n, f);
    buf[got] = 0;
    fclose(f);
    *out = buf;
    return true;
}

static const char *file_of(const EState *e) {
    const char *s = strrchr(e->path, '/');
    return s ? s + 1 : e->path;
}

const char *editor_file(void) { return E.path[0] ? file_of(&E) : ""; }

/* a code file's name without its .gb (a few at a time can be in use, as in "renamed a to b") */
const char *code_name(const char *file) {
    static char buf[4][64];
    static int k;
    char *out = buf[k++ & 3];
    size_t n = strlen(file);
    if (n > 3 && !strcmp(file + n - 3, ".gb")) n -= 3;
    snprintf(out, 64, "%.*s", (int)n, file);
    return out;
}

/* loads a file of the cart into a fresh buffer (an empty one if it isn't on disk yet) */
static void load_into_E(const char *name) {
    memset(&E, 0, sizeof E);
    char path[PATH_MAX + 80];
    snprintf(path, sizeof path, "%s/%s", cart.dir, name);
    char *text = NULL;
    bool exists = read_file_text(path, &text);
    if (!exists) text = strdup("");
    set_text(text);
    free(text);
    snprintf(E.path, sizeof E.path, "%s", path);
    E.modified = !exists;          /* a new file: it gets written by the next save */
    E.last_kind = -1;
}

static void free_state(EState *e) {
    EState keep = E;
    E = *e;
    clear_buffer();
    free(E.lines);
    free(E.rem_at);
    free_snaps(E.undo, &E.nundo);
    free_snaps(E.redo, &E.nredo);
    *e = E;
    E = keep;
}

/* makes sure the editor shows a file of the loaded cart (main.gb at first) */
bool editor_open(void) {
    if (!cart.loaded) return false;
    if (E.path[0]) return true;
    load_into_E("main.gb");
    return true;
}

bool editor_open_file(const char *name) {
    if (!cart.loaded) return false;
    size_t len = strlen(name);
    if (len < 4 || strcmp(name + len - 3, ".gb") || strchr(name, '/') || len > 64) return false;
    if (E.path[0] && !strcmp(file_of(&E), name)) return true;
    if (E.path[0]) {                       /* park the file being edited */
        if (nparked == MAX_FILES) {        /* forget the oldest unchanged one */
            int k = 0;
            while (k < nparked && parked[k].modified) k++;
            if (k == nparked) { status(COL_RED, "too many changed files open: save first"); return false; }
            free_state(&parked[k]);
            memmove(parked + k, parked + k + 1, sizeof parked[0] * (size_t)(nparked - k - 1));
            nparked--;
        }
        parked[nparked++] = E;
        memset(&E, 0, sizeof E);
    }
    for (int i = 0; i < nparked; i++)
        if (!strcmp(file_of(&parked[i]), name)) {
            E = parked[i];
            memmove(parked + i, parked + i + 1, sizeof parked[0] * (size_t)(nparked - i - 1));
            nparked--;
            E.rem_dirty = true;
            return true;
        }
    load_into_E(name);
    return true;
}

/* every .gb file with a buffer (the one being edited, then the parked ones),
 * on disk or not: a new file isn't written until the next save */
int editor_open_files(const char *names[], int max) {
    int n = 0;
    if (E.path[0] && n < max) names[n++] = file_of(&E);
    for (int i = 0; i < nparked && n < max; i++)
        if (parked[i].path[0]) names[n++] = file_of(&parked[i]);
    return n;
}

bool editor_file_modified(const char *name) {
    if (E.path[0] && !strcmp(file_of(&E), name)) return E.modified;
    for (int i = 0; i < nparked; i++)
        if (!strcmp(file_of(&parked[i]), name)) return parked[i].modified;
    return false;
}

bool editor_rename_file(const char *name, const char *to) {
    for (int i = -1; i < nparked; i++) {
        EState *e = i < 0 ? &E : &parked[i];
        if (!e->path[0] || strcmp(file_of(e), name)) continue;
        snprintf(e->path, sizeof e->path, "%s/%s", cart.dir, to);
        return true;
    }
    return false;
}

void editor_close_file(const char *name) {
    for (int i = 0; i < nparked; i++)
        if (!strcmp(file_of(&parked[i]), name)) {
            free_state(&parked[i]);
            memmove(parked + i, parked + i + 1, sizeof parked[0] * (size_t)(nparked - i - 1));
            nparked--;
            return;
        }
    if (!E.path[0] || strcmp(file_of(&E), name)) return;
    clear_buffer();
    free(E.lines);
    free(E.rem_at);
    free_snaps(E.undo, &E.nundo);
    free_snaps(E.redo, &E.nredo);
    memset(&E, 0, sizeof E);
    if (nparked) E = parked[--nparked];        /* show another open file (else main.gb next time) */
    E.rem_dirty = true;
}

/* the cart on disk changed under us (LOAD again): forget every buffer */
void editor_forget(void) {
    for (int i = 0; i < nparked; i++) free_state(&parked[i]);
    nparked = 0;
    clear_buffer();
    free(E.lines);
    free(E.rem_at);
    free_snaps(E.undo, &E.nundo);
    free_snaps(E.redo, &E.nredo);
    memset(&E, 0, sizeof E);
}

static bool write_state(const EState *e, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = true;
    for (int i = 0; i < e->n; i++) {
        /* trailing spaces are noise in diffs (the buffer keeps them, so the cursor doesn't jump) */
        int len = e->lines[i].len;
        while (len > 0 && e->lines[i].s[len - 1] == ' ') len--;
        if (fwrite(e->lines[i].s, 1, (size_t)len, f) != (size_t)len || fputc('\n', f) == EOF) ok = false;
    }
    if (fclose(f) != 0) ok = false;
    return ok;
}

bool editor_modified(void) {
    if (E.modified) return true;
    for (int i = 0; i < nparked; i++) if (parked[i].modified) return true;
    return false;
}

/* writes every changed file; `keep` leaves them counted as unsaved (a run of the untitled cart) */
static bool write_all(bool keep) {
    int saved = 0;
    const char *one = "";
    for (int i = -1; i < nparked; i++) {
        EState *e = i < 0 ? &E : &parked[i];
        if (!e->path[0] || !e->modified) continue;
        if (!write_state(e, e->path)) { status(COL_RED, "could not write %s", e->path); return false; }
        if (!keep) e->modified = false;
        saved++;
        one = file_of(e);
    }
    if (!keep && saved == 1) status(COL_GREEN, "saved %s (%d lines)", code_name(one), E.n);
    else if (!keep && saved > 1) status(COL_GREEN, "saved %d files", saved);
    return true;
}

bool editor_save(void) {
    if (!E.path[0]) return true;
    if (cart.untitled) { status(COL_ORANGE, "untitled cart: type SAVE name in the shell to keep it"); return false; }
    return write_all(false);
}

/* RUN on the untitled cart: the compiler reads the files, so write them to the
 * scratch folder, but they still aren't saved */
bool editor_flush(void) { return write_all(true); }

/* SAVE name moved the cart to a new folder: the buffers go with it */
void editor_follow_cart(void) {
    for (int i = -1; i < nparked; i++) {
        EState *e = i < 0 ? &E : &parked[i];
        if (!e->path[0]) continue;
        char name[80];
        snprintf(name, sizeof name, "%s", file_of(e));
        snprintf(e->path, sizeof e->path, "%s/%s", cart.dir, name);
    }
}

/* the window is closing with unsaved work: keep it next to the cart */
void editor_emergency_save(void) {
    for (int i = -1; i < nparked; i++) {
        EState *e = i < 0 ? &E : &parked[i];
        if (!e->path[0] || !e->modified) continue;
        char p[PATH_MAX + 16];
        snprintf(p, sizeof p, "%s.unsaved", e->path);
        if (write_state(e, p)) fprintf(stderr, "gooberengine: unsaved code kept in %s\n", p);
    }
}

void editor_show_error(int line, const char *msg) {
    if (line < 1 || line > E.n) return;
    E.err_line = line;
    E.cy = line - 1;
    E.cx = 0;
    while (E.cx < E.lines[E.cy].len && E.lines[E.cy].s[E.cx] == ' ') E.cx++;
    E.sel = false;
    E.follow = true;
    status(COL_RED, "line %d: %s", line, msg);
    E.status_until = LONG_MAX;     /* stays until the next edit */
}

/* ---------------------------------------------------------------- syntax colors */

static bool is_ident(int c) { return isalnum(c) || c == '_'; }

static bool is_const_name(const char *lower) {
    if (!strcmp(lower, "true") || !strcmp(lower, "false") || !strcmp(lower, "pi")) return true;
    for (int i = 0; gb_gfx_consts[i].name; i++)
        if (!strcasecmp(gb_gfx_consts[i].name, lower)) return true;
    return false;
}

static int word_color(const char *lower) {
    if (lex_is_keyword(lower)) return C_KEYWORD;
    /* `list<int>`, `new list<t_enemy>`: the lexer reads list as a name, the compiler knows it */
    if (!strcmp(lower, "list")) return C_KEYWORD;
    int b = compiler_builtin_word(lower);
    if (b == 1) return C_TYPE;
    if (b == 2) return C_COMMAND;
    if (lower[0] != '_' && native_find(lower) >= 0) return C_COMMAND;
    if (is_const_name(lower)) return C_CONST;
    return C_TEXT;
}

static bool word_is(const char *s, int n, int i, const char *w) {
    int wl = (int)strlen(w);
    if (i + wl > n || strncasecmp(s + i, w, (size_t)wl)) return false;
    if (i > 0 && is_ident((unsigned char)s[i - 1])) return false;
    return i + wl == n || !is_ident((unsigned char)s[i + wl]);
}

/* colors one line; col may be NULL to only track the rem-block state */
static void color_line(const char *s, int n, uint8_t *col, bool *rem) {
#define PAINT(a, b, c) do { if (col) for (int k_ = (a); k_ < (b); k_++) col[k_] = (uint8_t)(c); } while (0)
    int i = 0;
    while (i < n) {
        if (*rem) {
            int j = i;
            while (j < n && !word_is(s, n, j, "endrem")) j++;
            if (j >= n) { PAINT(i, n, C_COMMENT); return; }
            PAINT(i, j + 6, C_COMMENT);
            i = j + 6;
            *rem = false;
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        if (c == '\'') { PAINT(i, n, C_COMMENT); return; }
        if (c == '"') {
            int j = i + 1;
            while (j < n && s[j] != '"') j++;
            if (j < n) j++;
            PAINT(i, j, C_STRING);
            i = j;
            continue;
        }
        bool after_ident = i > 0 && (is_ident((unsigned char)s[i - 1]) || s[i - 1] == ')' || s[i - 1] == ']');
        if (isdigit(c) || (c == '.' && i + 1 < n && isdigit((unsigned char)s[i + 1]) && !after_ident) ||
            (c == '$' && i + 1 < n && isxdigit((unsigned char)s[i + 1])) ||
            (c == '%' && i + 1 < n && (s[i + 1] == '0' || s[i + 1] == '1') && !after_ident)) {
            int j = i + 1;
            while (j < n && (isalnum((unsigned char)s[j]) || (s[j] == '.' && !(j + 1 < n && s[j + 1] == '.')))) j++;
            PAINT(i, j, C_NUMBER);
            i = j;
            continue;
        }
        if (isalpha(c) || c == '_') {
            int j = i;
            while (j < n && is_ident((unsigned char)s[j])) j++;
            if (col || (j - i == 3)) {
                char lower[64];
                int wl = j - i < 63 ? j - i : 63;
                for (int k = 0; k < wl; k++) lower[k] = (char)tolower((unsigned char)s[i + k]);
                lower[wl] = 0;
                if (!strcmp(lower, "rem")) {
                    PAINT(i, j, C_COMMENT);
                    *rem = true;
                    i = j;
                    continue;
                }
                if (col) {
                    if (j < n && (s[j] == '%' || s[j] == '#' || s[j] == '$')) j++;    /* type suffix */
                    PAINT(i, j, word_color(lower));
                }
            }
            i = j;
            continue;
        }
        PAINT(i, i + 1, c == ' ' ? C_TEXT : C_PUNCT);
        i++;
    }
#undef PAINT
}

void editor_color_code(const char *s, int n, uint8_t *col) {
    bool rem = false;
    color_line(s, n, col, &rem);
}

static void update_rem_state(void);

/* F1: what the cursor is on, as a word the help can look up. A name gives
 * itself (lower case, without its type suffix); a number, string or comment
 * gives the page about those; punctuation gives "operators" (or the page for
 * what it does). The cursor sits between characters, so a name just typed
 * (cursor right after it) counts too. */
bool editor_word_at_cursor(char *out, size_t n) {
    if (!E.lines || E.n == 0 || n < 2) return false;
    update_rem_state();
    const Line *l = &E.lines[E.cy];
    const char *s = l->s;
    int len = l->len, x = E.cx;
    static uint8_t cols[4096];
    if (len > (int)sizeof cols) len = (int)sizeof cols;
    if (x > len) x = len;
    bool rem = E.rem_at[E.cy];
    color_line(s, len, cols, &rem);
    /* the character to look at: a name's character at or just before the cursor wins, then any non-space */
    #define IDENT_AT(i) ((i) >= 0 && (i) < len && (is_ident((unsigned char)s[i]) || \
                         (((s[i] == '%' || s[i] == '#' || s[i] == '$') && (i) > 0 && is_ident((unsigned char)s[(i) - 1])) && cols[i] != C_NUMBER)))
    int at = -1;
    if (IDENT_AT(x)) at = x;
    else if (IDENT_AT(x - 1)) at = x - 1;
    else if (x < len && s[x] != ' ') at = x;
    else if (x > 0 && x - 1 < len && s[x - 1] != ' ') at = x - 1;
    if (at < 0) return false;
    int c = cols[at];
    if (c == C_COMMENT) { snprintf(out, n, "comments"); return true; }
    if (c == C_STRING) { snprintf(out, n, "strings"); return true; }
    if (c == C_NUMBER) { snprintf(out, n, "numbers"); return true; }
    if (is_ident((unsigned char)s[at]) || s[at] == '%' || s[at] == '#' || s[at] == '$') {
        int a = at, b = at;
        if (!is_ident((unsigned char)s[a])) a--;              /* on the suffix: the name before it */
        while (a > 0 && is_ident((unsigned char)s[a - 1])) a--;
        while (b < len && is_ident((unsigned char)s[b])) b++;
        int k = 0;
        for (int i = a; i < b && k < (int)n - 1; i++) out[k++] = (char)tolower((unsigned char)s[i]);
        out[k] = 0;
        return k > 0;
    }
    char ch = s[at], next = at + 1 < len ? s[at + 1] : 0, prev = at > 0 ? s[at - 1] : 0;
    const char *w = "operators";
    if (ch == ':' && !strchr("+-*/", next)) w = (next == ' ' || !next) ? "basics" : "types";  /* separator / x:int */
    else if (ch == ';') w = "basics";
    else if (ch == '.') w = (next == '.' || prev == '.') ? "basics" : "type";              /* .. continues; a.b is a field */
    else if (ch == '[' || ch == ']') w = "arrays";
    else if (ch == '(' || ch == ')' || ch == ',') w = "functions";
    snprintf(out, n, "%s", w);
    return true;
    #undef IDENT_AT
}

static void update_rem_state(void) {
    if (!E.rem_dirty) return;
    if (E.rem_cap < E.n + 1) {
        E.rem_cap = E.n + 64;
        E.rem_at = realloc(E.rem_at, sizeof(bool) * (size_t)E.rem_cap);
    }
    bool rem = false;
    for (int y = 0; y < E.n; y++) {
        E.rem_at[y] = rem;
        color_line(E.lines[y].s, E.lines[y].len, NULL, &rem);
    }
    E.rem_dirty = false;
}

/* ---------------------------------------------------------------- editing commands */

static int indent_of(const Line *l) {
    int i = 0;
    while (i < l->len && l->s[i] == ' ') i++;
    return i;
}

/* does this line open a block, so the next line should be indented? */
static bool opens_block(const Line *l) {
    char first[16] = {0};
    int i = indent_of(l), k = 0;
    while (i < l->len && is_ident((unsigned char)l->s[i]) && k < 15) first[k++] = (char)tolower((unsigned char)l->s[i++]);
    static const char *const openers[] = {"function", "while", "for", "repeat", "select", "type",
                                          "else", "elseif", "case", "default", NULL};
    bool opener = false;
    for (int j = 0; openers[j]; j++) if (!strcmp(first, openers[j])) opener = true;
    if (!opener && strcmp(first, "if")) return false;
    /* line ends are only spaces in GooberBASIC, so a whole block can sit on
     * one line (for ... next, if ... endif): one that ends with its closer opens nothing */
    int end = l->len, q = 0;
    for (int j = 0; j < l->len; j++) {
        if (l->s[j] == '"') q = !q;
        else if (l->s[j] == '\'' && !q) { end = j; break; }
    }
    while (end > 0 && (l->s[end - 1] == ' ' || l->s[end - 1] == ':' || l->s[end - 1] == ';')) end--;
    int w = end;
    while (w > 0 && is_ident((unsigned char)l->s[w - 1])) w--;
    if (w > indent_of(l)) {
        static const char *const closers[] = {"endif", "next", "wend", "endwhile", "forever", "endfunction",
                                              "endselect", "endtype", NULL};
        for (int j = 0; closers[j]; j++) if (word_is(l->s, end, w, closers[j])) return false;
    }
    if (opener) return true;
    /* an if opens a block unless something follows its then */
    for (int j = 0; j + 4 <= l->len; j++)
        if (word_is(l->s, l->len, j, "then")) {
            int t = j + 4;
            while (t < l->len && l->s[t] == ' ') t++;
            return t >= l->len || l->s[t] == '\'';
        }
    return true;
}

/* Enter. The indentation of the new line follows settings.indent:
 *   smart  keep this line's, and one more after a line that opens a block (if, for ...)
 *   auto   keep this line's
 *   none   none */
static void newline(void) {
    before_edit(EDIT_OTHER);
    delete_selection();
    Line *l = &E.lines[E.cy];
    int ind = settings.indent == INDENT_NONE ? 0 : indent_of(l);
    if (ind > E.cx) ind = E.cx;
    bool more = settings.indent == INDENT_SMART && E.cx == l->len && opens_block(l);
    insert_text("\n");
    char sp[128];
    int n = ind + (more ? INDENT : 0);
    if (n > (int)sizeof sp - 1) n = sizeof sp - 1;
    memset(sp, ' ', (size_t)n);
    sp[n] = 0;
    insert_text(sp);
    E.fresh_newline = true;
}

static void backspace(void) {
    int y0, x0, y1, x1;
    if (sel_range(&y0, &x0, &y1, &x1)) { before_edit(EDIT_OTHER); delete_selection(); return; }
    if (E.cx == 0 && E.cy == 0) return;
    before_edit(EDIT_DELETE);
    /* auto indent: straight after Enter, Backspace takes the whole newline back, indentation and all */
    if (settings.indent == INDENT_AUTO && E.fresh_newline && E.cy > 0 && E.cx == indent_of(&E.lines[E.cy])) {
        int len = E.lines[E.cy - 1].len;
        delete_range(E.cy - 1, len, E.cy, E.cx);
        E.cy--;
        E.cx = len;
        return;
    }
    if (E.cx == 0) {
        int len = E.lines[E.cy - 1].len;
        delete_range(E.cy - 1, len, E.cy, 0);
        E.cy--;
        E.cx = len;
        return;
    }
    /* smart indent, inside the indentation: back up to the previous indent stop */
    int n = 1;
    if (settings.indent == INDENT_SMART && E.cx <= indent_of(&E.lines[E.cy])) n = (E.cx - 1) % INDENT + 1;
    delete_range(E.cy, E.cx - n, E.cy, E.cx);
    E.cx -= n;
}

static void delete_forward(void) {
    int y0, x0, y1, x1;
    if (sel_range(&y0, &x0, &y1, &x1)) { before_edit(EDIT_OTHER); delete_selection(); return; }
    Line *l = &E.lines[E.cy];
    if (E.cx == l->len && E.cy == E.n - 1) return;
    before_edit(EDIT_DELETE);
    if (E.cx < l->len) delete_range(E.cy, E.cx, E.cy, E.cx + 1);
    else delete_range(E.cy, E.cx, E.cy + 1, 0);
}

static void selected_lines(int *y0, int *y1) {
    int a, b, c, d;
    if (sel_range(&a, &b, &c, &d)) {
        *y0 = a;
        *y1 = (d == 0 && c > a) ? c - 1 : c;
    } else *y0 = *y1 = E.cy;
}

static void indent_lines(bool out) {
    int y0, y1;
    selected_lines(&y0, &y1);
    before_edit(EDIT_OTHER);
    for (int y = y0; y <= y1; y++) {
        Line *l = &E.lines[y];
        if (out) {
            int n = indent_of(l) < INDENT ? indent_of(l) : INDENT;
            line_delete(l, 0, n);
            if (y == E.cy) E.cx = E.cx > n ? E.cx - n : 0;
            if (y == E.ay && E.sel) E.ax = E.ax > n ? E.ax - n : 0;
        } else if (l->len > 0) {
            line_insert(l, 0, "        ", INDENT);  /* any run of >= INDENT spaces */
            if (y == E.cy) E.cx += INDENT;
            if (y == E.ay && E.sel) E.ax += INDENT;
        }
    }
}

static void tab(bool shift) {
    int y0, x0, y1, x1;
    if (shift || (sel_range(&y0, &x0, &y1, &x1) && y0 != y1)) { indent_lines(shift); return; }
    before_edit(EDIT_TYPE);
    delete_selection();
    char sp[INDENT + 1];
    int n = INDENT - E.cx % INDENT;
    memset(sp, ' ', n);
    sp[n] = 0;
    insert_text(sp);
}

static void toggle_comment(void) {
    int y0, y1;
    selected_lines(&y0, &y1);
    bool all = true;
    for (int y = y0; y <= y1; y++) {
        Line *l = &E.lines[y];
        int i = indent_of(l);
        if (i < l->len && l->s[i] != '\'') all = false;
    }
    before_edit(EDIT_OTHER);
    for (int y = y0; y <= y1; y++) {
        Line *l = &E.lines[y];
        int i = indent_of(l);
        if (i >= l->len) continue;
        if (all) {
            int n = (i + 1 < l->len && l->s[i + 1] == ' ') ? 2 : 1;
            line_delete(l, i, n);
            if (y == E.cy && E.cx > i) E.cx = E.cx - n > i ? E.cx - n : i;
        } else {
            line_insert(l, i, "' ", 2);
            if (y == E.cy && E.cx >= i) E.cx += 2;
        }
    }
    E.sel = false;
}

static void duplicate_line(void) {
    before_edit(EDIT_OTHER);
    Line *l = &E.lines[E.cy];
    lines_insert(E.cy + 1, l->s, l->len);
    E.cy++;
    E.sel = false;
}

static void copy(bool cut) {
    int y0, x0, y1, x1;
    char *t;
    if (sel_range(&y0, &x0, &y1, &x1)) t = get_range(y0, x0, y1, x1);
    else {   /* no selection: the whole line */
        Line *l = &E.lines[E.cy];
        t = malloc((size_t)l->len + 2);
        memcpy(t, l->s, (size_t)l->len);
        t[l->len] = '\n';
        t[l->len + 1] = 0;
    }
    clipboard_set(t);
    free(t);
    if (!cut) return;
    before_edit(EDIT_OTHER);
    if (!delete_selection()) {
        if (E.n > 1) lines_remove(E.cy);
        else line_set(&E.lines[0], "", 0);
        if (E.cy >= E.n) E.cy = E.n - 1;
        E.cx = 0;
    }
}

static void paste(void) {
    char *t = clipboard_get();
    if (*t) {
        before_edit(EDIT_OTHER);
        delete_selection();
        insert_text(t);
    }
    free(t);
}

/* ---------------------------------------------------------------- movement */

static void clamp_cursor(void) {
    if (E.cy < 0) E.cy = 0;
    if (E.cy >= E.n) E.cy = E.n - 1;
    if (E.cx < 0) E.cx = 0;
    if (E.cx > E.lines[E.cy].len) E.cx = E.lines[E.cy].len;
    /* the selection anchor too, so a selection never reaches past the text */
    if (E.ay < 0) E.ay = 0;
    if (E.ay >= E.n) E.ay = E.n - 1;
    if (E.ax < 0) E.ax = 0;
    if (E.ax > E.lines[E.ay].len) E.ax = E.lines[E.ay].len;
}

static void move_word(int dir) {
    Line *l = &E.lines[E.cy];
    if (dir < 0) {
        if (E.cx == 0) { if (E.cy > 0) { E.cy--; E.cx = E.lines[E.cy].len; } return; }
        while (E.cx > 0 && !is_ident((unsigned char)l->s[E.cx - 1])) E.cx--;
        while (E.cx > 0 && is_ident((unsigned char)l->s[E.cx - 1])) E.cx--;
    } else {
        if (E.cx == l->len) { if (E.cy < E.n - 1) { E.cy++; E.cx = 0; } return; }
        while (E.cx < l->len && is_ident((unsigned char)l->s[E.cx])) E.cx++;
        while (E.cx < l->len && !is_ident((unsigned char)l->s[E.cx])) E.cx++;
    }
}

/* moves the cursor; with shift held the selection follows it */
static void move(int sc, int mod) {
    bool shift = mod & MOD_SHIFT, ctrl = mod & MOD_CTRL;
    if (shift && !E.sel) { E.sel = true; E.ay = E.cy; E.ax = E.cx; }
    int y0, x0, y1, x1;
    bool had = sel_range(&y0, &x0, &y1, &x1);
    bool vertical = false;
    switch (sc) {
        case SDL_SCANCODE_LEFT:
            if (had && !shift) { E.cy = y0; E.cx = x0; break; }
            if (ctrl) move_word(-1);
            else if (E.cx > 0) E.cx--;
            else if (E.cy > 0) { E.cy--; E.cx = E.lines[E.cy].len; }
            break;
        case SDL_SCANCODE_RIGHT:
            if (had && !shift) { E.cy = y1; E.cx = x1; break; }
            if (ctrl) move_word(1);
            else if (E.cx < E.lines[E.cy].len) E.cx++;
            else if (E.cy < E.n - 1) { E.cy++; E.cx = 0; }
            break;
        case SDL_SCANCODE_UP: E.cy--; vertical = true; break;
        case SDL_SCANCODE_DOWN: E.cy++; vertical = true; break;
        case SDL_SCANCODE_PAGEUP: E.cy -= ROWS - 1; E.top -= ROWS - 1; vertical = true; break;
        case SDL_SCANCODE_PAGEDOWN: E.cy += ROWS - 1; E.top += ROWS - 1; vertical = true; break;
        case SDL_SCANCODE_HOME: {
            if (ctrl) { E.cy = 0; E.cx = 0; break; }
            int ind = indent_of(&E.lines[E.cy]);
            E.cx = E.cx == ind ? 0 : ind;     /* first press: start of the code; again: column 0 */
            break;
        }
        case SDL_SCANCODE_END:
            if (ctrl) E.cy = E.n - 1;
            E.cx = E.lines[E.cy].len;
            break;
    }
    if (vertical) {
        if (E.cy < 0) { E.cy = 0; E.want_x = 0; }
        if (E.cy >= E.n) { E.cy = E.n - 1; E.want_x = E.lines[E.cy].len; }
        E.cx = E.want_x;
    }
    clamp_cursor();
    if (!vertical) E.want_x = E.cx;
    if (!shift) E.sel = false;
    E.last_kind = -1;
    E.follow = true;
}

static void scroll_to_cursor(void) {
    if (E.cy < E.top) E.top = E.cy;
    if (E.cy >= E.top + ROWS) E.top = E.cy - ROWS + 1;
    if (E.cx < E.left) E.left = E.cx > 8 ? E.cx - 8 : 0;
    if (E.cx >= E.left + COLS) E.left = E.cx - COLS + 8;
}

static void clamp_scroll(void) {
    int max_top = E.n - ROWS / 2;
    if (E.top > max_top) E.top = max_top;
    if (E.top < 0) E.top = 0;
}

/* ---------------------------------------------------------------- find / go to */

static bool find_next(const char *what, bool from_start) {
    int wl = (int)strlen(what);
    if (!wl) return false;
    int sy = from_start ? 0 : E.cy, sx = from_start ? 0 : E.cx;
    for (int pass = 0; pass <= E.n; pass++) {
        int y = (sy + pass) % E.n;
        Line *l = &E.lines[y];
        for (int x = pass == 0 ? sx : 0; x + wl <= l->len; x++)
            if (!strncasecmp(l->s + x, what, (size_t)wl)) {
                E.sel = true;
                E.ay = y; E.ax = x;
                E.cy = y; E.cx = x + wl;
                E.want_x = E.cx;
                E.follow = true;
                return true;
            }
    }
    return false;
}

static void run_prompt(void) {
    if (E.prompt == 1) {
        snprintf(E.find_text, sizeof E.find_text, "%s", E.prompt_text);
        if (!find_next(E.find_text, false)) status(COL_ORANGE, "'%s' not found", E.find_text);
    } else if (E.prompt == 2) {
        int line = atoi(E.prompt_text);
        if (line >= 1 && line <= E.n) {
            E.cy = line - 1;
            E.cx = indent_of(&E.lines[E.cy]);
            E.sel = false;
            E.follow = true;
        }
    }
    E.prompt = 0;
}

/* ---------------------------------------------------------------- input */

static void key(const KeyEvent *ev) {
    int sc = ev->sc, mod = ev->mod;
    bool ctrl = mod & MOD_CTRL, shift = mod & MOD_SHIFT;
    E.blink_from = frame_count;
    bool fresh = E.fresh_newline;
    E.fresh_newline = false;

    if (!E.prompt && ui_global_key(ev)) return;
    if (E.prompt) {
        int n = (int)strlen(E.prompt_text);
        if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_KP_ENTER) run_prompt();
        else if (sc == SDL_SCANCODE_ESCAPE) E.prompt = 0;
        else if (sc == SDL_SCANCODE_BACKSPACE && n > 0) E.prompt_text[n - 1] = 0;
        return;
    }
    if (ctrl) {
        switch (sc) {
            case SDL_SCANCODE_S: editor_save(); return;
            case SDL_SCANCODE_R: ui_run_from_editor(); return;
            case SDL_SCANCODE_Z: if (shift) restore(E.redo, &E.nredo, E.undo, &E.nundo);
                                 else restore(E.undo, &E.nundo, E.redo, &E.nredo);
                                 return;
            case SDL_SCANCODE_Y: restore(E.redo, &E.nredo, E.undo, &E.nundo); return;
            case SDL_SCANCODE_A:
                E.sel = true; E.ay = 0; E.ax = 0;
                E.cy = E.n - 1; E.cx = E.lines[E.cy].len;
                return;
            case SDL_SCANCODE_C: copy(false); return;
            case SDL_SCANCODE_X: copy(true); return;
            case SDL_SCANCODE_V: paste(); return;
            case SDL_SCANCODE_D: duplicate_line(); return;
            case SDL_SCANCODE_SLASH: toggle_comment(); return;
            case SDL_SCANCODE_F: E.prompt = 1; snprintf(E.prompt_text, sizeof E.prompt_text, "%s", E.find_text); return;
            case SDL_SCANCODE_G: E.prompt = 2; E.prompt_text[0] = 0; return;
            case SDL_SCANCODE_N:
                if (!find_next(E.find_text, false)) status(COL_ORANGE, "'%s' not found", E.find_text);
                return;
            default: break;
        }
    }
    switch (sc) {
        case SDL_SCANCODE_ESCAPE:
            if (E.status_open) { E.status_open = false; return; }
            if (E.sel) { E.sel = false; return; }
            ui_mode = MODE_SHELL;
            return;
        case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: newline(); return;
        case SDL_SCANCODE_BACKSPACE: E.fresh_newline = fresh; backspace(); E.fresh_newline = false; return;
        case SDL_SCANCODE_DELETE: delete_forward(); return;
        case SDL_SCANCODE_TAB: tab(shift); return;
        case SDL_SCANCODE_LEFT: case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_UP: case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_HOME: case SDL_SCANCODE_END: case SDL_SCANCODE_PAGEUP: case SDL_SCANCODE_PAGEDOWN:
            move(sc, mod);
            return;
        default: break;
    }
}

static void type_text(const char *t) {
    E.blink_from = frame_count;
    E.fresh_newline = false;
    if (E.prompt) {
        size_t n = strlen(E.prompt_text);
        for (; *t && n < sizeof E.prompt_text - 1; t++)
            if (*t >= 32 && *t < 127) E.prompt_text[n++] = *t;
        E.prompt_text[n] = 0;
        return;
    }
    char clean[16];
    int n = 0;
    for (; *t && n < 15; t++) if (*t >= 32 && *t < 127) clean[n++] = *t;
    clean[n] = 0;
    if (!n) return;
    int y0, x0, y1, x1;
    before_edit(sel_range(&y0, &x0, &y1, &x1) ? EDIT_OTHER : EDIT_TYPE);
    delete_selection();
    insert_text(clean);
    E.want_x = E.cx;
}

static bool mouse_to_pos(int *y, int *x) {
    int row = (in.mouse_y - TEXT_Y) / FONT_H;
    if (in.mouse_y < TEXT_Y) row = -1;
    *y = E.top + row;
    *x = E.left + (in.mouse_x - GUTTER + FONT_W / 2) / FONT_W;
    if (*y < 0) *y = 0;
    if (*y >= E.n) { *y = E.n - 1; *x = E.lines[*y].len; }
    if (*x < 0) *x = 0;
    if (*x > E.lines[*y].len) *x = E.lines[*y].len;
    return true;
}

/* the bounds [*a, *b) of the token on line y that holds column x: a string
 * (quotes included), a number, a name (with its type suffix), a run of
 * spaces, or one punctuation character. Inside a comment it picks the word. */
static void token_at(int y, int x, int *a, int *b) {
    const char *s = E.lines[y].s;
    int n = E.lines[y].len;
    *a = *b = x;
    if (n == 0) return;
    if (x >= n) x = n - 1;
    int i = 0;
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        int j = i + 1;
        if (c == '\'') break;                                  /* comment: fall back to words */
        if (c == '"') {
            while (j < n && s[j] != '"') j++;
            if (j < n) j++;
        } else if (isdigit(c) || ((c == '.' || c == '$' || c == '%') && j < n && isalnum((unsigned char)s[j]))) {
            while (j < n && (isalnum((unsigned char)s[j]) || (s[j] == '.' && !(j + 1 < n && s[j + 1] == '.')))) j++;
        } else if (is_ident(c)) {
            while (j < n && is_ident((unsigned char)s[j])) j++;
            if (j < n && (s[j] == '%' || s[j] == '#' || s[j] == '$')) j++;
        } else if (c == ' ' || c == '\t') {
            while (j < n && (s[j] == ' ' || s[j] == '\t')) j++;
        }
        if (x < j) { *a = i; *b = j; return; }
        i = j;
    }
    /* in a comment (or rem block): the word or run of non-word characters */
    bool w = is_ident((unsigned char)s[x]);
    *a = x; *b = x + 1;
    while (*a > 0 && is_ident((unsigned char)s[*a - 1]) == w) (*a)--;
    while (*b < n && is_ident((unsigned char)s[*b]) == w) (*b)++;
}

static void mouse(void) {
    static long last_click = -1000;
    static int clicks, click_y, click_mx;
    if (in.wheel) { E.top -= in.wheel * 3; clamp_scroll(); }
    /* the status panel: a click anywhere closes it; a click on the status text opens it */
    if (in.mouse_hit[1] && E.status_open) { E.status_open = false; E.dragging = false; return; }
    if (in.mouse_hit[1] && !E.prompt && in.mouse_y >= SCREEN_H - BAR_H && status_shown()
        && in.mouse_x >= status_x() && in.mouse_x < STATUS_END) { E.status_open = true; E.dragging = false; return; }
    bool in_text = in.mouse_y >= TEXT_Y && in.mouse_y < TEXT_Y + ROWS * FONT_H;
    int y, x;
    if (in.mouse_hit[1] && in_text) {
        mouse_to_pos(&y, &x);
        /* double / triple click: same row, near the same spot, within 0.4 s */
        int mx = in.mouse_x;
        if (frame_count - last_click <= FPS * 2 / 5 && y == click_y && abs(mx - click_mx) <= FONT_W) clicks++;
        else clicks = 1;
        last_click = frame_count; click_y = y; click_mx = mx;
        if (clicks >= 2) {
            int a, b;
            if (clicks == 2) {
                int cx = E.left + (mx - GUTTER) / FONT_W;    /* the character under the pointer, not the nearest gap */
                if (cx < 0) cx = 0;
                token_at(y, cx, &a, &b);
            } else { a = 0; b = E.lines[y].len; clicks = 0; }
            E.sel = true; E.ay = y; E.ax = a;
            E.cy = y; E.cx = E.want_x = b;
            E.dragging = false;
            E.last_kind = -1;
            E.blink_from = frame_count;
            return;
        }
        bool shift = in.key[SDL_SCANCODE_LSHIFT] || in.key[SDL_SCANCODE_RSHIFT];
        if (!shift || !E.sel) { E.sel = true; E.ay = shift ? E.cy : y; E.ax = shift ? E.cx : x; }
        E.cy = y; E.cx = E.want_x = x;
        E.dragging = true;
        E.fresh_newline = false;
        E.last_kind = -1;
        E.blink_from = frame_count;
        E.prompt = 0;
    } else if (E.dragging && in.mouse[1]) {
        mouse_to_pos(&y, &x);
        E.cy = y; E.cx = E.want_x = x;
        if (in.mouse_y < TEXT_Y && E.top > 0) E.top--;
        if (in.mouse_y >= TEXT_Y + ROWS * FONT_H) E.top++;
        /* pan sideways when dragged past the left or right edge */
        if (in.mouse_x < GUTTER && E.left > 0) E.left--;
        if (in.mouse_x >= UI_W - FONT_W && E.cx >= E.left + COLS - 1) E.left++;
        clamp_scroll();
    } else E.dragging = false;
}

/* ---------------------------------------------------------------- drawing */

static void draw(void) {
    draw_reset();
    draw_cls(C_BG);
    update_rem_state();

    int sy0 = 0, sx0 = 0, sy1 = 0, sx1 = 0;
    bool sel = sel_range(&sy0, &sx0, &sy1, &sx1);
    static uint8_t cols[4096];

    for (int r = 0; r < ROWS; r++) {
        int y = E.top + r;
        int py = TEXT_Y + r * FONT_H;
        if (y >= E.n) break;
        Line *l = &E.lines[y];
        int bg = y + 1 == E.err_line ? C_ERRLINE : y == E.cy ? C_CURLINE : -1;
        if (bg >= 0) draw_rect(GUTTER - 2, py, UI_W - GUTTER + 2, FONT_H, bg, true);

        char num[8];
        snprintf(num, sizeof num, "%4d", y + 1);
        draw_text(num, 0, py, y == E.cy ? C_TEXT : C_GUTTER);

        /* selection background (a selected line break shows as one cell) */
        if (sel && y >= sy0 && y <= sy1) {
            int a = y == sy0 ? sx0 : 0, b = y == sy1 ? sx1 : l->len + 1;
            a -= E.left; b -= E.left;
            if (a < 0) a = 0;
            if (b > COLS) b = COLS;
            if (b > a) draw_rect(GUTTER + a * FONT_W - 1, py, (b - a) * FONT_W, FONT_H, C_SEL, true);
        }

        bool rem = E.rem_at[y];
        int len = l->len < (int)sizeof cols ? l->len : (int)sizeof cols;
        color_line(l->s, len, cols, &rem);
        for (int c = 0; c < COLS; c++) {
            int x = E.left + c;
            if (x >= len) break;
            if (l->s[x] != ' ') draw_char((unsigned char)l->s[x], GUTTER + c * FONT_W, py, cols[x]);
        }
        if (E.left > 0 && l->len > 0) draw_char('<', GUTTER - 7, py, C_GUTTER);
    }
    draw_line(GUTTER - 3, TEXT_Y, GUTTER - 3, TEXT_Y + ROWS * FONT_H - 1, C_GUTTER);

    /* cursor */
    int crow = E.cy - E.top, ccol = E.cx - E.left;
    if (!E.prompt && crow >= 0 && crow < ROWS && ccol >= 0 && ccol <= COLS && ((frame_count - E.blink_from) / 30) % 2 == 0)
        draw_rect(GUTTER + ccol * FONT_W - 1, TEXT_Y + crow * FONT_H, 1, FONT_H, COL_YELLOW, true);

    /* top bar */
    int x = ui_draw_tabs();

    /* bottom bar */
    int by = SCREEN_H - BAR_H;
    draw_rect(0, by, UI_W, BAR_H, C_BAR, true);
    if (E.prompt) {
        x = draw_text(E.prompt == 1 ? "find: " : "go to line: ", 3, by, COL_YELLOW);
        x = draw_text(E.prompt_text, x, by, COL_WHITE);
        if ((frame_count / 30) % 2 == 0) draw_rect(x - 1, by, 1, FONT_H, COL_YELLOW, true);
        draw_text("Enter / Esc", UI_W - 3 - text_width("Enter / Esc"), by, UI_RGB(2, 3, 3));
        return;
    }
    char pos[64];
    snprintf(pos, sizeof pos, "%s%s  ln %d  col %d", code_name(file_of(&E)), E.modified ? "*" : "", E.cy + 1, E.cx + 1);
    draw_text(pos, 3, by, UI_RGB(2, 3, 3));
    if (!status_shown()) { E.status_open = false; return; }
    int sxp = status_x();
    if (!status_cut()) draw_text(E.status, sxp, by, E.status_color);
    else {
        /* cut off: show what fits, then "..." to say there is more */
        int dots = text_width("...");
        draw_clip(sxp, by, STATUS_END - dots - sxp, BAR_H);
        draw_text(E.status, sxp, by, E.status_color);
        draw_clip_reset();
        draw_text("...", STATUS_END - dots, by, COL_WHITE);
    }
    if (ui_status_close(by)) { E.status[0] = 0; E.status_open = false; return; }
    if (!E.status_open) return;

    /* the panel: the whole status, wrapped, just above the bottom bar */
    enum { MAX_PANEL = 12 };
    int starts[MAX_PANEL], lens[MAX_PANEL];
    int wrap_cols = (UI_W - 8) / FONT_W;
    int n = status_wrap(wrap_cols, MAX_PANEL, starts, lens);
    int ph = n * FONT_H + 4, py = by - ph;
    draw_rect(0, py, UI_W, ph, C_BG, true);
    draw_rect(0, py, UI_W, ph, E.status_color, false);
    char buf[512];
    for (int i = 0; i < n; i++) {
        snprintf(buf, sizeof buf, "%.*s", lens[i], E.status + starts[i]);
        draw_text(buf, 4, py + 2 + i * FONT_H, E.status_color);
    }
}

void editor_frame(void) {
    if (!editor_open()) { ui_mode = MODE_SHELL; return; }
    for (int i = 0; i < in.nevents && ui_mode == MODE_EDITOR; i++) {     /* stops if a key switches editor */
        const KeyEvent *ev = &in.events[i];
        if (ev->type == EV_KEY) key(ev);
        else type_text(ev->text);
    }
    if (ui_mode != MODE_EDITOR) return;
    mouse();
    clamp_cursor();
    if (E.follow) { scroll_to_cursor(); E.follow = false; }
    draw();
}
