/* BROWSE: the carts in the carts folder as a list, one row each. Up/Down,
 * PgUp/PgDn, Home/End or the mouse choose, Enter (or a second click) runs,
 * L just loads, Esc goes back. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define MAX_CARTS 128
#define LIST_X 6
#define LIST_Y (FONT_H + 4)
#define ROW_H FONT_H
#define NROWS ((SCREEN_H - FONT_H - 4 - LIST_Y) / ROW_H)   /* rows that fit between the bars */

typedef struct {
    char name[128], title[128];
} Entry;

static Entry *carts;
static int ncarts, sel, top;    /* top: the first cart on screen */
static bool confirm;            /* Enter pressed once with unsaved work */

static int cmp_entry(const void *a, const void *b) { return strcasecmp(((const Entry *)a)->name, ((const Entry *)b)->name); }

static void read_title(const char *dir, char *out, size_t n) {
    char p[PATH_MAX + 16], line[256];
    snprintf(p, sizeof p, "%s/cart.txt", dir);
    FILE *f = fopen(p, "r");
    while (f && fgets(line, sizeof line, f)) {
        char *eq = strchr(line, '=');
        if (!eq || strncasecmp(line, "title", 5)) continue;
        char *v = eq + 1;
        while (*v == ' ') v++;
        v[strcspn(v, "\r\n")] = 0;
        if (*v) snprintf(out, n, "%s", v);
    }
    if (f) fclose(f);
}

void browse_open(void) {
    free(carts);
    carts = calloc(MAX_CARTS, sizeof *carts);
    ncarts = 0;
    DIR *d = opendir(carts_root());
    struct dirent *e;
    while (d && (e = readdir(d)) && ncarts < MAX_CARTS) {
        if (e->d_name[0] == '.') continue;
        char dir[PATH_MAX], p[PATH_MAX + 16];
        snprintf(dir, sizeof dir, "%s/%s", carts_root(), e->d_name);
        snprintf(p, sizeof p, "%s/main.gb", dir);
        struct stat st;
        if (stat(p, &st) != 0) {
            /* a zipped cart, unless it's unpacked already: its title comes from the cache */
            char cname[128];
            CartStub stub;
            if (!archive_name(e->d_name)) continue;
            archive_cart_name(e->d_name, cname, sizeof cname);
            snprintf(p, sizeof p, "%s/%s/main.gb", carts_root(), cname);
            if (stat(p, &st) == 0 || !archive_stub(dir, &stub)) continue;
            Entry *c = &carts[ncarts++];
            snprintf(c->name, sizeof c->name, "%s", e->d_name);
            snprintf(c->title, sizeof c->title, "%s", stub.title[0] ? stub.title : cname);
            continue;
        }
        Entry *c = &carts[ncarts++];
        snprintf(c->name, sizeof c->name, "%s", e->d_name);
        snprintf(c->title, sizeof c->title, "%s", e->d_name);
        read_title(dir, c->title, sizeof c->title);
    }
    if (d) closedir(d);
    qsort(carts, (size_t)ncarts, sizeof *carts, cmp_entry);
    sel = top = 0;
    for (int i = 0; i < ncarts; i++) if (cart.loaded && !strcmp(carts[i].name, cart.name)) sel = i;
    if (sel >= NROWS) top = sel - NROWS + 1;   /* the loaded cart starts on screen */
    confirm = false;
    if (!ncarts) { shell_print("no carts here (try FOLDER)", COL_GREY); return; }
    ui_mode = MODE_BROWSE;
}

static void choose(bool run) {
    if (ui_unsaved() && !confirm) { confirm = true; return; }   /* the screen asks; Enter again discards */
    char cmd[200];
    snprintf(cmd, sizeof cmd, "%s %s", run ? "run" : "load", carts[sel].name);
    ui_mode = MODE_SHELL;
    char echo[220];
    snprintf(echo, sizeof echo, "> %s", cmd);
    shell_print(echo, COL_WHITE);
    shell_exec_discarding(cmd);           /* this screen already asked */
}

