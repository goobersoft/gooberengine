/* The console shell: what GooberEngine boots into. A command line for
 * loading and running carts; the built-in editors will hang off it too. */
#include "engine.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define VIEW_ROWS (SCREEN_H / FONT_H)
#define MAX_LINES 256
#define HISTORY 32

bool shell_quit;

static struct { char text[TEXT_COLS + 1]; uint8_t color; } lines[MAX_LINES];
static int nlines;
static char input[TEXT_COLS - 1];
static int inlen;
static char history[HISTORY][sizeof input];
static int nhistory, hpos;

static void push_line(const char *s, int len, int color) {
    if (nlines == MAX_LINES) {
        memmove(lines, lines + 1, sizeof lines[0] * (MAX_LINES - 1));
        nlines--;
    }
    if (len > TEXT_COLS) len = TEXT_COLS;
    memcpy(lines[nlines].text, s, (size_t)len);
    lines[nlines].text[len] = 0;
    lines[nlines].color = (uint8_t)color;
    nlines++;
}

/* prints text, wrapping long lines and splitting on \n */
char player_errors[2048];

void shell_print(const char *s, int color) {
    if (opt.headless) printf("%s\n", s);    /* lets tests see the shell */
    if (opt.player) {                       /* an exported game has no shell: keep errors for its error screen */
        size_t n = strlen(player_errors);
        snprintf(player_errors + n, sizeof player_errors - n, "%s\n", s);
    }
    do {
        const char *nl = strchr(s, '\n');
        int len = nl ? (int)(nl - s) : (int)strlen(s);
        int done = 0;
        do {
            int chunk = len - done > TEXT_COLS ? TEXT_COLS : len - done;
            push_line(s + done, chunk, color);
            done += chunk;
        } while (done < len);
        s = nl ? nl + 1 : NULL;
    } while (s && *s);
}

