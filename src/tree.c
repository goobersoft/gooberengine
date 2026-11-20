/* The file tree: the cart's files down the left side of every editor.
 *
 *   code      +        click a file to edit it; + makes a new one
 *    main
 *   tiles     +        the file each editor has open is highlighted;
 *    tileset*          * = changed, not saved
 *   maps      +
 *   ...
 *
 * Right-click a file for rename (type the new name, Enter) or delete (Y to
 * confirm). main.gb can't be renamed or deleted: a cart starts there.
 *
 * F6 (or the button at the left of the tabs) shows and hides it. The editor
 * runs in the rest of the screen: its drawing is shifted right by the tree's
 * width (view_x) and the mouse is shifted to match, so an editor's own
 * coordinates start at 0 either way. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool tree_open = true;

#define ROW FONT_H
#define TOP (FONT_H + 1)
#define MAX_ROWS 200
#define SETTINGS_Y (SCREEN_H - ROW)     /* the settings button: the tree's bottom row */

enum { SEC_CODE = NRES };          /* sections: the resource kinds, then code */
static const int sec_order[] = {SEC_CODE, RES_TILES, RES_MAP, RES_SFX, RES_MUSIC, RES_FONT};
static const char *const sec_title[] = {"tiles", "maps", "sfx", "music", "fonts", "code"};
static const int sec_mode[] = {MODE_TILES, MODE_MAP, MODE_SOUND, MODE_MUSIC, MODE_FONT, MODE_EDITOR};

typedef struct { int sec; char name[64]; bool header; int nfiles; bool dirty; } Row;   /* nfiles, dirty: a header's section */
static Row rows[MAX_ROWS];
static int nrows, scroll;
static bool folded[SEC_CODE + 1];  /* a click on a section's name folds its files away, and back */

/* the right-click menu, and what it's doing */
enum { ACT_NONE, ACT_MENU, ACT_RENAME, ACT_CONFIRM };
static struct {
    int mode;
    Row row;                  /* the file it's about */
    char text[48];            /* rename: the new name (no extension) */
    int len;
    char msg[64];
    int msg_color;
    long msg_until;
} act;

static void say(int color, const char *msg) {
    snprintf(act.msg, sizeof act.msg, "%s", msg);
    act.msg_color = color;
    act.msg_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[tree] %s\n", msg);
}

static int cmp_str(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }

/* the cart's .gb files: on disk, plus one the editor has open that isn't saved yet */
static int code_files(char names[][64], int max) {
    int n = 0;
    DIR *d = opendir(cart.dir);
    struct dirent *e;
    while (d && (e = readdir(d)) && n < max) {
        size_t len = strlen(e->d_name);
        if (len > 3 && len < 64 && !strcmp(e->d_name + len - 3, ".gb") && e->d_name[0] != '.') snprintf(names[n++], 64, "%s", e->d_name);
    }
    if (d) closedir(d);
    const char *cur = editor_file();
    bool have = !*cur;
    for (int i = 0; i < n && !have; i++) have = !strcmp(names[i], cur);
    if (!have && n < max) snprintf(names[n++], 64, "%s", cur);
    char *ptr[64];
    for (int i = 0; i < n; i++) ptr[i] = names[i];
    qsort(ptr, (size_t)n, sizeof ptr[0], cmp_str);
    static char sorted[64][64];
    for (int i = 0; i < n; i++) snprintf(sorted[i], 64, "%s", ptr[i]);
    memcpy(names, sorted, sizeof sorted[0] * (size_t)n);
    return n;
}

static bool row_active(const Row *r) {
    if (r->sec == SEC_CODE) return !strcmp(editor_file(), r->name);
    return cart.active[r->sec] && !strcmp(cart.active[r->sec]->name, r->name);
}

static bool row_dirty(const Row *r) {
    if (r->sec == SEC_CODE) return editor_file_modified(r->name);
    Doc *d = doc_find(r->sec, r->name);
    return d && d->dirty;
}

/* a section's header, then its files (none when it's folded) */
static void build(void) {
    nrows = 0;
    for (size_t s = 0; s < sizeof sec_order / sizeof sec_order[0] && nrows < MAX_ROWS; s++) {
        int sec = sec_order[s];
        static char code[64][64];
        const char *names[MAX_DOCS * 2];
        int n;
        if (sec == SEC_CODE) {
            n = code_files(code, 64);
            for (int i = 0; i < n; i++) names[i] = code[i];
        } else n = docs_list(sec, names, MAX_DOCS * 2);
        Row *h = &rows[nrows++];
        *h = (Row){sec, "", true, n, false};
        for (int i = 0; i < n; i++) {
            Row r = {sec, "", false, 0, false};
            snprintf(r.name, sizeof r.name, "%s", names[i]);
            if (row_dirty(&r)) h->dirty = true;
            if (!folded[sec] && nrows < MAX_ROWS) rows[nrows++] = r;
        }
    }
}