static void draw(void) {
    draw_reset();
    draw_cls(UI_RGB(0, 0, 1));
    draw_rect(0, 0, SCREEN_W, FONT_H, UI_RGB(0, 1, 2), true);
    draw_text("CARTS", 3, 0, COL_YELLOW);
    char buf[160];
    snprintf(buf, sizeof buf, "%d/%d  Enter run  L load  Esc back", sel + 1, ncarts);
    draw_text(buf, SCREEN_W - 3 - text_width(buf), 0, UI_RGB(2, 3, 3));
    for (int r = 0; r < NROWS && top + r < ncarts; r++) {
        int i = top + r, y = LIST_Y + r * ROW_H;
        const Entry *c = &carts[i];
        bool on = i == sel;
        if (on) draw_rect(0, y, SCREEN_W, ROW_H, COL_YELLOW, true);
        snprintf(buf, sizeof buf, "%.30s", c->title);
        draw_text(buf, LIST_X, y, on ? COL_BLACK : COL_WHITE);
        if (strcmp(c->title, c->name)) {          /* the folder, when the title differs */
            snprintf(buf, sizeof buf, "%.30s", c->name);
            draw_text(buf, SCREEN_W / 2 + 10, y, on ? UI_RGB(1, 1, 0) : UI_RGB(1, 2, 2));
        }
    }
    if (top > 0) draw_text("^", SCREEN_W - 9, LIST_Y, UI_RGB(2, 3, 3));                       /* more above */
    if (top + NROWS < ncarts) draw_text("v", SCREEN_W - 9, LIST_Y + (NROWS - 1) * ROW_H, UI_RGB(2, 3, 3));
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, SCREEN_W, FONT_H, UI_RGB(0, 1, 2), true);
    if (confirm) draw_text("unsaved changes: Enter again to discard them, Esc to keep", 3, by, COL_ORANGE);
    else {
        snprintf(buf, sizeof buf, "%s  (%s)", carts[sel].title, carts[sel].name);
        draw_text(buf, 3, by, UI_RGB(2, 3, 3));
    }
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void browse_frame(void) {
    if (!ncarts) { ui_mode = MODE_SHELL; return; }
    int old = sel;
    for (int i = 0; i < in.nevents && ui_mode == MODE_BROWSE; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        int before = sel;
        switch (ev->sc) {
            case SDL_SCANCODE_ESCAPE: if (confirm) confirm = false; else ui_mode = MODE_SHELL; break;
            case SDL_SCANCODE_UP: sel--; break;
            case SDL_SCANCODE_DOWN: sel++; break;
            case SDL_SCANCODE_PAGEUP: sel -= NROWS; break;
            case SDL_SCANCODE_PAGEDOWN: sel += NROWS; break;
            case SDL_SCANCODE_HOME: sel = 0; break;
            case SDL_SCANCODE_END: sel = ncarts - 1; break;
            case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: case SDL_SCANCODE_SPACE: choose(true); break;
            case SDL_SCANCODE_L: choose(false); break;
            default: break;
        }
        sel = clampi(sel, 0, ncarts - 1);
        if (sel != before) confirm = false;
    }
    if (ui_mode != MODE_BROWSE) return;
    if (sel != old) {                      /* the keys moved it: keep it on screen */
        if (sel < top) top = sel;
        if (sel >= top + NROWS) top = sel - NROWS + 1;
    }
    top = clampi(top - in.wheel * 3, 0, ncarts > NROWS ? ncarts - NROWS : 0);   /* the wheel scrolls */
    if (in.mouse_hit[1] && in.mouse_y >= LIST_Y) {
        int i = top + (in.mouse_y - LIST_Y) / ROW_H;
        if (i < ncarts && i < top + NROWS) {
            if (sel == i) choose(true);    /* a second click runs it */
            else { sel = i; confirm = false; }
        }
    }
    if (ui_mode != MODE_BROWSE) return;
    draw();
}
