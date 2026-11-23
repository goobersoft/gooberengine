/* The file tree: the cart's files down the left side of every editor.
 *
 *   code      +        click a file to edit it; + makes a new one
 *   - main
 *   1 enemies          the number at the left: the slot the file loads into
 *     scratch          by itself when the game runs. Click there to put a
 *   tilesets  +        file in the lowest free slot or take it out again;
 *   0 tileset*         right-click there for the next slot (swapping with
 *   2 cave             the file in it). Slotted files come first, by slot.
 *     tiles-1          load_tileset() and friends still work on top of that.
 *   maps      +        main.gb always runs (-); slotted code is compiled in
 *   ...                ahead of it, in slot order, as if main included it.
 *
 * The file each editor has open is highlighted; * = changed, not saved.
 *
 * Right-click a file for rename (type the new name, Enter) or delete (Y to
 * confirm). main.gb can't be renamed or deleted: a cart starts there.
 *
 * F9 (or the button at the left of the tabs) shows and hides it. The editor
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
#define GAP 1                           /* the separator line between one section and the next */
#define SETTINGS_Y (SCREEN_H - ROW)     /* the settings button: the tree's bottom row */

enum { SEC_CODE = NRES };          /* sections: the resource kinds, then code */
static const int sec_order[] = {SEC_CODE, RES_TILES, RES_MAP, RES_SFX, RES_MUSIC, RES_FONT};
static const char *const sec_title[] = {"tilesets", "maps", "sfx", "music", "fonts", "code"};
static const int sec_mode[] = {MODE_TILES, MODE_MAP, MODE_SOUND, MODE_MUSIC, MODE_FONT, MODE_EDITOR};

typedef struct { int sec; char name[64]; bool header; bool dirty; int slot; } Row;   /* dirty: a header's section has an unsaved file; slot -1: none */
#define GUTTER 8                   /* the slot number's column, left of a file's name */
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

/* the cart's .gb files: on disk, plus any the editor has open that aren't saved yet */
static int code_files(char names[][64], int max) {
    int n = 0;
    DIR *d = opendir(cart.dir);
    struct dirent *e;
    while (d && (e = readdir(d)) && n < max) {
        size_t len = strlen(e->d_name);
        if (len > 3 && len < 64 && !strcmp(e->d_name + len - 3, ".gb") && e->d_name[0] != '.') snprintf(names[n++], 64, "%s", e->d_name);
    }
    if (d) closedir(d);
    const char *open[64];
    int nopen = editor_open_files(open, 64);
    for (int k = 0; k < nopen && n < max; k++) {
        bool have = false;
        for (int i = 0; i < n && !have; i++) have = !strcmp(names[i], open[k]);
        if (!have) snprintf(names[n++], 64, "%s", open[k]);
    }
    char *ptr[64];
    for (int i = 0; i < n; i++) ptr[i] = names[i];
    qsort(ptr, (size_t)n, sizeof ptr[0], cmp_str);
    static char sorted[64][64];
    for (int i = 0; i < n; i++) snprintf(sorted[i], 64, "%s", ptr[i]);
    memcpy(names, sorted, sizeof sorted[0] * (size_t)n);
    return n;
}

