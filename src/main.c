/* gooberengine - boots the console into its shell, optionally running a cart. */
#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void usage(void) {
    fprintf(stderr,
            "GooberEngine %s\n"
            "usage: gooberengine [options] [cart]\n"
            "  cart            a cart name (from the carts folder) or path to a cart folder: run it\n"
            "  --check cart    compile the cart's code only, report errors, no window\n"
            "  --exec \"cmd\"    run a shell command at boot (may repeat)\n"
            "  --scale N       window scale (default 3)\n"
            "  --frames N      quit after N frames\n"
            "  --shot file.png with --frames: save the last frame\n"
            "  --headless      no window; frames run unthrottled (for tests)\n"
            "  --type keys     scripted input, one key per frame: text, {enter}, {ctrl+s}...\n"
            "  --wav file.wav  with --headless: record the sound\n"
            "  --export targets cart [folder]\n"
            "                  make a standalone game, no window: targets are linux, windows,\n"
            "                  web or all (comma separated); folder defaults to exports/<cart>\n"
            "  --help-check    check the manual (F1): every keyword and command has a page\n"
            "  --version\n",
            GE_VERSION);
}

static void cli_log(const char *line, bool error) { fprintf(error ? stderr : stdout, "%s\n", line); }

static int cli_export(const char *targets, const char *cart_arg, const char *folder) {
    char err[512], t[256];
    if (!cart_load(cart_arg, err, sizeof err)) { fprintf(stderr, "%s\n", err); return 1; }
    snprintf(t, sizeof t, "%s", targets);
    for (char *p = t; *p; p++) if (*p == ',') *p = ' ';
    return game_export(t, folder, cli_log) ? 0 : 1;
}

int main(int argc, char **argv) {
#ifdef __EMSCRIPTEN__
    /* the page mounts the browser's storage (IndexedDB) at /persist */
    setenv("GOOBERENGINE_CARTS", "/persist/carts", 0);
    setenv("GOOBERENGINE_SAVES", "/persist/saves", 0);
#endif
#ifdef GE_PLAYER
    return player_main(argc, argv);
#endif
    const char *cartArg = NULL;
    const char *execs[16];
    int nexec = 0;
    bool scale_given = false;       /* --scale wins over the settings */
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        bool more = i + 1 < argc;
        if (!strcmp(a, "--check") && more) return cart_check(argv[++i]);
        else if (!strcmp(a, "--export") && i + 2 < argc)
            return cli_export(argv[i + 1], argv[i + 2], i + 3 < argc && argv[i + 3][0] != '-' ? argv[i + 3] : NULL);
        else if (!strcmp(a, "--exec") && more && nexec < 16) execs[nexec++] = argv[++i];
        else if (!strcmp(a, "--scale") && more) { opt.scale = atoi(argv[++i]); scale_given = true; }
        else if (!strcmp(a, "--frames") && more) opt.max_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--shot") && more) opt.shot = argv[++i];
        else if (!strcmp(a, "--headless")) opt.headless = true;
        else if (!strcmp(a, "--type") && more) opt.script = argv[++i];
        else if (!strcmp(a, "--wav") && more) opt.wav = argv[++i];
        else if (!strcmp(a, "--help-check")) return help_check() ? 1 : 0;
        else if (!strcmp(a, "--version")) { printf("GooberEngine %s\n", GE_VERSION); return 0; }
        else if (!strcmp(a, "-h") || !strcmp(a, "--help")) { usage(); return 0; }
        else if (a[0] == '-') { usage(); return 2; }
        else if (!cartArg) cartArg = a;
        else { usage(); return 2; }
    }
    if (opt.scale < 1) opt.scale = 1;
    if (opt.scale > 8) opt.scale = 8;
    if (opt.headless && opt.max_frames <= 0) opt.max_frames = 60;

    settings_load();
    if (!scale_given) opt.scale = settings.scale;
    if (!platform_init()) return 1;
    platform_set_fullscreen(settings.fullscreen);
    shell_init();
    {   /* boot into a fresh cart with no name: nothing is written to the carts folder until SAVE name */
        char err[512];
        if (cart_new_untitled(err, sizeof err)) shell_print("untitled cart: EDIT to edit, F1 for help, SAVE name to keep it", COL_GREY);
        else shell_print(err, COL_RED);
    }
    for (int i = 0; i < nexec && !in.quit; i++) {
        char echo[512];
        snprintf(echo, sizeof echo, "> %s", execs[i]);
        shell_print(echo, COL_WHITE);
        shell_exec(execs[i]);
    }
    if (cartArg && !in.quit) {
        char cmd[1100];
        snprintf(cmd, sizeof cmd, "run %s", cartArg);
        shell_exec(cmd);
    }
    while (!in.quit && !shell_quit) {
        if (ui_mode == MODE_EDITOR || ui_mode == MODE_TILES || ui_mode == MODE_MAP || ui_mode == MODE_SOUND || ui_mode == MODE_MUSIC || ui_mode == MODE_FONT || ui_mode == MODE_SETTINGS)
            ui_editor_frame();
        else if (ui_mode == MODE_BROWSE) browse_frame();
        else if (ui_mode == MODE_HELP) help_frame();
        else shell_frame();
        platform_frame();
        char dropped[1024];
        bool got = platform_take_dropped(dropped, sizeof dropped);
        size_t dn = got ? strlen(dropped) : 0;
        if (got && dn > 4 && !strcasecmp(dropped + dn - 4, ".png"))
            shell_import_png(dropped, NULL);              /* a picture: into a tileset, in the tile editor */
        else if (got) {                                   /* a zipped cart or cart folder: load it */
            char cmd[1100];
            snprintf(cmd, sizeof cmd, "load %s", dropped);
            ui_mode = MODE_SHELL;
            shell_print(cmd, COL_WHITE);
            shell_exec(cmd);
        }
    }
    if (in.quit && opt.max_frames <= 0) {   /* window closed with unsaved work: keep it beside the cart */
        editor_emergency_save();
        docs_emergency_save();
    }
    if (cart.untitled && in.quit && opt.max_frames <= 0 && ui_unsaved())
        fprintf(stderr, "gooberengine: the untitled cart's unsaved work is kept in %s\n", cart_scratch_dir());
    else cart_discard_scratch();
    platform_shutdown();
    return 0;
}
