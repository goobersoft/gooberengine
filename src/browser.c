/* BROWSE: the carts in the carts folder as a grid of labels. Arrows or the
 * mouse choose, Enter (or a second click) runs, L just loads, Esc goes back. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define COLS 3
#define ROWS 2
#define CELL_W 130
#define CELL_H 104
#define GRID_X ((SCREEN_W - COLS * CELL_W) / 2)
#define GRID_Y 22
#define MAX_CARTS 128

typedef struct {
    char name[128], title[128];
    bool has_label;
    uint8_t label[LABEL_H][LABEL_W];
} Entry;

static Entry *carts;
static int ncarts, sel, page;
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
        if (stat(p, &st) != 0) continue;
        Entry *c = &carts[ncarts++];
        snprintf(c->name, sizeof c->name, "%s", e->d_name);
        snprintf(c->title, sizeof c->title, "%s", e->d_name);
        read_title(dir, c->title, sizeof c->title);
        c->has_label = label_read(dir, c->label);
    }
    if (d) closedir(d);
    qsort(carts, (size_t)ncarts, sizeof *carts, cmp_entry);
    sel = page = 0;
    for (int i = 0; i < ncarts; i++) if (cart.loaded && !strcmp(carts[i].name, cart.name)) sel = i;
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
    draw_cls(RGB6(0, 0, 1));
    draw_rect(0, 0, SCREEN_W, FONT_H, RGB6(0, 1, 2), true);
    draw_text("CARTS", 3, 0, COL_YELLOW);
    char buf[128];
    int pages = (ncarts + COLS * ROWS - 1) / (COLS * ROWS);
    snprintf(buf, sizeof buf, "page %d/%d  Enter run  L load  Esc back", page + 1, pages);
    draw_text(buf, SCREEN_W - 3 - text_width(buf), 0, RGB6(2, 3, 3));
    for (int k = 0; k < COLS * ROWS; k++) {
        int i = page * COLS * ROWS + k;
        if (i >= ncarts) break;
        Entry *c = &carts[i];
        int x = GRID_X + (k % COLS) * CELL_W + (CELL_W - LABEL_W) / 2, y = GRID_Y + (k / COLS) * CELL_H;
        if (i == sel) draw_rect(x - 3, y - 3, LABEL_W + 6, LABEL_H + 6, COL_YELLOW, true);
        if (c->has_label) {
            for (int ly = 0; ly < LABEL_H; ly++)
                for (int lx = 0; lx < LABEL_W; lx++) draw_pset(x + lx, y + ly, c->label[ly][lx]);
        } else {                      /* no label yet: a plain card with the cart's initial */
            draw_rect(x, y, LABEL_W, LABEL_H, RGB6(0, 1, 2), true);
            char ch[2] = {(char)(c->title[0] ? c->title[0] : '?'), 0};
            draw_text(ch, x + LABEL_W / 2 - 3, y + LABEL_H / 2 - 5, RGB6(1, 2, 3));
            draw_text("F8 makes a label", x + 2, y + LABEL_H - 11, RGB6(1, 1, 2));
        }
        snprintf(buf, sizeof buf, "%.21s", c->title);
        draw_text(buf, x + (LABEL_W - text_width(buf)) / 2, y + LABEL_H + 5, i == sel ? COL_WHITE : RGB6(2, 2, 3));
    }
    int by = SCREEN_H - FONT_H;
    draw_rect(0, by, SCREEN_W, FONT_H, RGB6(0, 1, 2), true);
    if (confirm) draw_text("unsaved changes: Enter again to discard them, Esc to keep", 3, by, COL_ORANGE);
    else {
        snprintf(buf, sizeof buf, "%s  (%s)", carts[sel].title, carts[sel].name);
        draw_text(buf, 3, by, RGB6(2, 3, 3));
    }
}

void browse_frame(void) {
    if (!ncarts) { ui_mode = MODE_SHELL; return; }
    int per = COLS * ROWS;
    for (int i = 0; i < in.nevents && ui_mode == MODE_BROWSE; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        int old = sel;
        switch (ev->sc) {
            case SDL_SCANCODE_ESCAPE: if (confirm) confirm = false; else ui_mode = MODE_SHELL; break;
            case SDL_SCANCODE_LEFT: if (sel > 0) sel--; break;
            case SDL_SCANCODE_RIGHT: if (sel < ncarts - 1) sel++; break;
            case SDL_SCANCODE_UP: if (sel >= COLS) sel -= COLS; break;
            case SDL_SCANCODE_DOWN: if (sel + COLS < ncarts) sel += COLS; break;
            case SDL_SCANCODE_PAGEUP: sel = sel >= per ? sel - per : 0; break;
            case SDL_SCANCODE_PAGEDOWN: sel = sel + per < ncarts ? sel + per : ncarts - 1; break;
            case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: case SDL_SCANCODE_SPACE: choose(true); break;
            case SDL_SCANCODE_L: choose(false); break;
            default: break;
        }
        if (sel != old) confirm = false;
    }
    if (ui_mode != MODE_BROWSE) return;
    if (in.mouse_hit[1])
        for (int k = 0; k < per; k++) {
            int i = page * per + k;
            int x = GRID_X + (k % COLS) * CELL_W, y = GRID_Y + (k / COLS) * CELL_H;
            if (i < ncarts && in.mouse_x >= x && in.mouse_x < x + CELL_W && in.mouse_y >= y && in.mouse_y < y + CELL_H) {
                if (sel == i) choose(true);   /* a second click runs it */
                else { sel = i; confirm = false; }
                break;
            }
        }
    if (ui_mode != MODE_BROWSE) return;
    page = sel / per;
    draw();
}
