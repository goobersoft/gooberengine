/* The manual, inside the console: F1 from anywhere.
 *
 * In the code editor F1 opens the page for the word under the cursor: a
 * keyword, a type, a command, a constant (or, for a number, string, comment
 * or operator, the page about those). In the shell it opens the index of every
 * page's names (narrowed to the command being typed, if there is one); in
 * the other editors, that editor's page.
 *
 * The pages themselves are in help_pages.c, written in a small markup:
 *   a line starting with 4 spaces    code: shown as it is, in syntax colours
 *   # Heading                        a heading
 *   - item                           a bullet (wrapped with a hanging indent)
 *   {name} or {name|text}            a link to the page for name
 *   `code`                           code inside a sentence
 *   anything else                    a paragraph, wrapped to the screen
 *
 * Keys: arrows / PgUp / PgDn / Home / End / the wheel scroll, Tab and
 * Shift+Tab pick a link, Enter (or a click) follows it, Backspace goes back,
 * typing searches every page's names, F1 shows the index (every page's
 * names, A to Z; F1 again goes back to the page), and Esc goes back to where
 * you were. {index} in a page links to the index too.
 *
 * The index is a place like any page: going from a page to the index, or from
 * the index to a page, can be gone back over (Backspace, Alt+Left) and forward
 * again (Alt+Right). The buttons on the top bar do the same with the mouse:
 * back, forward, contents, index, close. */
#include "engine.h"
#include "common.h"
#include "lexer.h"
#include "compiler.h"
#include "vm.h"
#include <SDL2/SDL_scancode.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct { const char *names, *title, *body; } HelpPage;
extern const HelpPage help_pages[];
extern const int help_npages;

#define BAR_H FONT_H
#define BODY_Y BAR_H
#define BODY_ROWS ((SCREEN_H - 2 * BAR_H) / FONT_H)
#define MARGIN 4
#define WIDTH ((SCREEN_W - 2 * MARGIN - 4) / FONT_W)   /* characters per line (room for the scroll bar) */

#define C_BG UI_RGB(0, 0, 0)
#define C_BAR UI_RGB(0, 1, 2)
#define C_BARTEXT UI_RGB(2, 3, 3)
#define C_TEXT RGB6(3, 3, 3)
#define C_HEAD RGB6(3, 3, 0)
#define C_LINK RGB6(1, 3, 3)
#define C_DIM RGB6(2, 2, 2)
#define C_SELBG UI_RGB(1, 2, 3)
#define C_CODEBG UI_RGB(0, 0, 1)        /* behind lines of code */
#define C_INLINE RGB6(3, 3, 2)         /* plain names in `code` inside a sentence */

/* ---------------------------------------------------------------- names */

/* the i-th name of a page's name list into out; false past the last */
static bool page_name(int p, int i, char *out, size_t n) {
    const char *s = help_pages[p].names;
    for (;;) {
        while (*s == ' ') s++;
        if (!*s) return false;
        const char *e = s;
        while (*e && *e != ' ') e++;
        if (i-- == 0) {
            snprintf(out, n, "%.*s", (int)(e - s), s);
            return true;
        }
        s = e;
    }
}

/* the page for a word: an exact name first, then the longest NAME* pattern that fits */
static int find_page(const char *word) {
    char w[64];
    snprintf(w, sizeof w, "%s", word);
    size_t wl = strlen(w);
    while (wl > 1 && (w[wl - 1] == '%' || w[wl - 1] == '#' || w[wl - 1] == '$')) w[--wl] = 0;   /* type suffix */
    if (!wl) return -1;
    int best = -1;
    size_t best_len = 0;
    char name[64];
    for (int p = 0; p < help_npages; p++)
        for (int i = 0; page_name(p, i, name, sizeof name); i++) {
            size_t nl = strlen(name);
            if (nl && name[nl - 1] == '*') {
                if (nl - 1 > best_len && wl >= nl - 1 && !strncasecmp(w, name, nl - 1)) { best = p; best_len = nl - 1; }
            } else if (!strcasecmp(w, name)) return p;
        }
    return best;
}

/* ---------------------------------------------------------------- layout */

/* a laid-out line: its characters, the colour of each, and which link each belongs to */
typedef struct { char t[WIDTH + 1]; uint8_t c[WIDTH]; int16_t link[WIDTH]; bool code; } DLine;
typedef struct { char target[48]; } Link;

/* a place in the help, to go back (or forward) to: a page, or the index */
typedef struct { int page, top, sel; bool index; char q[32]; int qsel, qtop; } Loc;
#define MAX_HIST 32

