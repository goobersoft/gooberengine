/* The settings screen (F9, the settings button at the bottom of the file
 * tree, or SETTINGS in the shell): the editors' theme, the sound levels,
 * fullscreen, the window's size and the code editor's indentation. Every change is used at once and kept in
 * settings.txt (settings.c).
 *
 *   theme        blue  green  orange  grey
 *   volume       ##########  10
 *   music        ##########  10
 *   sound fx     ##########  10
 *   fullscreen   off
 *   window size  < 3x >
 *   indent       smart  auto  none
 *
 * Up / Down pick a line, Left / Right (or clicks) change it, Esc or F9 goes
 * back to where you were. */
#include "engine.h"
#include <SDL2/SDL_scancode.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define TOP FONT_H
#define BOT (SCREEN_H - FONT_H)
#define ROWS_Y (TOP + 16)
#define ROW_H 18
#define LABEL_X 10
#define CTRL_X 86
#define CELL 8                  /* a volume step */

enum { S_THEME, S_VOLUME, S_MUSIC, S_SFX, S_FULLSCREEN, S_SCALE, S_INDENT, NSET };
static const char *const labels[NSET] = {"theme", "volume", "music", "sound fx", "fullscreen", "window size", "indent"};
static const char *const hints[NSET] = {
    "the colours of the shell and the editors",
    "all sound",
    "the music, on top of the volume",
    "sound effects and notes, on top of the volume",
    "the whole screen (F11 too)",
    "screen pixels for each console pixel",
    "Enter in the code editor: smart (after if, for ...), auto (keep it), none",
};

static struct {
    int row;
    int back;                   /* the mode to go back to */
    char status[128];
    int status_color;
    long status_until;
} S = {0, MODE_SHELL, "", 0, 0};

