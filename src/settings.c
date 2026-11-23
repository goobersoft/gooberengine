/* The console's settings: the editors' colour theme, the sound levels,
 * fullscreen and the window's size. The settings screen (settings_screen.c)
 * changes them; the THEME command changes the theme.
 *
 * They're kept in settings.txt, beside the save-data folder:
 *   $GOOBERENGINE_SETTINGS, else the folder above the saves folder /settings.txt
 *   (~/.local/share/gooberengine/settings.txt; in the browser /persist/settings.txt)
 * The file is "key = value" lines:
 *   theme = blue | green | orange | grey
 *   volume = 0-10          all sound
 *   music_volume = 0-10    the music, on top of that
 *   sfx_volume = 0-10      sound effects and notes, on top of that
 *   fullscreen = yes | no
 *   scale = 1-8            the window: pixels per console pixel
 *   indent = smart | auto | none   the code editor's Enter: smart indents after
 *                          if, for ...; auto keeps the line's indentation; none adds none
 *   tile_right = eyedrop | colour_b   the tile editor's right button: picks a
 *                          colour up into A, or draws with colour B
 *   fx_crt = 0-100         post-processing (postfx.c): CRT screen bending
 *   fx_tint = 0-359        hue shift, in degrees
 *   fx_saturation = 0-100  100 is the console's own colours, 0 grey
 *   fx_scanlines = 0-100   100 makes the gaps between lines black (not at 1x)
 *   fx_burn = 0-80         screen burn: lit pixels fade out instead of snapping off
 * Lines it doesn't know are kept when it's written. An exported game doesn't
 * read it: its player's settings belong to whoever plays it. */
#include "engine.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

int ui_theme = THEME_BLUE;
const char *const theme_names[NTHEMES] = {"blue", "green", "orange", "grey"};
const char *const indent_names[NINDENTS] = {"smart", "auto", "none"};
const char *const tile_right_names[NRIGHTS] = {"eyedrop", "colour_b"};
Settings settings = {VOLUME_MAX, VOLUME_MAX, VOLUME_MAX, false, 3, INDENT_SMART, .fx_saturation = 100};

static const char *const keys[] = {"theme", "volume", "music_volume", "sfx_volume", "fullscreen", "scale", "indent", "tile_right",
                                   "fx_crt", "fx_tint", "fx_saturation", "fx_scanlines", "fx_burn"};
#define NKEYS (int)(sizeof keys / sizeof keys[0])

static void settings_path(char *out, size_t n) {
    const char *env = getenv("GOOBERENGINE_SETTINGS");
    if (env && *env) { snprintf(out, n, "%s", env); return; }
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", save_dir());
    char *slash = strrchr(dir, '/');
    if (slash && slash != dir) *slash = 0;
    snprintf(out, n, "%s/settings.txt", dir);
}

int theme_find(const char *name) {
    for (int i = 0; i < NTHEMES; i++) if (!strcasecmp(name, theme_names[i])) return i;
    if (!strcasecmp(name, "gray") || !strcasecmp(name, "white")) return THEME_GREY;
    return -1;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) *--e = 0;
    return s;
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* the sound levels as the synth wants them */
static void apply_volume(void) {
    audio_lock();
    synth_set_volume(settings.volume / (float)VOLUME_MAX, settings.music_volume / (float)VOLUME_MAX, settings.sfx_volume / (float)VOLUME_MAX);
    audio_unlock();
}

void settings_load(void) {
    char path[PATH_MAX + 32], line[512];
    settings_path(path, sizeof path);
    FILE *f = fopen(path, "r");
    while (f && fgets(line, sizeof line, f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *k = trim(line), *v = trim(eq + 1);
        int n = atoi(v);
        if (!strcasecmp(k, "theme")) { int t = theme_find(v); if (t >= 0) ui_theme = t; }
        else if (!strcasecmp(k, "volume")) settings.volume = clampi(n, 0, VOLUME_MAX);
        else if (!strcasecmp(k, "music_volume")) settings.music_volume = clampi(n, 0, VOLUME_MAX);
        else if (!strcasecmp(k, "sfx_volume")) settings.sfx_volume = clampi(n, 0, VOLUME_MAX);
        else if (!strcasecmp(k, "fullscreen")) settings.fullscreen = !strcasecmp(v, "yes") || !strcasecmp(v, "on") || n == 1;
        else if (!strcasecmp(k, "scale")) settings.scale = clampi(n, 1, 8);
        else if (!strcasecmp(k, "indent")) {
            for (int i = 0; i < NINDENTS; i++) if (!strcasecmp(v, indent_names[i])) settings.indent = i;
        }
        else if (!strcasecmp(k, "tile_right")) {
            for (int i = 0; i < NRIGHTS; i++) if (!strcasecmp(v, tile_right_names[i])) settings.tile_right = i;
        }
        else if (!strcasecmp(k, "fx_crt")) settings.fx_crt = clampi(n, 0, 100);
        else if (!strcasecmp(k, "fx_tint")) settings.fx_tint = ((n % 360) + 360) % 360;
        else if (!strcasecmp(k, "fx_saturation")) settings.fx_saturation = clampi(n, 0, 100);
        else if (!strcasecmp(k, "fx_scanlines")) settings.fx_scanlines = clampi(n, 0, 100);
        else if (!strcasecmp(k, "fx_burn")) settings.fx_burn = clampi(n, 0, FX_BURN_MAX);
    }
    if (f) fclose(f);
    apply_volume();
}

bool settings_save(char *err, int errlen) {
    apply_volume();
    char path[PATH_MAX + 32], tmp[PATH_MAX + 48], line[512];
    settings_path(path, sizeof path);
    char dir[PATH_MAX + 32];
    snprintf(dir, sizeof dir, "%s", path);
    for (char *p = dir + 1; *p; p++)                 /* make the folders on the way */
        if (*p == '/') { *p = 0; mkdir(dir, 0755); *p = '/'; }
    snprintf(tmp, sizeof tmp, "%s.saving", path);
    FILE *out = fopen(tmp, "w");
    if (!out) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    FILE *in_f = fopen(path, "r");
    while (in_f && fgets(line, sizeof line, in_f)) {  /* keep the lines this doesn't write */
        char copy[512], *eq;
        snprintf(copy, sizeof copy, "%s", line);
        bool ours = false;
        if ((eq = strchr(copy, '='))) {
            *eq = 0;
            for (int i = 0; i < NKEYS; i++) if (!strcasecmp(trim(copy), keys[i])) ours = true;
        }
        if (!ours) fputs(line, out);
    }
    if (in_f) fclose(in_f);
    fprintf(out, "theme = %s\nvolume = %d\nmusic_volume = %d\nsfx_volume = %d\nfullscreen = %s\nscale = %d\nindent = %s\ntile_right = %s\n",
            theme_names[ui_theme], settings.volume, settings.music_volume, settings.sfx_volume,
            settings.fullscreen ? "yes" : "no", settings.scale, indent_names[settings.indent],
            tile_right_names[settings.tile_right]);
    fprintf(out, "fx_crt = %d\nfx_tint = %d\nfx_saturation = %d\nfx_scanlines = %d\nfx_burn = %d\n",
            settings.fx_crt, settings.fx_tint, settings.fx_saturation, settings.fx_scanlines, settings.fx_burn);
    if (fclose(out) != 0 || rename(tmp, path) != 0) { snprintf(err, (size_t)errlen, "could not write %s", path); return false; }
    return true;
}

bool theme_set(int theme, char *err, int errlen) {
    ui_theme = theme;
    return settings_save(err, errlen);
}