/* switch the editor of the row's kind to its file */
static void open_row(const Row *r) {
    if (r->sec == SEC_CODE) {
        if (editor_open_file(r->name)) ui_switch(MODE_EDITOR);
        return;
    }
    char err[512];
    Doc *d = doc_open(r->sec, r->name, err, sizeof err);
    if (!d) { shell_print(err, COL_RED); if (opt.headless) fprintf(stderr, "[tree] %s\n", err); return; }
    if (cart.active[r->sec] != d) {
        doc_set_active(d);
        docs_bind();
        if (r->sec == RES_TILES) tiles_doc_changed();
        else if (r->sec == RES_MAP) map_doc_changed();
        else if (r->sec == RES_SFX) sound_doc_changed();
        else if (r->sec == RES_FONT) font_doc_changed();
        else music_doc_changed();
    }
    ui_switch(sec_mode[r->sec]);
}

/* + on a section: a new file with the next free name (tiles-1, map-2 ...) */
static void new_file(int sec) {
    static const char *const stem[] = {"tiles", "map", "sfx", "music", "font", "code"};
    for (int i = 1; i < 100; i++) {
        char name[64], path[PATH_MAX + 80];
        if (sec == SEC_CODE) {
            snprintf(name, sizeof name, "%s-%d.gb", stem[sec], i);
            snprintf(path, sizeof path, "%s/%s", cart.dir, name);
            FILE *f = fopen(path, "r");
            if (f) { fclose(f); continue; }
            if (editor_file_modified(name) || !strcmp(editor_file(), name)) continue;
            Row r = {sec, "", false, 0, false};
            snprintf(r.name, sizeof r.name, "%s", name);
            open_row(&r);
            return;
        }
        snprintf(name, sizeof name, "%s-%d", stem[sec], i);
        char err[256];
        Doc *d = doc_find(sec, name) ? NULL : doc_create(sec, name, err, sizeof err);
        if (!d) continue;
        Row r = {sec, "", false, 0, false};
        snprintf(r.name, sizeof r.name, "%s", name);
        open_row(&r);
        return;
    }
}

static bool file_there(const char *path) { FILE *f = fopen(path, "r"); if (f) fclose(f); return f != NULL; }

static void do_rename(void) {
    const Row *r = &act.row;
    char err[512], to[64];
    act.text[act.len] = 0;
    if (r->sec == SEC_CODE) {
        snprintf(to, sizeof to, "%s.gb", act.text);
        char from_p[PATH_MAX + 80], to_p[PATH_MAX + 80];
        snprintf(from_p, sizeof from_p, "%s/%s", cart.dir, r->name);
        snprintf(to_p, sizeof to_p, "%s/%s", cart.dir, to);
        if (!strcmp(to, r->name)) return;
        if (!cart_valid_name(act.text) || act.len > 40) { say(COL_RED, "use letters, digits, - and _"); return; }
        if (!strcmp(to, "main.gb") || file_there(to_p) || editor_file_modified(to) || !strcmp(editor_file(), to)) {
            snprintf(err, sizeof err, "%s already exists", act.text);
            say(COL_RED, err);
            return;
        }
        if (file_there(from_p) && rename(from_p, to_p) != 0) { say(COL_RED, "could not rename it"); return; }
        editor_rename_file(r->name, to);
    } else {
        snprintf(to, sizeof to, "%s", act.text);
        if (!doc_rename(r->sec, r->name, to, err, sizeof err)) { say(COL_RED, err); return; }
    }
    snprintf(err, sizeof err, "renamed %s to %s", r->sec == SEC_CODE ? code_name(r->name) : r->name, r->sec == SEC_CODE ? code_name(to) : to);
    say(COL_GREEN, err);
}

static void do_delete(void) {
    const Row *r = &act.row;
    char err[512];
    if (r->sec == SEC_CODE) {
        char path[PATH_MAX + 80];
        snprintf(path, sizeof path, "%s/%s", cart.dir, r->name);
        if (file_there(path) && remove(path) != 0) { say(COL_RED, "could not delete it"); return; }
        editor_close_file(r->name);
    } else if (!doc_delete(r->sec, r->name, err, sizeof err)) { say(COL_RED, err); return; }
    snprintf(err, sizeof err, "deleted %s", r->sec == SEC_CODE ? code_name(r->name) : r->name);
    say(COL_GREEN, err);
}