static void status(int color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void status(int color, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(S.status, sizeof S.status, fmt, ap);
    va_end(ap);
    S.status_color = color;
    S.status_until = frame_count + 4 * FPS;
    if (opt.headless) printf("[settings] %s\n", S.status);
}

void settings_open(void) {
    if (ui_mode == MODE_SETTINGS) return;
    S.back = ui_mode;
    ui_mode = MODE_SETTINGS;
}

void settings_frame_close(void) {
    ui_mode = S.back == MODE_SETTINGS ? MODE_SHELL : S.back;
    if (ui_mode != MODE_SHELL && !cart.loaded) ui_mode = MODE_SHELL;
}

static int *level(int row) {
    return row == S_VOLUME ? &settings.volume : row == S_MUSIC ? &settings.music_volume : row == S_SFX ? &settings.sfx_volume : NULL;
}

static void save(void) {
    char err[256];
    if (!settings_save(err, sizeof err)) status(COL_RED, "%s", err);
}

/* a short note at the new level, so you hear it */
static void preview(int row) {
    if (row != S_VOLUME && row != S_SFX) return;     /* the music level is heard with the music */
    audio_lock();
    synth_note(NVOICES - 1, 33, 3, 5, 14);
    audio_unlock();
}

/* change a line by `d` steps (Left -1, Right +1; Enter cycles) */
static void change(int row, int d) {
    switch (row) {
        case S_THEME: ui_theme = (ui_theme + d + NTHEMES) % NTHEMES; status(ui_col(COL_SKY), "theme: %s", theme_names[ui_theme]); break;
        case S_VOLUME: case S_MUSIC: case S_SFX: {
            int *v = level(row);
            int n = *v + d;
            if (n < 0 || n > VOLUME_MAX) return;
            *v = n;
            save();
            preview(row);
            status(ui_col(COL_SKY), "%s: %d", labels[row], n);
            return;
        }
        case S_FULLSCREEN:
            settings.fullscreen = !settings.fullscreen;
            platform_set_fullscreen(settings.fullscreen);
            status(ui_col(COL_SKY), "fullscreen: %s", settings.fullscreen ? "on" : "off");
            break;
        case S_SCALE: {
            int n = settings.scale + d;
            if (n < 1 || n > 8) return;
            settings.scale = n;
            platform_set_scale(n);
            status(ui_col(COL_SKY), "window size: %dx", n);
            break;
        }
        case S_INDENT:
            settings.indent = (settings.indent + d + NINDENTS) % NINDENTS;
            status(ui_col(COL_SKY), "indent: %s", indent_names[settings.indent]);
            break;
    }
    save();
}

static void set_level(int row, int n) {
    int *v = level(row);
    if (!v || n == *v) return;
    *v = n < 0 ? 0 : n > VOLUME_MAX ? VOLUME_MAX : n;
    save();
    preview(row);
    status(ui_col(COL_SKY), "%s: %d", labels[row], *v);
}

static void key(const KeyEvent *ev) {
    switch (ev->sc) {
        case SDL_SCANCODE_ESCAPE: settings_frame_close(); return;
        case SDL_SCANCODE_UP: S.row = (S.row + NSET - 1) % NSET; return;
        case SDL_SCANCODE_DOWN: S.row = (S.row + 1) % NSET; return;
        case SDL_SCANCODE_LEFT: change(S.row, -1); return;
        case SDL_SCANCODE_RIGHT: change(S.row, 1); return;
        case SDL_SCANCODE_RETURN: case SDL_SCANCODE_SPACE:
            if (level(S.row)) return;
            change(S.row, 1);
            return;
        default: return;
    }
}

/* ---------------------------------------------------------------- the parts of a line */

static bool inside(int x, int y, int w, int h) {
    return in.mouse_x >= x && in.mouse_x < x + w && in.mouse_y >= y && in.mouse_y < y + h;
}

static int row_y(int row) { return ROWS_Y + row * ROW_H; }

/* a row of named buttons (the themes, the indent modes): where button t is */
static int name_btn(const char *const *names, int t, int *w) {
    int x = CTRL_X;
    for (int i = 0; i < t; i++) x += text_width(names[i]) + 10;
    *w = text_width(names[t]) + 6;
    return x;
}

static int themed(int t, int r, int g, int b) {
    int keep = ui_theme;
    ui_theme = t;
    int c = ui_rgb(r, g, b);
    ui_theme = keep;
    return c;
}

/* a small button: < or > */
static bool arrow(int x, int y, char c, bool click) {
    bool over = inside(x, y, 9, FONT_H);
    draw_rect(x, y, 9, FONT_H, over ? UI_RGB(1, 2, 3) : UI_RGB(0, 1, 2), true);
    draw_char(c, x + (9 - FONT_INK_W) / 2, y, over ? COL_BLACK : UI_RGB(2, 3, 3));
    return over && click;
}

static void draw_row(int row, bool click) {
    int y = row_y(row), cy = y + 3;
    bool on = S.row == row;
    if (on) draw_rect(4, y, UI_W - 8, ROW_H - 2, UI_RGB(0, 0, 1), true);
    if (inside(4, y, UI_W - 8, ROW_H - 2) && click) S.row = row;
    draw_text(labels[row], LABEL_X, cy, on ? COL_YELLOW : UI_RGB(2, 3, 3));
    switch (row) {
        case S_THEME:
            for (int t = 0; t < NTHEMES; t++) {
                int w, x = name_btn(theme_names, t, &w);
                bool sel = ui_theme == t, over = inside(x - 2, cy - 1, w, FONT_H + 2);
                draw_rect(x - 2, cy - 1, w, FONT_H + 2, sel ? themed(t, 1, 2, 3) : themed(t, 0, 1, 2), true);
                if (over && !sel) draw_rect(x - 3, cy - 2, w + 2, FONT_H + 4, COL_YELLOW, false);
                draw_text(theme_names[t], x + 1, cy, sel ? COL_BLACK : themed(t, 2, 3, 3));
                if (over && click && !sel) { ui_theme = t; save(); status(ui_col(COL_SKY), "theme: %s", theme_names[t]); }
            }
            break;
        case S_VOLUME: case S_MUSIC: case S_SFX: {
            int v = *level(row);
            for (int i = 0; i < VOLUME_MAX; i++) {
                int x = CTRL_X + i * CELL;
                draw_rect(x, cy + 1, CELL - 2, FONT_H - 3, i < v ? (on ? COL_YELLOW : UI_RGB(1, 2, 3)) : UI_RGB(0, 1, 2), true);
            }
            /* a click on step i sets the level to i + 1; left of the first step, 0 */
            if (click && inside(CTRL_X - 8, cy - 1, VOLUME_MAX * CELL + 8, FONT_H + 2)) {
                int i = (in.mouse_x - CTRL_X) / CELL;
                set_level(row, in.mouse_x < CTRL_X ? 0 : i + 1);
            }
            char n[8];
            snprintf(n, sizeof n, "%d", v);
            draw_text(n, CTRL_X + VOLUME_MAX * CELL + 6, cy, UI_RGB(2, 3, 3));
            break;
        }
        case S_FULLSCREEN: {
            const char *t = settings.fullscreen ? "on" : "off";
            int w = text_width("off") + 6;
            bool over = inside(CTRL_X - 2, cy - 1, w, FONT_H + 2);
            draw_rect(CTRL_X - 2, cy - 1, w, FONT_H + 2, settings.fullscreen ? UI_RGB(1, 2, 3) : UI_RGB(0, 1, 2), true);
            if (over) draw_rect(CTRL_X - 3, cy - 2, w + 2, FONT_H + 4, COL_YELLOW, false);
            draw_text(t, CTRL_X + 1, cy, settings.fullscreen ? COL_BLACK : UI_RGB(2, 3, 3));
            if (over && click) change(row, 1);
            break;
        }
        case S_SCALE: {
            if (arrow(CTRL_X - 2, cy, '<', click)) change(row, -1);
            char t[16];
            snprintf(t, sizeof t, "%dx", settings.scale);
            draw_text(t, CTRL_X + 12, cy, UI_RGB(2, 3, 3));
            if (arrow(CTRL_X + 14 + text_width("8x"), cy, '>', click)) change(row, 1);
            if (settings.fullscreen) draw_text("(when not fullscreen)", CTRL_X + 40, cy, UI_RGB(1, 2, 2));
            break;
        }
        case S_INDENT:
            for (int t = 0; t < NINDENTS; t++) {
                int w, x = name_btn(indent_names, t, &w);
                bool sel = settings.indent == t, over = inside(x - 2, cy - 1, w, FONT_H + 2);
                draw_rect(x - 2, cy - 1, w, FONT_H + 2, sel ? UI_RGB(1, 2, 3) : UI_RGB(0, 1, 2), true);
                if (over && !sel) draw_rect(x - 3, cy - 2, w + 2, FONT_H + 4, COL_YELLOW, false);
                draw_text(indent_names[t], x + 1, cy, sel ? COL_BLACK : UI_RGB(2, 3, 3));
                if (over && click && !sel) change(row, t - settings.indent);
            }
            break;
    }
}

static void draw(void) {
    draw_reset();
    draw_cls(COL_BLACK);
    bool click = in.mouse_hit[1];
    draw_text("settings", LABEL_X, TOP + 3, COL_WHITE);
    for (int r = 0; r < NSET; r++) draw_row(r, click);
    int hy = row_y(NSET) + 6;
    draw_text("up/down: choose   left/right: change   Esc: back", LABEL_X, hy, UI_RGB(1, 2, 2));

    if (cart.loaded) ui_draw_tabs();
    else {
        draw_rect(0, 0, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
        draw_text("settings", 3, 0, UI_RGB(2, 3, 3));
    }
    draw_rect(0, BOT, UI_W, FONT_H, UI_RGB(0, 1, 2), true);
    if (S.status[0] && frame_count < S.status_until) draw_text(S.status, 3, BOT, S.status_color);
    else draw_text(hints[S.row], 3, BOT, UI_RGB(2, 3, 3));
}

void settings_frame(void) {
    settings.fullscreen = platform_fullscreen();       /* F11 may have changed it */
    for (int i = 0; i < in.nevents && ui_mode == MODE_SETTINGS; i++) {
        const KeyEvent *ev = &in.events[i];
        if (ev->type != EV_KEY) continue;
        if (ui_global_key(ev)) continue;
        key(ev);
    }
    if (ui_mode != MODE_SETTINGS) return;
    draw();
}