static const char *slot_key(const Row *r) { return r->sec == SEC_CODE ? code_name(r->name) : r->name; }
static bool is_main(const Row *r) { return r->sec == SEC_CODE && !strcmp(r->name, "main.gb"); }

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
        *h = (Row){sec, "", true, false, -1};
        Row files[MAX_DOCS * 2];
        int order[MAX_DOCS * 2];   /* main.gb, then the slotted files by slot, then the rest by name */
        for (int i = 0; i < n; i++) {
            Row *r = &files[i];
            *r = (Row){sec, "", false, false, -1};
            snprintf(r->name, sizeof r->name, "%s", names[i]);
            r->slot = cart_slot_of(sec, slot_key(r));
            order[i] = is_main(r) ? -1 : r->slot >= 0 ? r->slot : NSLOTS + i;
            if (row_dirty(r)) h->dirty = true;
        }
        for (int i = 1; i < n; i++)            /* a handful of files: insertion sort */
            for (int j = i; j > 0 && order[j - 1] > order[j]; j--) {
                int o = order[j]; order[j] = order[j - 1]; order[j - 1] = o;
                Row t = files[j]; files[j] = files[j - 1]; files[j - 1] = t;
            }
        for (int i = 0; i < n && !folded[sec] && nrows < MAX_ROWS; i++) rows[nrows++] = files[i];
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
            Row r = {sec, "", false, false, -1};
            snprintf(r.name, sizeof r.name, "%s", name);
            open_row(&r);
            return;
        }
        snprintf(name, sizeof name, "%s-%d", stem[sec], i);
        char err[256];
        Doc *d = doc_find(sec, name) ? NULL : doc_create(sec, name, err, sizeof err);
        if (!d) continue;
        Row r = {sec, "", false, false, -1};
        snprintf(r.name, sizeof r.name, "%s", name);
        open_row(&r);
        return;
    }
}

/* a click on a file's slot number: into the lowest free slot / out of its slot;
 * next (a right-click): on to the next slot, swapping with the file there */