static struct {
    int back;                  /* the mode F1 was pressed in */
    int page, top, sel;        /* page, first row shown, selected link (-1 none) */
    Loc hist[MAX_HIST], fwd[MAX_HIST];   /* where Back and Forward go, the most recent last */
    int nhist, nfwd;
    bool search;               /* showing the index (narrowed to q if anything is typed) */
    char q[32];
    int qsel, qtop;
    char msg[128];
    int msg_color;
    long msg_until;
    DLine *lines;              /* the page, laid out */
    int nlines, cap;
    Link *links;
    int nlinks, link_cap;
    int laid;                  /* the page laid out, -1 = none */
} H = {.back = MODE_SHELL, .sel = -1, .laid = -1};

static DLine *new_line(void) {
    if (H.nlines == H.cap) {
        H.cap = H.cap ? H.cap * 2 : 128;
        H.lines = realloc(H.lines, sizeof *H.lines * (size_t)H.cap);
    }
    DLine *d = &H.lines[H.nlines++];
    memset(d->t, 0, sizeof d->t);
    memset(d->c, C_TEXT, sizeof d->c);
    for (int i = 0; i < WIDTH; i++) d->link[i] = -1;
    d->code = false;
    return d;
}

static int new_link(const char *target, int len) {
    if (H.nlinks == H.link_cap) {
        H.link_cap = H.link_cap ? H.link_cap * 2 : 64;
        H.links = realloc(H.links, sizeof *H.links * (size_t)H.link_cap);
    }
    snprintf(H.links[H.nlinks].target, sizeof H.links[0].target, "%.*s", len, target);
    return H.nlinks++;
}

/* a styled run of text before it is wrapped */
#define PARA_MAX 2048
static char pt[PARA_MAX];
static uint8_t pc[PARA_MAX];
static int16_t pl[PARA_MAX];
static bool pbreak[PARA_MAX];      /* a line may break at this space */
static int plen;

static void para_add(const char *s, int n, int colour, int link, bool breakable_spaces) {
    for (int i = 0; i < n && plen < PARA_MAX; i++, plen++) {
        pt[plen] = s[i];
        pc[plen] = (uint8_t)colour;
        pl[plen] = (int16_t)link;
        pbreak[plen] = breakable_spaces && s[i] == ' ';
    }
}

/* the inline markup of one source line: {links}, `code`, plain text */
static void para_parse(const char *s, int n, int colour) {
    plen = 0;
    int i = 0;
    while (i < n) {
        if (s[i] == '{') {
            const char *close = memchr(s + i, '}', (size_t)(n - i));
            if (close) {
                const char *in = s + i + 1;
                int inl = (int)(close - in);
                const char *bar = memchr(in, '|', (size_t)inl);
                int tl = bar ? (int)(bar - in) : inl;
                int id = new_link(in, tl);
                if (bar) para_add(bar + 1, inl - tl - 1, C_LINK, id, false);
                else para_add(in, tl, C_LINK, id, false);
                i = (int)(close - s) + 1;
                continue;
            }
        }
        if (s[i] == '`') {
            const char *close = memchr(s + i + 1, '`', (size_t)(n - i - 1));
            if (close) {
                int start = plen, cl = (int)(close - (s + i + 1));
                para_add(s + i + 1, cl, C_TEXT, -1, false);
                editor_colour_code(pt + start, plen - start, pc + start);
                for (int k = start; k < plen; k++) if (pc[k] == C_TEXT) pc[k] = C_INLINE;
                i = (int)(close - s) + 1;
                continue;
            }
        }
        para_add(s + i, 1, colour, -1, true);
        i++;
    }
}

/* wrap the run into lines `indent` in from the margin (`first` for the first line) */
static void para_wrap(int first, int indent) {
    int i = 0;
    bool firstline = true;
    while (i < plen) {
        int ind = firstline ? first : indent, room = WIDTH - ind;
        if (!firstline) while (i < plen && pt[i] == ' ' && pbreak[i]) i++;    /* no spaces at a line's start */
        if (i >= plen) break;
        int end = i + room;
        if (end >= plen) end = plen;
        else {
            int b = end;
            while (b > i && !(pbreak[b])) b--;                 /* the last space it may break at */
            if (b > i) end = b;
        }
        DLine *d = new_line();
        for (int k = i; k < end; k++) {
            d->t[ind + k - i] = pt[k];
            d->c[ind + k - i] = pc[k];
            d->link[ind + k - i] = pl[k];
        }
        for (int k = 0; k < ind; k++) d->t[k] = ' ';
        i = end;
        firstline = false;
    }
    if (plen == 0) new_line();
}