/* keys while renaming or confirming: the tree has the keyboard */
static void act_keys(void) {
    for (int i = 0; i < in.nevents; i++) {
        const KeyEvent *ev = &in.events[i];
        if (act.mode == ACT_RENAME && ev->type == EV_TEXT) {
            for (const char *p = ev->text; *p; p++)
                if ((isalnum((unsigned char)*p) || *p == '-' || *p == '_') && act.len < 40) act.text[act.len++] = *p;
            continue;
        }
        if (ev->type != EV_KEY) continue;
        if (ev->sc == SDL_SCANCODE_ESCAPE) { act.mode = ACT_NONE; return; }
        if (act.mode == ACT_RENAME) {
            if (ev->sc == SDL_SCANCODE_BACKSPACE && act.len > 0) act.len--;
            if (ev->sc == SDL_SCANCODE_RETURN || ev->sc == SDL_SCANCODE_KP_ENTER) { act.mode = ACT_NONE; do_rename(); return; }
        } else if (act.mode == ACT_CONFIRM) {
            if (ev->sc == SDL_SCANCODE_Y) { act.mode = ACT_NONE; do_delete(); return; }
            if (ev->sc == SDL_SCANCODE_N) { act.mode = ACT_NONE; return; }
        }
    }
}

/* right-click on a file */
static void open_menu(const Row *r) {
    if (r->sec == SEC_CODE && !strcmp(r->name, "main.gb")) { say(COL_ORANGE, "main stays: a cart starts there"); return; }
    act.mode = ACT_MENU;
    act.row = *r;
}

static int row_index(const Row *r) {
    for (int i = 0; i < nrows; i++)
        if (!rows[i].header && rows[i].sec == r->sec && !strcmp(rows[i].name, r->name)) return i;
    return -1;
}