static void slot_click(const Row *r, bool next) {
    const char *name = slot_key(r);
    if (is_main(r)) return;            /* main.gb always runs: it has no slot to change */
    int cur = cart_slot_of(r->sec, name), to;
    if (cur < 0) {
        to = cart_slot_free(r->sec);
        if (to < 0) { say(COL_RED, "every slot is taken"); return; }
    } else to = next ? (cur + 1) % NSLOTS : -1;
    char err[256];                     /* success is silent: the row's slot column shows it */
    if (!cart_slot_set(r->sec, name, to, err, sizeof err)) say(COL_RED, err);
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
        cart_slot_rename(KIND_CODE, code_name(r->name), code_name(to));
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
        cart_slot_forget(KIND_CODE, code_name(r->name));
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

/* every section but the first has a separator above it, so positions are sums of heights */
static int row_h(int i) { return rows[i].header && i > 0 ? ROW + GAP : ROW; }
static int row_y(int i) {                  /* the screen y of row i (at or below the scroll) */
    int y = TOP;
    for (int j = scroll; j < i; j++) y += row_h(j);
    return y;
}
static int row_at(int y) {                 /* the row under screen y, or nrows */
    int ry = TOP;
    for (int i = scroll; i < nrows; i++) {
        if (y < ry + row_h(i)) return i;
        ry += row_h(i);
    }
    return nrows;
}
static int rows_fit(int from) {            /* how many rows from `from` fit above the settings button */
    int n = 0, h = 0;
    while (from + n < nrows && h + row_h(from + n) <= SETTINGS_Y - TOP) h += row_h(from + n++);
    return n;
}

static void tree_frame(void) {
    build();
    bool over = in.mouse_x >= 0 && in.mouse_x < TREE_W;
    if (over && in.wheel) scroll -= in.wheel * 2;
    int max_scroll = nrows, h = 0;             /* the last scroll that still fills the view */
    while (max_scroll > 0 && h + row_h(max_scroll - 1) <= SETTINGS_Y - TOP) h += row_h(--max_scroll);
    if (scroll > max_scroll) scroll = max_scroll;
    if (scroll < 0) scroll = 0;
    int shown = rows_fit(scroll);

    if (act.mode != ACT_NONE) act_keys();          /* Esc closes the menu too */
    int arow = act.mode ? row_index(&act.row) : -1;
    if (act.mode && arow < 0) act.mode = ACT_NONE;          /* the file went away */
    int ay = arow >= scroll ? row_y(arow) : TOP - ROW * (scroll - arow);                      /* the menu sits below its row */
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
    if (over && in.mouse_hit[2] && in.mouse_y >= TOP && in.mouse_y < SETTINGS_Y) {
        int i = row_at(in.mouse_y);
        if (i < nrows && !rows[i].header) {
            if (in.mouse_x < GUTTER) slot_click(&rows[i], true);
            else open_menu(&rows[i]);
        }
    }
    if (over && in.mouse_hit[1]) {
        if (in.mouse_y < FONT_H) { if (in.mouse_x >= TREE_W - 10) tree_open = false; }
        else if (in.mouse_y >= SETTINGS_Y) {
            if (ui_mode == MODE_SETTINGS) settings_frame_close();
            else settings_open();
        } else {
            int i = row_at(in.mouse_y);
            if (in.mouse_y >= TOP && i < nrows) {
                if (rows[i].header) {
                    if (in.mouse_x >= TREE_W - 10) { folded[rows[i].sec] = false; new_file(rows[i].sec); }   /* the new file shows */
                    else folded[rows[i].sec] = !folded[rows[i].sec];
                }
                else if (in.mouse_x < GUTTER) slot_click(&rows[i], false);
                else open_row(&rows[i]);
                build();
            }
        }
    }

    /* a click above may have just opened a menu: place it on its row this frame, not the next */
    arow = act.mode ? row_index(&act.row) : -1;
    if (act.mode && arow < 0) act.mode = ACT_NONE;
    ay = arow >= scroll ? row_y(arow) : TOP - ROW * (scroll - arow);
    my = ay + ROW + 2 * ROW > SCREEN_H ? ay - 2 * ROW : ay + ROW;

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
        int y = row_y(i);
        bool hover = over && in.mouse_y >= y && in.mouse_y < y + row_h(i);
        if (r->header) {
            if (i > 0) { draw_rect(0, y, TREE_W, GAP, UI_RGB(0, 1, 2), true); y += GAP; }   /* the separator */
            bool f = folded[r->sec], name_hover = hover && in.mouse_x < TREE_W - 10;
            int c = r->sec == mode_sec ? COL_YELLOW : name_hover ? COL_WHITE : UI_RGB(2, 2, 3);
            int tc = name_hover ? COL_YELLOW : UI_RGB(1, 2, 2);
            for (int k = 0; k < 3; k++) {      /* a small triangle: pointing right when folded, down when open */
                if (f) draw_line(2 + k, y + 3 + k, 2 + k, y + 7 - k, tc);
                else draw_line(2 + k, y + 4 + k, 6 - k, y + 4 + k, tc);
            }
            int x = draw_text(sec_title[r->sec], 9, y, c);
            if (f && r->dirty) draw_text("*", x + 1, y, UI_RGB(1, 2, 2));   /* folded: * if a file in it isn't saved */
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
        if (active && r->sec == mode_sec) draw_rect(0, y, TREE_W, ROW, UI_RGB(1, 2, 3), true);
        else if (hover) draw_rect(0, y, TREE_W, ROW, UI_RGB(0, 1, 1), true);
        char label[16];
        snprintf(label, sizeof label, "%.10s%s", slot_key(r), row_dirty(r) ? "*" : "");
        bool dark = active && r->sec == mode_sec;           /* black text on the bright highlight */
        draw_text(label, GUTTER, y, dark ? COL_BLACK : COL_WHITE);
        bool on_gutter = hover && in.mouse_x < GUTTER && !is_main(r);     /* main's dash isn't a button */
        if (on_gutter) draw_rect(0, y, GUTTER - 1, ROW, UI_RGB(1, 1, 2), true);
        if (is_main(r)) draw_char('-', 1, y, dark ? COL_BLACK : UI_RGB(1, 2, 2));
        else if (r->slot >= 0) draw_char('0' + r->slot, 1, y, on_gutter ? COL_YELLOW : dark ? COL_BLACK : COL_ORANGE);
        else if (on_gutter) draw_char('+', 1, y, COL_YELLOW);
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
    if (ui_mode != mode && ui_mode != MODE_SHELL && ui_mode != MODE_BROWSE && ui_mode != MODE_HELP) {
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