static void layout(int p) {
    H.nlines = 0;
    H.nlinks = 0;
    H.laid = p;
    const char *s = help_pages[p].body;
    while (*s) {
        const char *e = strchr(s, '\n');
        int n = e ? (int)(e - s) : (int)strlen(s);
        if (n >= 4 && !strncmp(s, "    ", 4)) {                  /* code: as it is, coloured like the editor */
            DLine *d = new_line();
            d->code = true;
            int cl = n - 4 > WIDTH - 2 ? WIDTH - 2 : n - 4;
            memset(d->t, ' ', 2);
            memcpy(d->t + 2, s + 4, (size_t)cl);
            editor_colour_code(d->t + 2, cl, d->c + 2);
        } else if (n >= 2 && s[0] == '#' && s[1] == ' ') {
            para_parse(s + 2, n - 2, C_HEAD);
            para_wrap(0, 0);
        } else if (n >= 2 && s[0] == '-' && s[1] == ' ') {
            para_parse(s, n, C_TEXT);
            pc[0] = C_DIM;
            para_wrap(0, 2);
        } else {
            para_parse(s, n, C_TEXT);
            para_wrap(0, 0);
        }
        s = e ? e + 1 : s + n;
    }
    while (H.nlines > 0 && !H.lines[H.nlines - 1].t[0]) H.nlines--;   /* no blank lines at the end */
}

/* ---------------------------------------------------------------- search */

static bool has_text(const char *s, const char *q) {     /* q in s, ignoring case (strcasestr isn't everywhere) */
    size_t n = strlen(q);
    for (; *s; s++) if (!strncasecmp(s, q, n)) return true;
    return !n;
}

typedef struct { char name[64]; int page, rank; } Hit;
static Hit hits[512];
static int nhits;

static int cmp_hit(const void *a, const void *b) {
    const Hit *x = a, *y = b;
    if (x->rank != y->rank) return x->rank - y->rank;
    return strcasecmp(x->name, y->name);
}

/* a word of the language (keyword, type, command, constant), as the code editor colours them */
static bool is_code_word(const char *name) {
    char lower[64];
    int k = 0;
    for (const char *c = name; *c && k < 63; c++) {
        if (!isalnum((unsigned char)*c) && *c != '_') return false;
        lower[k++] = (char)tolower((unsigned char)*c);
    }
    lower[k] = 0;
    if (lex_is_keyword(lower) || compiler_builtin_word(lower) || native_find(lower) >= 0) return true;
    /* words written in code that the lexer reads as plain names: constants, the list type, and
     * other BASICs' words that have a page saying what to use instead */
    static const char *const more[] = {"true", "false", "pi", "list", "break", "stop", "data",
        "after", "before", NULL};
    for (int i = 0; more[i]; i++) if (!strcmp(lower, more[i])) return true;
    const ConstDef *tables[] = {gb_gfx_consts, gb_net_consts};
    for (int t = 0; t < 2; t++)
        for (int i = 0; tables[t][i].name; i++) if (!strcasecmp(tables[t][i].name, lower)) return true;
    return false;
}

/* how a name reads in the index: code as it's written, a manual topic as words
 * (code-editor: "Code editor", or the page's title if it's that) */
static void index_name(int p, const char *name, char *out, size_t n) {
    snprintf(out, n, "%s", name);
    bool topic = true;
    for (const char *c = name; *c; c++) if (!(islower((unsigned char)*c) || isdigit((unsigned char)*c) || *c == '-')) topic = false;
    if (!topic || !islower((unsigned char)name[0]) || (!strchr(name, '-') && is_code_word(name))) return;
    for (char *c = out; *c; c++) if (*c == '-') *c = ' ';
    out[0] = (char)toupper((unsigned char)out[0]);
    if (!strcasecmp(out, help_pages[p].title)) snprintf(out, n, "%s", help_pages[p].title);
}

/* every name that holds the query: exact matches first, then names that start with it */
static void search_update(void) {
    nhits = 0;
    size_t ql = strlen(H.q);
    char name[64];
    for (int p = 0; p < help_npages; p++)
        for (int i = 0; page_name(p, i, name, sizeof name) && nhits < (int)(sizeof hits / sizeof hits[0]); i++) {
            const char *shown = name[0] == ':' && name[1] ? name + 1 : name;   /* :load is the shell's LOAD */
            if (!isalpha((unsigned char)shown[0])) continue;   /* ( + .. are only for F1 on an operator */
            char words[64];
            if (name[0] != ':') { index_name(p, name, words, sizeof words); shown = words; }
            int rank;
            if (!ql) rank = 2;
            else if (!strcasecmp(shown, H.q) || !strcasecmp(name, H.q)) rank = 0;
            else if (!strncasecmp(shown, H.q, ql) || !strncasecmp(name, H.q, ql)) rank = 1;
            else if (has_text(shown, H.q) || has_text(name, H.q) || has_text(help_pages[p].title, H.q)) rank = 2;
            else continue;
            Hit *h = &hits[nhits++];
            if (name[0] == ':') {
                int k = 0;
                for (const char *c = shown; *c && k < 40; c++) h->name[k++] = (char)toupper((unsigned char)*c);
                snprintf(h->name + k, sizeof h->name - (size_t)k, " (shell)");
            } else snprintf(h->name, sizeof h->name, "%s", shown);
            h->page = p;
            h->rank = rank;
        }
    qsort(hits, (size_t)nhits, sizeof hits[0], cmp_hit);
    H.qsel = 0;
    H.qtop = 0;
}