static void tree_frame(void) {
    build();
    int shown = (SETTINGS_Y - TOP) / ROW;
    bool over = in.mouse_x >= 0 && in.mouse_x < TREE_W;
    if (over && in.wheel) scroll -= in.wheel * 2;
    if (scroll > nrows - shown) scroll = nrows - shown;
    if (scroll < 0) scroll = 0;

    if (act.mode != ACT_NONE) act_keys();          /* Esc closes the menu too */
    int arow = act.mode ? row_index(&act.row) : -1;
    if (act.mode && arow < 0) act.mode = ACT_NONE;          /* the file went away */
    int ay = TOP + (arow - scroll) * ROW;                      /* the menu sits below its row */
    int my = ay + ROW + 2 * ROW > SCREEN_H ? ay - 2 * ROW : ay + ROW;

    /* clicks */
    if (act.mode == ACT_MENU && (in.mouse_hit[1] || in.mouse_hit[2])) {
        bool on = over && in.mouse_y >= my && in.mouse_y < my + 2 * ROW;
        if (on && in.mouse_hit[1] && in.mouse_y < my + ROW) {
            act.mode = ACT_RENAME;
            snprintf(act.text, sizeof act.text, "%s", act.row.name);
            act.len = (int)strlen(act.text);
            if (act.row.sec == SEC_CODE && act.len > 3) act.text[act.len -= 3] = 0;    /* the .gb stays */
        } else if (on && in.mouse_hit[1]) act.mode = ACT_CONFIRM;
        else act.mode = ACT_NONE;
        memset(in.mouse_hit, 0, sizeof in.mouse_hit);
    } else if ((act.mode == ACT_RENAME || act.mode == ACT_CONFIRM) && (in.mouse_hit[1] || in.mouse_hit[2])) {
        bool on_row = over && in.mouse_y >= ay && in.mouse_y < ay + ROW;
        if (!on_row) act.mode = ACT_NONE;                       /* a click elsewhere cancels */
        memset(in.mouse_hit, 0, sizeof in.mouse_hit);
    }
    if (over && in.mouse_hit[2] && in.mouse_y >= TOP) {
        int i = scroll + (in.mouse_y - TOP) / ROW;
        if (i < nrows && !rows[i].header) open_menu(&rows[i]);
    }
    if (over && in.mouse_hit[1]) {
        if (in.mouse_y < FONT_H) { if (in.mouse_x >= TREE_W - 10) tree_open = false; }
        else if (in.mouse_y >= SETTINGS_Y) {
            if (ui_mode == MODE_SETTINGS) settings_frame_close();
            else settings_open();
        } else {
            int i = scroll + (in.mouse_y - TOP) / ROW;
            if (in.mouse_y >= TOP && i < nrows) {
                if (rows[i].header) {
                    if (in.mouse_x >= TREE_W - 10) { folded[rows[i].sec] = false; new_file(rows[i].sec); }   /* the new file shows */
                    else folded[rows[i].sec] = !folded[rows[i].sec];
                }
                else open_row(&rows[i]);
                build();
            }
        }
    }

    /* drawing */
    draw_reset();
    draw_cls(UI_RGB(0, 0, 1));
    draw_rect(0, 0, TREE_W, FONT_H, UI_RGB(0, 1, 2), true);
    draw_text("files", 3, 0, UI_RGB(2, 3, 3));
    draw_text("<", TREE_W - 8, 0, over && in.mouse_y < FONT_H && in.mouse_x >= TREE_W - 10 ? COL_YELLOW : UI_RGB(2, 3, 3));
    int mode_sec = -1;
    for (int k = 0; k <= SEC_CODE; k++) if (sec_mode[k] == ui_mode) mode_sec = k;
    for (int k = 0; k < shown; k++) {
        int i = scroll + k;
        if (i >= nrows) break;
        const Row *r = &rows[i];
        int y = TOP + k * ROW;
        bool hover = over && in.mouse_y >= y && in.mouse_y < y + ROW;
        if (r->header) {
            bool f = folded[r->sec], name_hover = hover && in.mouse_x < TREE_W - 10;
            int c = r->sec == mode_sec ? COL_YELLOW : name_hover ? COL_WHITE : UI_RGB(2, 2, 3);
            int tc = name_hover ? COL_YELLOW : UI_RGB(1, 2, 2);
            for (int k = 0; k < 3; k++) {      /* a small triangle: pointing right when folded, down when open */
                if (f) draw_line(2 + k, y + 3 + k, 2 + k, y + 7 - k, tc);
                else draw_line(2 + k, y + 4 + k, 6 - k, y + 4 + k, tc);
            }
            int x = draw_text(sec_title[r->sec], 9, y, c);
            if (f) {              /* folded: how many files it holds, and * if one isn't saved */
                char n[16];
                snprintf(n, sizeof n, "%d%s", r->nfiles, r->dirty ? "*" : "");
                draw_text(n, x + 3, y, UI_RGB(1, 2, 2));
            }
            draw_text("+", TREE_W - 8, y, hover && in.mouse_x >= TREE_W - 10 ? COL_YELLOW : UI_RGB(1, 2, 2));
            continue;
        }
        if (act.mode >= ACT_RENAME && i == arow) {
            if (act.mode == ACT_CONFIRM) {
                draw_rect(0, y, TREE_W, ROW, UI_RGB(2, 0, 0), true);
                draw_text("delete? y/n", 2, y, COL_WHITE);
            } else {
                draw_rect(0, y, TREE_W, ROW, COL_BLACK, true);
                draw_rect(0, y, TREE_W - 1, ROW, COL_YELLOW, false);
                char t[48];
                snprintf(t, sizeof t, "%.*s", act.len, act.text);
                int tx = 3 - (act.len > 10 ? (act.len - 10) * FONT_W : 0);     /* the end stays in view */
                draw_clip(1, y, TREE_W - 3, ROW);
                int ex = draw_text(t, tx, y, COL_WHITE);
                if ((frame_count / 20) % 2 == 0) draw_rect(ex, y + 1, 1, ROW - 2, COL_YELLOW, true);
                draw_clip_reset();
            }
            continue;
        }
        bool active = row_active(r);
        if (active) draw_rect(0, y, TREE_W, ROW, r->sec == mode_sec ? UI_RGB(1, 2, 3) : UI_RGB(0, 1, 2), true);
        else if (hover) draw_rect(0, y, TREE_W, ROW, UI_RGB(0, 1, 1), true);
        char label[16];
        snprintf(label, sizeof label, "%.10s%s", r->sec == SEC_CODE ? code_name(r->name) : r->name, row_dirty(r) ? "*" : "");
        draw_text(label, 8, y, active && r->sec == mode_sec ? COL_BLACK : COL_WHITE);
    }
    if (scroll > 0) draw_char('^', TREE_W - 8, TOP, COL_YELLOW);
    if (scroll + shown < nrows) draw_char('v', TREE_W - 8, SETTINGS_Y - ROW, COL_YELLOW);
    {   /* the settings button, bottom left */
        bool on = ui_mode == MODE_SETTINGS, hover = over && in.mouse_y >= SETTINGS_Y;
        draw_rect(0, SETTINGS_Y, TREE_W - 1, ROW, on ? UI_RGB(1, 2, 3) : hover ? UI_RGB(0, 1, 2) : UI_RGB(0, 0, 1), true);
        draw_line(0, SETTINGS_Y - 1, TREE_W - 2, SETTINGS_Y - 1, UI_RGB(0, 1, 2));
        draw_text("settings", 3, SETTINGS_Y, on ? COL_BLACK : hover ? COL_YELLOW : UI_RGB(2, 3, 3));
    }
    draw_line(TREE_W - 1, 0, TREE_W - 1, SCREEN_H - 1, UI_RGB(0, 1, 2));
    if (act.mode == ACT_MENU) {
        static const char *const items[2] = {"rename", "delete"};
        draw_rect(2, my - 1, TREE_W - 5, 2 * ROW + 2, COL_WHITE, false);
        for (int k = 0; k < 2; k++) {
            int iy = my + k * ROW;
            bool hover = over && in.mouse_y >= iy && in.mouse_y < iy + ROW;
            draw_rect(3, iy, TREE_W - 7, ROW, hover ? UI_RGB(1, 2, 3) : UI_RGB(0, 1, 2), true);
            draw_text(items[k], 6, iy, hover ? COL_BLACK : k ? UI_RGB(3, 2, 2) : COL_WHITE);
        }
    }
    if (act.msg[0] && frame_count < act.msg_until) {      /* the last word on a rename / delete */
        int lines = (text_width(act.msg) + TREE_W - 5) / (TREE_W - 4);
        if (lines < 1) lines = 1;
        if (lines > 4) lines = 4;
        int y0 = SCREEN_H - lines * ROW;
        draw_rect(0, y0, TREE_W - 1, lines * ROW, UI_RGB(0, 0, 0), true);
        int per = (TREE_W - 4) / FONT_W;
        for (int k = 0; k < lines; k++) {
            char part[16];
            snprintf(part, sizeof part, "%.*s", per, act.msg + k * per < act.msg + strlen(act.msg) ? act.msg + k * per : "");
            draw_text(part, 2, y0 + k * ROW, act.msg_color);
        }
    }
}