static void printf_line(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void printf_line(int color, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    shell_print(buf, color);
}

void shell_init(void) {
    shell_print("GOOBERENGINE - VER: " GE_VERSION, COL_YELLOW);
    shell_print("HELP = COMMANDS, LS = CART LIST", COL_GREY);
    shell_print("", COL_WHITE);
}

static void cmd_help(void) {
    static const char *rows[][2] = {
        {"DIR", "list the carts in the carts folder"},
        {"LOAD name", "load a cart (a folder with main.gb)"},
        {"RUN [name]", "save, then run the loaded cart (or load one)"},
        {"EDIT", "edit the cart's code (F1, or Esc on an empty line)"},
        {"NEW [name]", "make a new cart (untitled without a name)"},
        {"TILES", "edit the cart's tiles (F2 anywhere)"},
        {"MAP", "edit the cart's map (F3 anywhere)"},
        {"SOUND", "edit the cart's sound effects (F4 anywhere)"},
        {"MUSIC", "edit the cart's music (F5 anywhere)"},
        {"F6", "in an editor: show or hide the cart's files"},
        {"SAVE [name]", "save everything (an untitled cart is saved as untitled)"},
        {"BROWSE", "pick a cart by its picture"},
        {"EXPORT [file]", "write the cart as one .gbcart file"},
        {"EXPORT target", "make a standalone game: LINUX WINDOWS WEB ALL"},
        {"IMPORT file", "unpack a .gbcart into the carts folder"},
#ifdef __EMSCRIPTEN__
        {"UPLOAD", "open a .gbcart from your computer (or drop one)"},
        {"DOWNLOAD", "save the cart to your computer as a .gbcart"},
#endif
        {"INFO", "show the loaded cart's details"},
        {"FOLDER", "show where carts are read from"},
        {"CLS", "clear the screen"},
        {"QUIT", "leave GooberEngine"},
    };
    for (size_t i = 0; i < sizeof rows / sizeof rows[0]; i++)
        printf_line(COL_WHITE, "  %-13s %s", rows[i][0], rows[i][1]);
    shell_print("in a cart: Esc returns, F8 label, F11 fullscreen, F12 shot", COL_GREY);
}

int ui_mode = MODE_SHELL;

/* Commands that would throw away unsaved work ask "discard them? (Y/N)".
 * The command waits in `pending`: Y runs it with `discarding` set, N (or
 * Esc, or any other command) drops it. */
static char pending[1024];
static const char *pending_no = "kept your changes";   /* what N says */
static bool discarding;
bool ui_unsaved(void) { return editor_modified() || (cart.loaded && docs_dirty(-1)); }

bool ui_save_all(void) {
    if (cart.untitled) { shell_print("untitled cart: SAVE name to keep it", COL_ORANGE); return false; }
    bool ok = true;
    if (editor_modified()) ok = editor_save() && ok;
    if (cart.loaded && docs_dirty(-1)) {
        char msg[1024];
        if (docs_save_all(-1, msg, sizeof msg)) shell_print(msg, COL_GREEN);
        else { shell_print(msg, COL_RED); ok = false; }
    }
    return ok;
}

static bool confirm_discard(const char *cmd) {
    if (!ui_unsaved() || discarding) { pending[0] = 0; return true; }
    snprintf(pending, sizeof pending, "%s", cmd);
    pending_no = "kept your changes";
    const char *parts[5];
    int n = 0;
    if (editor_modified()) parts[n++] = "code";
    if (docs_dirty(RES_TILES)) parts[n++] = "tiles";
    if (docs_dirty(RES_MAP)) parts[n++] = "map";
    if (docs_dirty(RES_SFX)) parts[n++] = "sound";
    if (docs_dirty(RES_MUSIC)) parts[n++] = "music";
    char list[64] = "";
    for (int i = 0; i < n; i++) {     /* "code", "code and map", "code, tiles and map" */
        if (i) strcat(list, i == n - 1 ? " and " : ", ");
        strcat(list, parts[i]);
    }
    printf_line(COL_ORANGE, "unsaved changes in the %s:", list);
    shell_print("discard them? (Y/N)", COL_ORANGE);
    return false;
}

/* runs a command as if Y had already been answered (the cart browser asks on its own screen) */
void shell_exec_discarding(const char *line) {
    discarding = true;
    shell_exec(line);
    discarding = false;
}

/* "carts/name": the cart's folder as the carts folder's own name plus the cart's,
 * short enough to read on one line (FOLDER and INFO still give the full path) */
static const char *short_dir(void) {
    static char out[512];
    const char *root = carts_root(), *base = strrchr(root, '/');
    snprintf(out, sizeof out, "%s/%s", base && base[1] ? base + 1 : root, cart.name);
    return out;
}

static void export_log(const char *line, bool error) { shell_print(line, error ? COL_RED : COL_GREEN); }

static int cmp_str(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static void cmd_dir(void) {
    const char *root = carts_root();
    DIR *d = opendir(root);
    if (!d) { printf_line(COL_RED, "cannot read %s", root); return; }
    char *names[256];
    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < 256) {
        if (e->d_name[0] == '.') continue;
        char p[4096];
        struct stat st;
        snprintf(p, sizeof p, "%s/%s/main.gb", root, e->d_name);
        if (stat(p, &st) == 0) names[n++] = strdup(e->d_name);
    }
    closedir(d);
    qsort(names, (size_t)n, sizeof names[0], cmp_str);
    if (!n) shell_print("no carts here (try FOLDER)", COL_GREY);
    for (int i = 0; i < n; i++) {
        printf_line(COL_SKY, "  %s", names[i]);
        free(names[i]);
    }
}

static bool ends_with(const char *s, const char *tail) {
    size_t a = strlen(s), b = strlen(tail);
    return a >= b && !strcasecmp(s + a - b, tail);
}

static bool do_load(const char *name) {
    char err[512], dir[4096];
    if (ends_with(name, ".gbcart")) {        /* a one-file cart: unpack it into the carts folder first */
        if (!cart_import(name, NULL, dir, sizeof dir, err, sizeof err)) { shell_print(err, COL_RED); return false; }
        printf_line(COL_GREEN, "imported %s", dir);
        name = dir;
    }
    if (!cart_load(name, err, sizeof err)) { shell_print(err, COL_RED); return false; }
    editor_forget();
    cart_discard_scratch();                  /* the untitled cart was given up (confirmed if it had edits) */
    printf_line(COL_GREEN, "loaded %s", cart.title);
    return true;
}

/* running always saves the code being edited first, so what runs is what you see */
static int do_run(void) {
    if (editor_modified() && !(cart.untitled ? editor_flush() : editor_save())) return 1;
    int r = cart_run();
    input_consume();          /* the Esc that stopped the cart must not leak into the shell */
    draw_reset();
    if (r == 0) printf_line(COL_GREY, "%s ended", cart.title);
    else if (r == 2) printf_line(COL_GREY, "%s stopped", cart.title);
    if (label_taken) shell_print("saved the cart's label (label.txt)", COL_GREEN);
    return r;
}

/* run from an editor and come back to it; an error in main.gb goes to its line in the code editor */
void ui_run_from_editor(void) {
    int back = ui_mode;
    int r = do_run();
    ui_mode = back;
    size_t fl = strlen(cart_err_file);
    bool cart_file = fl > 3 && !strcmp(cart_err_file + fl - 3, ".gb") && !strchr(cart_err_file, '/');
    if (r == 1 && cart_err_line > 0 && cart_file && editor_open_file(cart_err_file)) {
        ui_switch(MODE_EDITOR);
        editor_show_error(cart_err_line, cart_err_msg);
    } else if (r == 1) ui_mode = MODE_SHELL;     /* error elsewhere (an include): the shell shows it */
}

void ui_switch(int mode) {
    if (mode != MODE_SHELL && !cart.loaded) {
        shell_print("no cart loaded: LOAD or NEW one first", COL_RED);
        ui_mode = MODE_SHELL;
        return;
    }
    if (mode == MODE_EDITOR && !editor_open()) return;
    ui_mode = mode;
}

static void open_editor(void) { ui_switch(MODE_EDITOR); }

bool ui_global_key(const KeyEvent *ev) {
    if (ev->type != EV_KEY) return false;
    if (ev->sc == SDL_SCANCODE_F1) { ui_switch(MODE_EDITOR); return true; }
    if (ev->sc == SDL_SCANCODE_F2) { ui_switch(MODE_TILES); return true; }
    if (ev->sc == SDL_SCANCODE_F3) { ui_switch(MODE_MAP); return true; }
    if (ev->sc == SDL_SCANCODE_F4) { ui_switch(MODE_SOUND); return true; }
    if (ev->sc == SDL_SCANCODE_F5) { ui_switch(MODE_MUSIC); return true; }
    if (ev->sc == SDL_SCANCODE_F6) { tree_open = !tree_open; return true; }
    return false;
}

/* the editors' top bar: the file tree's button, a clickable tab per editor
 * (* = unsaved), and the save / run keys if there is room */
int ui_draw_tabs(void) {
    static const struct { const char *label, *word; int mode, kind; } tabs[] = {
        {"F1 code", "code", MODE_EDITOR, -1}, {"F2 tiles", "tiles", MODE_TILES, RES_TILES}, {"F3 map", "map", MODE_MAP, RES_MAP},
        {"F4 sound", "sound", MODE_SOUND, RES_SFX}, {"F5 music", "music", MODE_MUSIC, RES_MUSIC}};
    draw_rect(0, 0, UI_W, FONT_H, RGB6(0, 1, 2), true);
    /* the tree button: F6 */
    bool over = in.mouse_y >= 0 && in.mouse_y < FONT_H && in.mouse_x >= 0 && in.mouse_x < 9;
    draw_rect(0, 0, 9, FONT_H, over ? RGB6(1, 2, 3) : RGB6(0, 0, 1), true);
    for (int k = 0; k < 3; k++) draw_line(2, 2 + k * 3, 6, 2 + k * 3, tree_open ? COL_YELLOW : RGB6(2, 3, 3));
    if (in.mouse_hit[1] && over) tree_open = !tree_open;
    bool full = UI_W >= SCREEN_W;      /* beside the tree there's no room for the F keys */
    int x = 10;
    for (size_t i = 0; i < sizeof tabs / sizeof tabs[0]; i++) {
        bool dirty = tabs[i].kind < 0 ? editor_modified() : docs_dirty(tabs[i].kind);
        char label[32];
        snprintf(label, sizeof label, "%s%s", full ? tabs[i].label : tabs[i].word, dirty ? "*" : "");
        int w = text_width(label) + 5;
        bool on = ui_mode == tabs[i].mode;
        if (on) draw_rect(x, 0, w, FONT_H, RGB6(1, 2, 3), true);
        draw_text(label, x + 3, 0, on ? COL_BLACK : RGB6(2, 3, 3));
        if (in.mouse_hit[1] && in.mouse_y >= 0 && in.mouse_y < FONT_H && in.mouse_x >= x && in.mouse_x < x + w) ui_switch(tabs[i].mode);
        x += w + 1;
    }
    const char *keys = "^S save ^R run";
    if (x + text_width(keys) + 6 <= UI_W) draw_text(keys, UI_W - 3 - text_width(keys), 0, RGB6(2, 3, 3));
    return x;
}

void shell_exec(const char *raw) {
    char line[1024];          /* longer than the prompt: --exec and BROWSE send whole paths */
    snprintf(line, sizeof line, "%s", raw);
    char *cmd = line;
    while (isspace((unsigned char)*cmd)) cmd++;
    char *arg = cmd;
    while (*arg && !isspace((unsigned char)*arg)) arg++;
    if (*arg) *arg++ = 0;
    while (isspace((unsigned char)*arg)) arg++;
    for (char *e = arg + strlen(arg); e > arg && isspace((unsigned char)e[-1]);) *--e = 0;

    if (!*cmd) return;
    if (pending[0]) {                               /* answering "discard them? (Y/N)" */
        char waiting[sizeof pending];
        snprintf(waiting, sizeof waiting, "%s", pending);
        pending[0] = 0;
        if (!*arg && (!strcasecmp(cmd, "y") || !strcasecmp(cmd, "yes"))) { shell_exec_discarding(waiting); return; }
        if (!*arg && (!strcasecmp(cmd, "n") || !strcasecmp(cmd, "no"))) { shell_print(pending_no, COL_GREY); return; }
        /* anything else is a new command, and the question is dropped */
    }
    char again[1024];
    snprintf(again, sizeof again, "%s %s", cmd, arg);

    if (!strcasecmp(cmd, "help") || !strcmp(cmd, "?")) cmd_help();
    else if (!strcasecmp(cmd, "dir") || !strcasecmp(cmd, "ls")) cmd_dir();
    else if (!strcasecmp(cmd, "cls") || !strcasecmp(cmd, "clear")) nlines = 0;
    else if (!strcasecmp(cmd, "folder")) shell_print(carts_root(), COL_SKY);
    else if (!strcasecmp(cmd, "load")) {
        if (!*arg) shell_print("usage: LOAD name", COL_RED);
        else if (confirm_discard(again)) do_load(arg);
    } else if (!strcasecmp(cmd, "run")) {
        if (*arg && (!confirm_discard(again) || !do_load(arg))) return;
        do_run();
    } else if (!strcasecmp(cmd, "export") || !strcasecmp(cmd, "download")) {
        char err[512], out[4096], first[16] = "";
        sscanf(arg, "%15s", first);
        bool game = !strcasecmp(first, "linux") || !strcasecmp(first, "windows") ||
                    !strcasecmp(first, "web") || !strcasecmp(first, "all");
        if (!cart.loaded) shell_print("no cart loaded", COL_RED);
        else if (cart.untitled) shell_print("untitled cart: SAVE name first", COL_ORANGE);
        else if (ui_unsaved()) shell_print("SAVE first: EXPORT bundles the files on disk", COL_ORANGE);
        else if (game && !strcasecmp(cmd, "export")) game_export(arg, NULL, export_log);
        else {
            char tmp[300];
#ifdef __EMSCRIPTEN__
            if (!*arg) { snprintf(tmp, sizeof tmp, "/tmp/%s.gbcart", cart.name); arg = tmp; }
#else
            (void)tmp;
            if (!strcasecmp(cmd, "download")) { shell_print("DOWNLOAD is for the web version: use EXPORT", COL_GREY); return; }
#endif
            if (cart_export(arg, out, sizeof out, err, sizeof err)) {
                printf_line(COL_GREEN, "wrote %s", out);
                web_offer_download(out);
            } else shell_print(err, COL_RED);
        }
    } else if (!strcasecmp(cmd, "upload")) {
#ifdef __EMSCRIPTEN__
        web_pick_file();
        shell_print("pick a .gbcart file (or drop one on the screen)", COL_GREY);
#else
        shell_print("UPLOAD is for the web version: drop a .gbcart or cart folder on the window", COL_GREY);
#endif
    } else if (!strcasecmp(cmd, "import")) {
        char err[512], dir[4096], file[256], name[128] = "";
        if (sscanf(arg, "%255s %127s", file, name) < 1) shell_print("usage: IMPORT file.gbcart [name]", COL_RED);
        else if (!confirm_discard(again)) return;
        else if (!cart_import(file, name, dir, sizeof dir, err, sizeof err)) shell_print(err, COL_RED);
        else { printf_line(COL_GREEN, "imported %s", dir); do_load(dir); }
    } else if (!strcasecmp(cmd, "browse") || !strcasecmp(cmd, "carts")) {
        browse_open();
    } else if (!strcasecmp(cmd, "edit")) {
        open_editor();
    } else if (!strcasecmp(cmd, "new")) {
        char err[512];
        if (!confirm_discard(again)) return;
        else if (!*arg) {                          /* NEW alone: a fresh untitled cart */
            if (!cart_new_untitled(err, sizeof err)) { shell_print(err, COL_RED); return; }
            editor_forget();
            shell_print("new untitled cart: SAVE name to keep it", COL_GREEN);
            open_editor();
        } else if (!cart_create(arg, err, sizeof err)) shell_print(err, COL_RED);
        else {
            editor_forget();
            cart_discard_scratch();
            printf_line(COL_GREEN, "created %s", short_dir());
            open_editor();
        }
    } else if (!strcasecmp(cmd, "save")) {
        char err[512];
        if (!cart.loaded) shell_print("no cart loaded", COL_RED);
        else if (cart.untitled) {
            if (!*arg) arg = "untitled";            /* SAVE alone keeps it as carts/untitled */
            if (cart_exists(arg) && !discarding) {  /* same Y/N as discarding: Y reruns this SAVE as confirmed */
                snprintf(pending, sizeof pending, "%s", again);
                pending_no = "not saved: SAVE another name";
                printf_line(COL_ORANGE, "cart '%s' already exists", arg);
                shell_print("overwrite it? (Y/N)", COL_ORANGE);
                return;
            }
            if (!cart_save_as(arg, discarding, err, sizeof err)) { shell_print(err, COL_RED); return; }
            editor_follow_cart();
            ui_save_all();                         /* the edits the scratch copy didn't have yet */
            printf_line(COL_GREEN, "saved as %s", short_dir());
        } else if (*arg) printf_line(COL_RED, "the cart is already named %s: SAVE alone saves it", cart.name);
        else if (!ui_unsaved()) shell_print("nothing to save", COL_GREY);
        else {
            bool code = editor_modified();
            if (ui_save_all() && code) shell_print("saved the code", COL_GREEN);
        }
    } else if (!strcasecmp(cmd, "info")) {
        if (!cart.loaded) { shell_print("no cart loaded", COL_GREY); return; }
        printf_line(COL_WHITE, "title   %s", cart.title);
        if (cart.author[0]) printf_line(COL_WHITE, "author  %s", cart.author);
        if (cart.version[0]) printf_line(COL_WHITE, "version %s", cart.version);
        const Tileset *ts = doc_active(RES_TILES)->data;
        int used = 0;
        for (int t = 0; t < NTILES; t++)
            for (int i = 0; i < TILE * TILE; i++)
                if (ts->t[t][i] != CLEAR) { used++; break; }
        printf_line(COL_WHITE, "tiles   %d of %d used in %s", used, NTILES, doc_active(RES_TILES)->name);
        for (int k = 0; k < NRES; k++) {
            const char *names[MAX_DOCS * 2];
            int n = docs_list(k, names, MAX_DOCS * 2);
            char list[256] = "";
            for (int i = 0; i < n; i++) {
                size_t l = strlen(list);
                snprintf(list + l, sizeof list - l, "%s%s", i ? " " : "", names[i]);
            }
            printf_line(COL_WHITE, "%-7s %s", res_dir[k], n ? list : "-");
        }
        shell_print(cart.untitled ? "untitled: not saved (SAVE name to keep it)" : cart.dir, COL_GREY);
    } else if (!strcasecmp(cmd, "quit") || !strcasecmp(cmd, "exit") || !strcasecmp(cmd, "shutdown")) {
        if (confirm_discard(again)) shell_quit = true;
    } else if (!strcasecmp(cmd, "tiles")) {
        ui_switch(MODE_TILES);
    } else if (!strcasecmp(cmd, "map")) {
        ui_switch(MODE_MAP);
    } else if (!strcasecmp(cmd, "sound") || !strcasecmp(cmd, "sfx")) {
        ui_switch(MODE_SOUND);
    } else if (!strcasecmp(cmd, "music")) {
        ui_switch(MODE_MUSIC);
    } else {
        printf_line(COL_RED, "unknown command '%s' (try HELP)", cmd);
    }
}

/* while "discard them? (Y/N)" is waiting, Y or N on an empty line answers at once, and Esc means N */
static bool answer_key(void) {
    if (!pending[0] || inlen) return false;
    const char *ans = NULL;
    if (in.escape) ans = "n";
    for (const char *p = in.text; *p && !ans; p++) {
        if (*p == 'y' || *p == 'Y') ans = "y";
        else if (*p == 'n' || *p == 'N') ans = "n";
    }
    if (!ans) return false;
    char echo[16];
    snprintf(echo, sizeof echo, "> %s", ans);
    push_line(echo, (int)strlen(echo), COL_WHITE);
    shell_exec(ans);
    return true;
}

void shell_frame(void) {
    /* keys */
    if (answer_key()) { in.text[0] = 0; in.escape = false; }    /* the answer key is used up */
    for (const char *p = in.text; *p; p++)
        if (*p >= 32 && *p < 127 && inlen < (int)sizeof input - 1) input[inlen++] = *p;
    input[inlen] = 0;
    if (in.key_hit[SDL_SCANCODE_BACKSPACE] && inlen > 0) input[--inlen] = 0;
    if (in.key_hit[SDL_SCANCODE_F1] && cart.loaded) { ui_switch(MODE_EDITOR); return; }
    if (in.key_hit[SDL_SCANCODE_F2] && cart.loaded) { ui_switch(MODE_TILES); return; }
    if (in.key_hit[SDL_SCANCODE_F3] && cart.loaded) { ui_switch(MODE_MAP); return; }
    if (in.key_hit[SDL_SCANCODE_F4] && cart.loaded) { ui_switch(MODE_SOUND); return; }
    if (in.key_hit[SDL_SCANCODE_F5] && cart.loaded) { ui_switch(MODE_MUSIC); return; }
    if (in.escape) {
        if (inlen == 0 && cart.loaded) { open_editor(); return; }
        inlen = 0;
        input[0] = 0;
    }
    if (in.key_hit[SDL_SCANCODE_UP] && hpos > 0) {
        snprintf(input, sizeof input, "%s", history[--hpos]);
        inlen = (int)strlen(input);
    }
    if (in.key_hit[SDL_SCANCODE_DOWN] && hpos < nhistory) {
        hpos++;
        snprintf(input, sizeof input, "%s", hpos < nhistory ? history[hpos] : "");
        inlen = (int)strlen(input);
    }
    if (in.key_hit[SDL_SCANCODE_RETURN] || in.key_hit[SDL_SCANCODE_KP_ENTER]) {
        char cmd[sizeof input];
        snprintf(cmd, sizeof cmd, "%s", input);
        char echo[sizeof input + 2];
        snprintf(echo, sizeof echo, "> %s", cmd);
        push_line(echo, (int)strlen(echo), COL_WHITE);
        if (inlen && (nhistory == 0 || strcmp(history[nhistory - 1], cmd))) {
            if (nhistory == HISTORY) { memmove(history, history + 1, sizeof history[0] * (HISTORY - 1)); nhistory--; }
            snprintf(history[nhistory++], sizeof history[0], "%s", cmd);
        }
        hpos = nhistory;
        inlen = 0;
        input[0] = 0;
        shell_exec(cmd);
    }

    /* draw: scrollback, then the prompt on the line after the last output */
    draw_reset();
    draw_cls(COL_BLACK);
    int total = nlines + 1;
    int first = total > VIEW_ROWS ? total - VIEW_ROWS : 0;
    int y = 0;
    for (int i = first; i < nlines; i++, y += FONT_H) draw_text(lines[i].text, 0, y, lines[i].color);
    int x = draw_text(pending[0] && !inlen ? "(Y/N) > " : "> ", 0, y, pending[0] ? COL_ORANGE : COL_GREEN);
    x = draw_text(input, x, y, COL_WHITE);
    if ((frame_count / 30) % 2 == 0) draw_char(127, x, y, COL_GREEN);
}