/* ---------------------------------------------------------------- opening and closing */

static void message(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void message(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(H.msg, sizeof H.msg, fmt, ap);
    va_end(ap);
    H.msg_color = color;
    H.msg_until = frame_count + 5 * FPS;
    if (opt.headless) printf("[help] %s\n", H.msg);
}

static void clamp_top(void);

static void enter_help(void) {
    if (ui_mode == MODE_HELP) return;
    H.back = ui_mode;
    H.nhist = H.nfwd = 0;
    H.msg[0] = 0;
    ui_mode = MODE_HELP;
}

/* ---- where you are, and going back and forward */

static Loc here(void) {
    Loc l = {H.page, H.top, H.sel, H.search, "", H.qsel, H.qtop};
    snprintf(l.q, sizeof l.q, "%s", H.q);
    return l;
}

static void push(Loc *stack, int *n, Loc l) {
    if (*n == MAX_HIST) { memmove(stack, stack + 1, sizeof *stack * (MAX_HIST - 1)); (*n)--; }
    stack[(*n)++] = l;
}

static void announce(void) {      /* tests read where the help went */
    if (!opt.headless) return;
    if (H.search) printf("[help] Index: %d names%s%s\n", nhits, nhits ? ", first " : "", nhits ? hits[H.qsel].name : "");
    else printf("[help] %s\n", help_pages[H.page].title);
}

static void go_to(Loc l) {
    H.page = l.page;
    layout(l.page);
    H.top = l.top;
    H.sel = l.sel < H.nlinks ? l.sel : -1;
    clamp_top();
    H.search = l.index;
    if (l.index) {
        snprintf(H.q, sizeof H.q, "%s", l.q);
        search_update();
        H.qsel = l.qsel < nhits ? l.qsel : 0;
        H.qtop = l.qtop;
    }
    announce();
}

/* leaving this place for another: it goes on the Back list, and Forward starts over */
static void leave_here(void) {
    if (ui_mode != MODE_HELP || H.laid < 0) return;
    push(H.hist, &H.nhist, here());
    H.nfwd = 0;
}

static void go_back(void) {
    if (!H.nhist) { help_close(); return; }      /* nothing before: back to where F1 was pressed */
    push(H.fwd, &H.nfwd, here());
    go_to(H.hist[--H.nhist]);
}

static void go_forward(void) {
    if (!H.nfwd) return;
    push(H.hist, &H.nhist, here());
    go_to(H.fwd[--H.nfwd]);
}

static void show_page(int p, bool remember) {
    if (remember) leave_here();
    enter_help();
    H.page = p;
    H.top = 0;
    H.sel = -1;
    H.search = false;
    layout(p);
    announce();
}

/* the index, narrowed to q; from outside the help it's the first place, with nothing to go back to */
static void open_index(const char *q) {
    if (ui_mode == MODE_HELP) leave_here();
    else {
        enter_help();
        H.page = find_page("contents");     /* something to stand on until a name is opened */
        layout(H.page);
        H.top = 0;
        H.sel = -1;
    }
    snprintf(H.q, sizeof H.q, "%s", q);
    H.search = true;
    search_update();
}

/* the index: every name of every page, A to Z (the search, with nothing typed yet) */
static bool is_index(const char *word) { return !strcasecmp(word, "index"); }

void help_index(void) {
    if (ui_mode == MODE_HELP && H.search) {
        if (!H.q[0]) { go_back(); return; }    /* F1 on the index: back where you came from */
        H.q[0] = 0;                            /* narrowed: the whole index again, in the same place */
        search_update();
    } else open_index("");
    announce();
}

bool help_open_word(const char *word, bool search) {
    if (is_index(word)) { help_index(); return true; }
    int p = find_page(word);
    if (p < 0 && search) {          /* HELP theme in the shell: the THEME command's page */
        char cmd[64];
        snprintf(cmd, sizeof cmd, ":%s", word);
        p = find_page(cmd);
    }
    if (p >= 0) { show_page(p, true); return true; }
    if (!search) return false;
    snprintf(H.q, sizeof H.q, "%s", word);
    search_update();
    if (!nhits) return false;
    open_index(word);
    announce();
    return true;
}

void help_close(void) {
    H.search = false;
    ui_mode = H.back == MODE_HELP ? MODE_SHELL : H.back;
    if (ui_mode != MODE_SHELL && ui_mode != MODE_SETTINGS && !cart.loaded) ui_mode = MODE_SHELL;
}

void help_open_context(void) {
    const char *page = "contents";
    switch (ui_mode) {
        case MODE_EDITOR: {
            char w[64];
            if (!editor_word_at_cursor(w, sizeof w)) { page = "code-editor"; break; }
            if (help_open_word(w, false)) return;
            /* not a word the console knows: a name of the cart's own, most likely */
            snprintf(H.q, sizeof H.q, "%s", w);
            search_update();
            if (nhits) { open_index(w); announce(); message(COL_ORANGE, "no page for '%s': pages with it in their names", w); }
            else { show_page(find_page("contents"), true); message(COL_ORANGE, "no page for '%s' (a name of the cart's own?)", w); }
            return;
        }
        case MODE_TILES: page = "tile-editor"; break;
        case MODE_MAP: page = "map-editor"; break;
        case MODE_SOUND: page = "sound-editor"; break;
        case MODE_MUSIC: page = "music-editor"; break;
        case MODE_FONT: page = "font-editor"; break;
        case MODE_SETTINGS: page = "settings"; break;
        case MODE_BROWSE: page = ":browse"; break;
        default: break;
    }
    show_page(find_page(page), true);
}

/* F1 in the shell: the index, narrowed to the command being typed (LOAD comes first for "load") */
void help_open_shell(const char *line) {
    char w[32] = "";
    sscanf(line, "%31s", w);
    open_index(w);
    if (w[0] && !nhits) {
        H.q[0] = 0;
        search_update();
        message(COL_ORANGE, "no page names with '%s'", w);
    }
    announce();
}

/* ---------------------------------------------------------------- tests */

/* every keyword, type, command and constant has a page, and every link leads to one;
 * prints what's missing and returns how many */
int help_check(void) {
    int bad = 0;
    for (int t = T_IF; t <= T_ENDDATA; t++)
        if (find_page(tok_name((TokType)t)) < 0) { printf("help: no page for keyword '%s'\n", tok_name((TokType)t)); bad++; }
    static const char *const more[] = {"endwhile", "rem", "endrem", "true", "false", "pi", "list",
        "int", "float", "double", "byte", "short", "long", "string", "uint8", "sint8", "uint16", "sint16",
        "uint32", "sint32", "uint64", "sint64", "len", "str", "abs", "sgn", "min", "max",
        "add_last", "add_first", "insert", "first", "last", "remove_first", "remove_last", "remove", "contains",
        "count", "is_empty", "clear",
        "comments", "numbers", "operators", "basics", "types", "type", "arrays",
        "contents", "shell", "code-editor", "tile-editor", "map-editor", "sound-editor", "music-editor",
        "font-editor", "settings", ":browse", NULL};
    for (int i = 0; more[i]; i++)
        if (find_page(more[i]) < 0) { printf("help: no page for '%s'\n", more[i]); bad++; }
    for (int i = 0; i < native_count(); i++) {
        const char *name = native_get(i)->name;
        if (name[0] != '_' && find_page(name) < 0) { printf("help: no page for command '%s'\n", name); bad++; }
    }
    const ConstDef *tables[] = {gb_gfx_consts, gb_net_consts};
    for (int k = 0; k < 2; k++)
        for (int i = 0; tables[k][i].name; i++)
            if (find_page(tables[k][i].name) < 0) { printf("help: no page for constant '%s'\n", tables[k][i].name); bad++; }
    for (int p = 0; p < help_npages; p++) {
        layout(p);
        for (int i = 0; i < H.nlinks; i++)
            if (!is_index(H.links[i].target) && find_page(H.links[i].target) < 0) { printf("help: page '%s' links to '%s', which has no page\n", help_pages[p].title, H.links[i].target); bad++; }
        for (const char *s = help_pages[p].body; *s;) {          /* code lines must fit on screen */
            const char *e = strchr(s, '\n');
            int n = e ? (int)(e - s) : (int)strlen(s);
            if (n >= 4 && !strncmp(s, "    ", 4) && n - 4 > WIDTH - 2) {
                printf("help: page '%s' has a code line too long for the screen: %.*s\n", help_pages[p].title, n, s); bad++;
            }
            s = e ? e + 1 : s + n;
        }
    }
    H.laid = -1;
    printf("help: %d pages, %d problems\n", help_npages, bad);
    return bad;
}

/* ---------------------------------------------------------------- input */

static void scroll_to(int row) {
    if (row < H.top) H.top = row;
    if (row >= H.top + BODY_ROWS) H.top = row - BODY_ROWS + 1;
}

static void clamp_top(void) {
    int max = H.nlines - BODY_ROWS;
    if (H.top > max) H.top = max;
    if (H.top < 0) H.top = 0;
}

static int link_row(int id) {
    for (int r = 0; r < H.nlines; r++)
        for (int x = 0; x < WIDTH; x++)
            if (H.lines[r].link[x] == id) return r;
    return -1;
}

static void follow(int id) {
    if (id < 0 || id >= H.nlinks) return;
    char target[48];
    snprintf(target, sizeof target, "%s", H.links[id].target);
    if (is_index(target)) { help_index(); return; }
    int p = find_page(target);
    if (p < 0) { message(COL_RED, "no page for '%s'", target); return; }
    show_page(p, true);
}

/* Tab: the next link on screen (or after it); Shift+Tab: the one before */
static void next_link(int dir) {
    if (!H.nlinks) return;
    int s = H.sel;
    if (s < 0) {      /* start from what's on screen */
        s = dir > 0 ? -1 : H.nlinks;
        for (int i = 0; i < H.nlinks; i++) {
            int r = link_row(i);
            if (r >= H.top) { s = dir > 0 ? i - 1 : i; break; }
        }
    }
    s += dir;
    if (s < 0) s = H.nlinks - 1;
    if (s >= H.nlinks) s = 0;
    H.sel = s;
    scroll_to(link_row(s));
}

static void search_key(int sc, int mod) {
    switch (sc) {
        case SDL_SCANCODE_ESCAPE: go_back(); return;
        case SDL_SCANCODE_BACKSPACE: {
            size_t n = strlen(H.q);
            if (!n) { go_back(); return; }
            if (mod & MOD_CTRL) H.q[0] = 0;
            else H.q[n - 1] = 0;
            search_update();
            return;
        }
        case SDL_SCANCODE_UP: if (H.qsel > 0) H.qsel--; break;
        case SDL_SCANCODE_DOWN: if (H.qsel < nhits - 1) H.qsel++; break;
        case SDL_SCANCODE_PAGEUP: H.qsel -= BODY_ROWS - 1; if (H.qsel < 0) H.qsel = 0; break;
        case SDL_SCANCODE_PAGEDOWN: H.qsel += BODY_ROWS - 1; if (H.qsel > nhits - 1) H.qsel = nhits - 1; if (H.qsel < 0) H.qsel = 0; break;
        case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: case SDL_SCANCODE_TAB:
            if (nhits) show_page(hits[H.qsel].page, true);
            return;
        default: return;
    }
    if (H.qsel < H.qtop) H.qtop = H.qsel;
    if (H.qsel >= H.qtop + BODY_ROWS - 1) H.qtop = H.qsel - BODY_ROWS + 2;
}

static void key(const KeyEvent *ev) {
    int sc = ev->sc, mod = ev->mod;
    if (ui_global_key(ev)) return;
    if (mod & MOD_ALT && sc == SDL_SCANCODE_LEFT) { go_back(); return; }
    if (mod & MOD_ALT && sc == SDL_SCANCODE_RIGHT) { go_forward(); return; }
    if (H.search) { search_key(sc, mod); return; }
    switch (sc) {
        case SDL_SCANCODE_ESCAPE: help_close(); return;
        case SDL_SCANCODE_BACKSPACE: go_back(); return;
        case SDL_SCANCODE_UP: H.top--; break;
        case SDL_SCANCODE_DOWN: H.top++; break;
        case SDL_SCANCODE_PAGEUP: H.top -= BODY_ROWS - 1; break;
        case SDL_SCANCODE_PAGEDOWN: case SDL_SCANCODE_SPACE: H.top += BODY_ROWS - 1; break;
        case SDL_SCANCODE_HOME: H.top = 0; break;
        case SDL_SCANCODE_END: H.top = H.nlines; break;
        case SDL_SCANCODE_TAB: next_link(mod & MOD_SHIFT ? -1 : 1); return;
        case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER:
            if (H.sel >= 0) follow(H.sel);
            return;
        default: return;
    }
    clamp_top();
    /* a selected link that scrolled away is dropped, so Tab starts from the screen again */
    if (H.sel >= 0) { int r = link_row(H.sel); if (r < H.top || r >= H.top + BODY_ROWS) H.sel = -1; }
}

static void type_text(const char *t) {
    for (; *t; t++) {
        if (*t < 33 || *t > 126) continue;     /* a space scrolls (as a key), so it never starts a search */
        size_t n = strlen(H.q);
        if (!H.search) { open_index(""); n = 0; }      /* typing on a page: the index, narrowed (Back returns) */
        if (n < sizeof H.q - 1) { H.q[n] = *t; H.q[n + 1] = 0; }
        search_update();
    }
}

/* ---- the top bar's buttons: back, forward, then (on the right) contents, index, close */

enum { B_BACK, B_FORWARD, B_CONTENTS, B_INDEX, B_CLOSE, NBUTTONS_TOP };
static const char *const button_label[NBUTTONS_TOP] = {"< back", "forward >", "contents", "index", "close"};
static int button_x[NBUTTONS_TOP], button_w[NBUTTONS_TOP];

static void place_buttons(void) {
    int x = 1;
    for (int b = B_BACK; b <= B_FORWARD; b++) {
        button_w[b] = text_width(button_label[b]) + 7;
        button_x[b] = x;
        x += button_w[b] + 1;
    }
    x = SCREEN_W - 1;
    for (int b = B_CLOSE; b >= B_CONTENTS; b--) {
        button_w[b] = text_width(button_label[b]) + 7;
        x -= button_w[b];
        button_x[b] = x;
        x -= 1;
    }
}

static bool on_contents(void) { return !H.search && H.page == find_page("contents"); }

static bool button_enabled(int b) {
    switch (b) {
        case B_BACK: return true;           /* with nothing before, back leaves the help, like Backspace */
        case B_FORWARD: return H.nfwd > 0;
        case B_CONTENTS: return !on_contents();
        case B_INDEX: return !(H.search && !H.q[0]);
        default: return true;
    }
}

static bool button_here(int b) { return (b == B_CONTENTS && on_contents()) || (b == B_INDEX && H.search && !H.q[0]); }

static int button_at(int mx, int my) {
    if (my < 0 || my >= BAR_H) return -1;
    for (int b = 0; b < NBUTTONS_TOP; b++)
        if (mx >= button_x[b] && mx < button_x[b] + button_w[b]) return b;
    return -1;
}

static void press_button(int b) {
    if (!button_enabled(b)) return;
    switch (b) {
        case B_BACK: go_back(); break;
        case B_FORWARD: go_forward(); break;
        case B_CONTENTS: show_page(find_page("contents"), true); break;
        case B_INDEX: help_index(); break;
        case B_CLOSE: help_close(); break;
    }
}

static void draw_buttons(void) {
    int hover = button_at(in.mouse_x, in.mouse_y);
    for (int b = 0; b < NBUTTONS_TOP; b++) {
        bool on = button_enabled(b), here_ = button_here(b), lit = on && b == hover;
        int bg = lit ? C_SELBG : here_ ? UI_RGB(1, 1, 2) : UI_RGB(0, 0, 1);
        int fg = lit ? COL_BLACK : here_ ? COL_YELLOW : on ? C_BARTEXT : UI_RGB(1, 1, 2);
        draw_rect(button_x[b], 1, button_w[b], BAR_H - 1, bg, true);
        draw_text(button_label[b], button_x[b] + 4, 0, fg);
    }
}

static void mouse(void) {
    if (in.mouse_hit[1] && in.mouse_y < BAR_H) {
        int b = button_at(in.mouse_x, in.mouse_y);
        if (b >= 0) press_button(b);
        return;
    }
    if (in.wheel) {
        if (H.search) {
            H.qtop -= in.wheel * 3;
            if (H.qtop > nhits - (BODY_ROWS - 1)) H.qtop = nhits - (BODY_ROWS - 1);
            if (H.qtop < 0) H.qtop = 0;
        } else { H.top -= in.wheel * 3; clamp_top(); }
    }
    if (!in.mouse_hit[1] || in.mouse_y < BODY_Y || in.mouse_y >= BODY_Y + BODY_ROWS * FONT_H) return;
    int row = (in.mouse_y - BODY_Y) / FONT_H;
    if (H.search) {
        int i = H.qtop + row - 1;             /* row 0 is the heading */
        if (row > 0 && i >= 0 && i < nhits) show_page(hits[i].page, true);
        return;
    }
    int r = H.top + row, x = (in.mouse_x - MARGIN) / FONT_W;
    if (r < H.nlines && x >= 0 && x < WIDTH && H.lines[r].link[x] >= 0) follow(H.lines[r].link[x]);
}

/* ---------------------------------------------------------------- drawing */

static void draw_bar_text(const char *left, const char *right, int y, int lcolor) {
    draw_rect(0, y, SCREEN_W, BAR_H, C_BAR, true);
    int rw = right ? text_width(right) : 0;
    draw_clip(0, y, SCREEN_W - rw - 9, BAR_H);
    draw_text(left, 3, y, lcolor);
    draw_clip_reset();
    if (right) draw_text(right, SCREEN_W - 3 - rw, y, C_BARTEXT);
}

static void draw_page(void) {
    int hover = -1, hx = (in.mouse_x - MARGIN) / FONT_W, hr = H.top + (in.mouse_y - BODY_Y) / FONT_H;
    if (in.mouse_x >= MARGIN && hx < WIDTH && in.mouse_y >= BODY_Y && hr < H.nlines && hr < H.top + BODY_ROWS) hover = H.lines[hr].link[hx];
    for (int row = 0; row < BODY_ROWS; row++) {
        int r = H.top + row;
        if (r >= H.nlines) break;
        const DLine *d = &H.lines[r];
        int y = BODY_Y + row * FONT_H;
        if (d->code) draw_rect(MARGIN - 2, y, WIDTH * FONT_W + 3, FONT_H, C_CODEBG, true);
        for (int x = 0; x < WIDTH && d->t[x]; x++) {
            int px = MARGIN + x * FONT_W, c = d->c[x], id = d->link[x];
            bool sel = id >= 0 && id == H.sel;
            if (sel) { draw_rect(px - 1, y, FONT_W, FONT_H, C_SELBG, true); c = COL_BLACK; }
            if (d->t[x] != ' ') draw_char((unsigned char)d->t[x], px, y, c);
            if (id >= 0 && id == hover) draw_line(px - 1, y + FONT_H - 1, px + FONT_W - 2, y + FONT_H - 1, sel ? COL_BLACK : C_LINK);
        }
    }
    if (H.nlines > BODY_ROWS) {       /* the scroll bar */
        int h = BODY_ROWS * FONT_H, bh = h * BODY_ROWS / H.nlines;
        if (bh < 6) bh = 6;
        int by = BODY_Y + (h - bh) * H.top / (H.nlines - BODY_ROWS);
        draw_rect(SCREEN_W - 3, BODY_Y, 2, h, UI_RGB(0, 0, 1), true);
        draw_rect(SCREEN_W - 3, by, 2, bh, UI_RGB(1, 2, 3), true);
    }
}

static void draw_search(void) {
    char head[96];
    if (!H.q[0]) snprintf(head, sizeof head, "Index: every page name, A to Z (%d)", nhits);
    else snprintf(head, sizeof head, "%d page name%s with '%s'", nhits, nhits == 1 ? "" : "s", H.q);
    draw_text(head, MARGIN, BODY_Y, C_HEAD);
    for (int row = 1; row < BODY_ROWS; row++) {
        int i = H.qtop + row - 1;
        if (i >= nhits) break;
        int y = BODY_Y + row * FONT_H;
        bool on = i == H.qsel;
        if (on) draw_rect(0, y, SCREEN_W, FONT_H, C_SELBG, true);
        draw_text(hits[i].name, MARGIN + 6, y, on ? COL_BLACK : C_LINK);
        const char *title = help_pages[hits[i].page].title;
        draw_text(title, SCREEN_W - MARGIN - text_width(title), y, on ? COL_BLACK : C_DIM);
    }
    if (!nhits) draw_text("nothing: Backspace to change it, Esc to go back", MARGIN, BODY_Y + 2 * FONT_H, C_DIM);
}

static void draw(void) {
    draw_cls(C_BG);
    char top[160];
    snprintf(top, sizeof top, "%s", H.search ? "Index" : help_pages[H.page].title);
    place_buttons();
    draw_rect(0, 0, SCREEN_W, BAR_H, C_BAR, true);       /* the title, between the buttons */
    int tx = button_x[B_FORWARD] + button_w[B_FORWARD] + 5;
    draw_clip(tx, 0, button_x[B_CONTENTS] - 4 - tx, BAR_H);
    draw_text(top, tx, 0, COL_WHITE);
    draw_clip_reset();
    draw_buttons();
    if (H.search) draw_search();
    else draw_page();
    int by = SCREEN_H - BAR_H;
    if (H.search) {
        char q[64];
        snprintf(q, sizeof q, "find: %s", H.q);
        draw_bar_text(q, H.q[0] ? "Enter open  Esc back" : "type to narrow it  Enter open  F1 back", by, COL_YELLOW);
        if ((frame_count / 30) % 2 == 0) draw_rect(3 + text_width(q), by, 1, FONT_H, COL_YELLOW, true);
    } else if (H.msg[0] && frame_count < H.msg_until) draw_bar_text(H.msg, NULL, by, H.msg_color);
    else draw_bar_text("type to search", "Tab link  Enter go  Bksp back  F1 index", by, C_BARTEXT);
}

void help_frame(void) {
    if (H.laid < 0) layout(H.page = find_page("contents"));
    view_x = 0;
    view_w = SCREEN_W;
    draw_reset();                    /* the built-in font, for the buttons' sizes */
    for (int i = 0; i < in.nevents && ui_mode == MODE_HELP; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type == EV_KEY) key(ev);
        else type_text(ev->text);
    }
    if (ui_mode != MODE_HELP) return;
    place_buttons();
    mouse();
    if (ui_mode != MODE_HELP) return;
    clamp_top();
    draw_reset();
    draw();
}