/* the tree has the keyboard while a name is typed or a delete is confirmed */
static bool tree_keys(void) { return tree_open && (act.mode == ACT_RENAME || act.mode == ACT_CONFIRM || act.mode == ACT_MENU); }

static void run_editor(void) {
    switch (ui_mode) {
        case MODE_EDITOR: editor_frame(); break;
        case MODE_TILES: tiles_frame(); break;
        case MODE_MAP: map_frame(); break;
        case MODE_SOUND: sound_frame(); break;
        case MODE_MUSIC: music_frame(); break;
        case MODE_FONT: font_frame(); break;
        case MODE_SETTINGS: settings_frame(); break;
        default: break;
    }
}

void ui_editor_frame(void) {
    docs_bind();
    bool tree = tree_open && cart.loaded;          /* the settings screen can open with no cart */
    int tw = tree ? TREE_W : 0;
    bool in_tree = tree && in.mouse_x < TREE_W;
    if (tree) {
        view_x = 0;
        view_w = TREE_W;
        tree_frame();
    }
    /* the editor: its own coordinates, and no clicks that were meant for the tree */
    view_x = tw;
    view_w = SCREEN_W - tw;
    int mx = in.mouse_x, wheel = in.wheel;
    bool hits[4];
    memcpy(hits, in.mouse_hit, sizeof hits);
    in.mouse_x -= tw;
    if (in_tree) { memset(in.mouse_hit, 0, sizeof in.mouse_hit); in.wheel = 0; }
    int nev = in.nevents;
    bool esc = in.escape;
    if (tree_keys()) { in.nevents = 0; in.escape = false; }
    int mode = ui_mode;
    run_editor();
    if (ui_mode != mode && ui_mode != MODE_SHELL && ui_mode != MODE_BROWSE) {
        /* a key switched editors: show the new one this frame too */
        memset(in.mouse_hit, 0, sizeof in.mouse_hit);
        in.nevents = 0;
        docs_bind();
        run_editor();
    }
    in.nevents = nev;
    in.escape = esc;
    in.mouse_x = mx;
    in.wheel = wheel;
    memcpy(in.mouse_hit, hits, sizeof hits);
    view_x = 0;
    view_w = SCREEN_W;
}
