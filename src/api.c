/* The console API that cartridges call.
 *
 * GooberBASIC looks up native commands in two tables: gb_core_natives
 * (builtins.c: print, math, strings, lists...) and gb_gfx_natives, which the
 * standalone `goober` binary gets from its SDL layer (gfx.c). GooberEngine
 * links the same compiler and VM but supplies this file in place of gfx.c,
 * so a cart sees the core language plus the console hardware below and
 * nothing else: no windows, no image files, no TTF fonts.
 *
 * Colour arguments are palette indices 0-63. Where a colour is optional,
 * leaving it out uses the pen colour (see pen()). */
#include "engine.h"
#include "vm.h"
#include <SDL2/SDL.h>

#define ARG_I(n) (args[n].as.i)
#define ARG_S(n) (str_chars(args[n].as.s))
#define OPT_I(n, d) (argc > (n) ? ARG_I(n) : (d))
#define RET_VOID return v_int(0)
#define UNUSED (void)vm; (void)argc; (void)args

extern bool cart_flipped, cart_stopped;

/* ---- the VM's hooks into the platform ---- */

/* called by delay(): keep the window alive and honour Esc / close */
void gfx_pump_events(void) {
    if (!in_cart) return;
    platform_frame();
    frame_count--;     /* a delay is not a frame */
    if (in.quit || in.escape) { cart_stopped = true; vm_exit(gb_vm, 0); }
}

void gfx_shutdown(void) {}

/* ---- screen ---- */

static Value a_cls(VM *vm, int argc, Value *args) { UNUSED; draw_cls(OPT_I(0, 0)); RET_VOID; }
static Value a_pen(VM *vm, int argc, Value *args) { UNUSED; ds.pen = (uint8_t)(ARG_I(0) & 63); RET_VOID; }
static Value a_pset(VM *vm, int argc, Value *args) { UNUSED; draw_pset(ARG_I(0), ARG_I(1), OPT_I(2, -1)); RET_VOID; }
static Value a_pget(VM *vm, int argc, Value *args) { UNUSED; return v_int(draw_pget(ARG_I(0), ARG_I(1))); }
static Value a_line(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_line(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3), OPT_I(4, -1));
    RET_VOID;
}
static Value a_rect(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_rect(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3), OPT_I(4, -1), false);
    RET_VOID;
}
static Value a_rect_fill(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_rect(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3), OPT_I(4, -1), true);
    RET_VOID;
}
static Value a_circle(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_circle(ARG_I(0), ARG_I(1), ARG_I(2), OPT_I(3, -1), false);
    RET_VOID;
}
static Value a_circle_fill(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_circle(ARG_I(0), ARG_I(1), ARG_I(2), OPT_I(3, -1), true);
    RET_VOID;
}
static Value a_text(VM *vm, int argc, Value *args) {
    UNUSED;
    return v_int(draw_text(ARG_S(0), ARG_I(1), ARG_I(2), OPT_I(3, -1)));
}
static Value a_text_width(VM *vm, int argc, Value *args) { UNUSED; return v_int(text_width(ARG_S(0))); }

static Value a_rgb(VM *vm, int argc, Value *args) {
    UNUSED;
    int r = ARG_I(0), g = ARG_I(1), b = ARG_I(2);
    if (r < 0 || r > 3 || g < 0 || g > 3 || b < 0 || b > 3) vm_error(vm, "rgb: each channel is 0 to 3, got %d, %d, %d", r, g, b);
    return v_int(RGB6(r, g, b));
}

/* pal(a, b): from now on, drawing in colour a puts colour b. pal() puts every colour back */
static Value a_pal(VM *vm, int argc, Value *args) {
    UNUSED;
    if (argc < 2) { for (int i = 0; i < NCOLORS; i++) ds.pal[i] = (uint8_t)i; RET_VOID; }
    int a = ARG_I(0), b = ARG_I(1);
    if (a < 0 || a >= NCOLORS || b < 0 || b >= NCOLORS) vm_error(vm, "pal: colours are 0-63, got %d, %d", a, b);
    ds.pal[a] = (uint8_t)b;
    RET_VOID;
}

static Value a_camera(VM *vm, int argc, Value *args) { UNUSED; ds.cam_x = OPT_I(0, 0); ds.cam_y = OPT_I(1, 0); RET_VOID; }

static Value a_clip(VM *vm, int argc, Value *args) {
    UNUSED;
    if (argc < 4) draw_clip_reset();
    else draw_clip(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3));
    RET_VOID;
}

/* ---- tiles, sprites and the map ---- */

static void need_tile(VM *vm, const char *what, int n) {
    if (n < 0 || n >= NTILES) vm_error(vm, "%s: tile %d out of range 0-%d", what, n, NTILES - 1);
}

static Value a_sprite(VM *vm, int argc, Value *args) {
    UNUSED;
    need_tile(vm, "sprite", ARG_I(0));
    draw_sprite(ARG_I(0), ARG_I(1), ARG_I(2), OPT_I(3, 1), OPT_I(4, 1), OPT_I(5, 0));
    RET_VOID;
}

static Tileset *need_tileset(VM *vm, const char *what) {
    if (!cur_tiles) vm_error(vm, "%s: tileset slot %d is empty (load_tileset(%d, \"name\") first)", what, rt_current(RES_TILES), rt_current(RES_TILES));
    return cur_tiles;
}

static Value a_tile_pget(VM *vm, int argc, Value *args) {
    UNUSED;
    int n = ARG_I(0), x = ARG_I(1), y = ARG_I(2);
    need_tile(vm, "tile_pget", n);
    if (!cur_tiles || x < 0 || y < 0 || x >= TILE || y >= TILE) return v_int(-1);
    uint8_t c = cur_tiles->t[n][y * TILE + x];
    return v_int(c == CLEAR ? -1 : c);
}

static Value a_tile_pset(VM *vm, int argc, Value *args) {
    UNUSED;
    int n = ARG_I(0), x = ARG_I(1), y = ARG_I(2), c = ARG_I(3);
    need_tile(vm, "tile_pset", n);
    Tileset *ts = need_tileset(vm, "tile_pset");
    if (x < 0 || y < 0 || x >= TILE || y >= TILE) RET_VOID;
    ts->t[n][y * TILE + x] = c < 0 ? CLEAR : (uint8_t)(c & 63);
    RET_VOID;
}

static Value a_map_draw(VM *vm, int argc, Value *args) {
    UNUSED;
    int w = cur_map ? cur_map->w : 0, h = cur_map ? cur_map->h : 0;
    draw_map(OPT_I(0, 0), OPT_I(1, 0), OPT_I(2, 0), OPT_I(3, 0), OPT_I(4, w), OPT_I(5, h));
    RET_VOID;
}

/* an empty cell (or one outside the map) reads as -1; tile 0 is a tile */
static Value a_map_get(VM *vm, int argc, Value *args) {
    UNUSED;
    uint16_t t = map_cell(cur_map, ARG_I(0), ARG_I(1));
    return v_int(t == MAP_EMPTY ? -1 : t);
}

static Value a_map_set(VM *vm, int argc, Value *args) {
    UNUSED;
    int x = ARG_I(0), y = ARG_I(1), t = ARG_I(2);
    if (t != -1) need_tile(vm, "map_set", t);
    if (!cur_map) vm_error(vm, "map_set: map slot %d is empty (load_map(%d, \"name\") first)", rt_current(RES_MAP), rt_current(RES_MAP));
    if (x >= 0 && y >= 0 && x < cur_map->w && y < cur_map->h) cur_map->cells[y * cur_map->w + x] = t < 0 ? MAP_EMPTY : (uint16_t)t;
    RET_VOID;
}

static Value a_map_w(VM *vm, int argc, Value *args) { UNUSED; return v_int(cur_map ? cur_map->w : 0); }
static Value a_map_h(VM *vm, int argc, Value *args) { UNUSED; return v_int(cur_map ? cur_map->h : 0); }

/* ---- slots: load_tileset(slot, "name") loads tiles/name.g64 into a slot,
 * load_tileset(slot) empties it, swap_tileset(slot) makes it the one in use.
 * The same for maps, sfx and music. swap_*() alone returns the slot in use. ---- */

static void slot_load(VM *vm, int kind, const char *what, int argc, Value *args) {
    char err[512];
    if (!rt_load(kind, ARG_I(0), argc > 1 ? ARG_S(1) : NULL, err, sizeof err)) vm_error(vm, "%s: %s", what, err);
}

static Value slot_swap(VM *vm, int kind, const char *what, int argc, Value *args) {
    char err[512];
    if (argc > 0 && !rt_swap(kind, ARG_I(0), err, sizeof err)) vm_error(vm, "%s: %s", what, err);
    return v_int(rt_current(kind));
}

static Value a_load_tileset(VM *vm, int argc, Value *args) { slot_load(vm, RES_TILES, "load_tileset", argc, args); RET_VOID; }
static Value a_load_map(VM *vm, int argc, Value *args) { slot_load(vm, RES_MAP, "load_map", argc, args); RET_VOID; }
static Value a_load_sfx(VM *vm, int argc, Value *args) { slot_load(vm, RES_SFX, "load_sfx", argc, args); RET_VOID; }
static Value a_load_music(VM *vm, int argc, Value *args) { slot_load(vm, RES_MUSIC, "load_music", argc, args); RET_VOID; }
static Value a_swap_tileset(VM *vm, int argc, Value *args) { return slot_swap(vm, RES_TILES, "swap_tileset", argc, args); }
static Value a_swap_map(VM *vm, int argc, Value *args) { return slot_swap(vm, RES_MAP, "swap_map", argc, args); }
static Value a_swap_sfx(VM *vm, int argc, Value *args) { return slot_swap(vm, RES_SFX, "swap_sfx", argc, args); }
static Value a_swap_music(VM *vm, int argc, Value *args) {
    audio_lock();
    synth_music_stop();      /* a new song starts with music() */
    audio_unlock();
    return slot_swap(vm, RES_MUSIC, "swap_music", argc, args);
}

/* ---- the dot limit ---- */

static Value a_dot_limit(VM *vm, int argc, Value *args) {
    UNUSED;
    if (argc > 0) {
        int n = ARG_I(0);
        if (n < 0 || n > DOT_LIMIT_MAX) vm_error(vm, "dot_limit: %d out of range 0-%d", n, DOT_LIMIT_MAX);
        dot_limit = n;
    }
    return v_int(dot_limit);
}

static Value a_dots_used(VM *vm, int argc, Value *args) { UNUSED; return v_int(dots_used); }

/* ---- input ---- */

static int need_btn(VM *vm, int b) {
    if (b < 0 || b >= NBUTTONS) vm_error(vm, "button %d does not exist (use the BTN_ constants)", b);
    return b;
}
static Value a_btn(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.btn[need_btn(vm, ARG_I(0))]); }
static Value a_btn_hit(VM *vm, int argc, Value *args) {
    UNUSED;
    int b = need_btn(vm, ARG_I(0));
    return v_int(in.btn[b] && !in.btn_prev[b]);
}
static int need_key(VM *vm, int k) {
    if (k < 0 || k >= 512) vm_error(vm, "key %d does not exist (use the KEY_ constants)", k);
    return k;
}
static Value a_key_down(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.key[need_key(vm, ARG_I(0))]); }
static Value a_key_hit(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.key_hit[need_key(vm, ARG_I(0))]); }
static Value a_mouse_x(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.mouse_x); }
static Value a_mouse_y(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.mouse_y); }
static Value a_mouse_down(VM *vm, int argc, Value *args) { UNUSED; int b = ARG_I(0); return v_int(b >= 1 && b <= 3 && in.mouse[b]); }
static Value a_mouse_hit(VM *vm, int argc, Value *args) { UNUSED; int b = ARG_I(0); return v_int(b >= 1 && b <= 3 && in.mouse_hit[b]); }

/* ---- sound ---- */

static Value a_sfx(VM *vm, int argc, Value *args) {
    UNUSED;
    int n = ARG_I(0), v = OPT_I(1, -1);
    if (n < 0 || n >= NSFX) vm_error(vm, "sfx: %d out of range 0-%d", n, NSFX - 1);
    if (v < -1 || v >= NVOICES) vm_error(vm, "sfx: voice %d out of range 0-%d", v, NVOICES - 1);
    audio_lock();
    int got = synth_sfx(n, v);
    audio_unlock();
    return v_int(got);
}

static Value a_note(VM *vm, int argc, Value *args) {
    UNUSED;
    int v = ARG_I(0), p = ARG_I(1), w = ARG_I(2), vol = ARG_I(3), ticks = OPT_I(4, 0);
    if (v < 0 || v >= NVOICES) vm_error(vm, "note: voice %d out of range 0-%d", v, NVOICES - 1);
    if (p < 0 || p >= NPITCH) vm_error(vm, "note: pitch %d out of range 0-%d", p, NPITCH - 1);
    if (w < 0 || w > WAVE_NOISE) vm_error(vm, "note: wave %d out of range 0-%d (%d = NOISE)", w, WAVE_NOISE, WAVE_NOISE);
    if (vol < 0 || vol > 7) vm_error(vm, "note: volume %d out of range 0-7", vol);
    audio_lock();
    synth_note(v, p, w, vol, ticks);
    audio_unlock();
    RET_VOID;
}

static Value a_voice_stop(VM *vm, int argc, Value *args) {
    UNUSED;
    int v = OPT_I(0, -1);
    if (v < -1 || v >= NVOICES) vm_error(vm, "voice_stop: voice %d out of range 0-%d", v, NVOICES - 1);
    audio_lock();
    synth_stop(v);
    audio_unlock();
    RET_VOID;
}

static Value a_voice_sfx(VM *vm, int argc, Value *args) {
    UNUSED;
    int v = ARG_I(0);
    if (v < 0 || v >= NVOICES) vm_error(vm, "voice_sfx: voice %d out of range 0-%d", v, NVOICES - 1);
    audio_lock();
    int r = synth_voice_sfx(v);
    audio_unlock();
    return v_int(r);
}

static Value a_music(VM *vm, int argc, Value *args) {
    UNUSED;
    int pos = OPT_I(0, 0);
    if (pos < 0 || pos >= ORDER_LEN) vm_error(vm, "music: order entry %d out of range 0-%d", pos, ORDER_LEN - 1);
    audio_lock();
    synth_music(pos, -1);
    audio_unlock();
    RET_VOID;
}

static Value a_music_stop(VM *vm, int argc, Value *args) {
    UNUSED;
    audio_lock();
    synth_music_stop();
    audio_unlock();
    RET_VOID;
}

static Value a_music_pos(VM *vm, int argc, Value *args) {
    UNUSED;
    audio_lock();
    int p = synth_music_pos();
    audio_unlock();
    return v_int(p);
}

static Value a_music_row(VM *vm, int argc, Value *args) {
    UNUSED;
    audio_lock();
    int r = synth_music_pos() < 0 ? -1 : synth_music_row();
    audio_unlock();
    return v_int(r);
}

/* ---- save data ---- */

static void need_save_key(VM *vm, const char *what, const char *key) {
    if (!save_key_ok(key)) vm_error(vm, "%s: key '%s' must be 1-64 letters, digits, _ - or .", what, key);
}

static Value a_save_data(VM *vm, int argc, Value *args) {
    UNUSED;
    need_save_key(vm, "save_data", ARG_S(0));
    const char *err = save_store(ARG_S(0), ARG_S(1));
    if (err) vm_error(vm, "save_data: %s", err);
    RET_VOID;
}

static Value a_load_data(VM *vm, int argc, Value *args) {
    UNUSED;
    need_save_key(vm, "load_data", ARG_S(0));
    char *v = save_load(ARG_S(0));
    Value r = v_str(str_from_c(vm, v ? v : argc > 1 ? ARG_S(1) : ""));
    free(v);
    return r;
}

/* ---- system ---- */

/* F8 while a cart runs: this frame becomes the cart's label */
static void check_label_key(void) {
    if (opt.player || !in.key_hit[SDL_SCANCODE_F8]) return;
    char err[256];
    if (label_capture(err, sizeof err)) label_taken = true;
    else fprintf(stderr, "gooberengine: %s\n", err);
}

static Value a_flip(VM *vm, int argc, Value *args) {
    UNUSED;
    cart_flipped = true;
    check_label_key();          /* the frame being shown is complete */
    platform_frame();
    dots_used = 0;              /* a new frame, a new dot budget */
    if (in.quit || in.escape) { cart_stopped = true; vm_exit(vm, 0); }
    RET_VOID;
}
static Value a_frame_count(VM *vm, int argc, Value *args) { UNUSED; return v_int((int32_t)frame_count); }
static Value a_cart_title(VM *vm, int argc, Value *args) { UNUSED; return v_str(str_from_c(vm, cart.title)); }

NativeDef gb_gfx_natives[] = {
    {"cls", 'v', "|i", a_cls},
    {"pen", 'v', "i", a_pen},
    {"pset", 'v', "ii|i", a_pset},
    {"pget", 'i', "ii", a_pget},
    {"line", 'v', "iiii|i", a_line},
    {"rect", 'v', "iiii|i", a_rect},
    {"rect_fill", 'v', "iiii|i", a_rect_fill},
    {"circle", 'v', "iii|i", a_circle},
    {"circle_fill", 'v', "iii|i", a_circle_fill},
    {"text", 'i', "sii|i", a_text},
    {"text_width", 'i', "s", a_text_width},
    {"rgb", 'i', "iii", a_rgb},
    {"camera", 'v', "|ii", a_camera},
    {"pal", 'v', "|ii", a_pal},
    {"clip", 'v', "|iiii", a_clip},

    {"sprite", 'v', "iii|iii", a_sprite},
    {"tile_pget", 'i', "iii", a_tile_pget},
    {"tile_pset", 'v', "iiii", a_tile_pset},
    {"map_draw", 'v', "|iiiiii", a_map_draw},
    {"map_get", 'i', "ii", a_map_get},
    {"map_set", 'v', "iii", a_map_set},
    {"map_w", 'i', "", a_map_w},
    {"map_h", 'i', "", a_map_h},
    {"load_tileset", 'v', "i|s", a_load_tileset},
    {"swap_tileset", 'i', "|i", a_swap_tileset},
    {"load_map", 'v', "i|s", a_load_map},
    {"swap_map", 'i', "|i", a_swap_map},
    {"load_sfx", 'v', "i|s", a_load_sfx},
    {"swap_sfx", 'i', "|i", a_swap_sfx},
    {"load_music", 'v', "i|s", a_load_music},
    {"swap_music", 'i', "|i", a_swap_music},
    {"dot_limit", 'i', "|i", a_dot_limit},
    {"dots_used", 'i', "", a_dots_used},

    {"btn", 'i', "i", a_btn},
    {"btn_hit", 'i', "i", a_btn_hit},
    {"key_down", 'i', "i", a_key_down},
    {"key_hit", 'i', "i", a_key_hit},
    {"mouse_x", 'i', "", a_mouse_x},
    {"mouse_y", 'i', "", a_mouse_y},
    {"mouse_down", 'i', "i", a_mouse_down},
    {"mouse_hit", 'i', "i", a_mouse_hit},

    {"sfx", 'i', "i|i", a_sfx},
    {"note", 'v', "iiii|i", a_note},
    {"voice_stop", 'v', "|i", a_voice_stop},
    {"voice_sfx", 'i', "i", a_voice_sfx},
    {"music", 'v', "|i", a_music},
    {"music_stop", 'v', "", a_music_stop},
    {"music_pos", 'i', "", a_music_pos},
    {"music_row", 'i', "", a_music_row},

    {"save_data", 'v', "ss", a_save_data},
    {"load_data", 's', "s|s", a_load_data},

    {"flip", 'v', "", a_flip},
    {"frame_count", 'i', "", a_frame_count},
    {"cart_title", 's', "", a_cart_title},
    {NULL, 0, NULL, NULL},
};

#define K(name, sc) {"KEY_" #name, SDL_SCANCODE_##sc}
ConstDef gb_gfx_consts[] = {
    {"SCREEN_W", SCREEN_W}, {"SCREEN_H", SCREEN_H}, {"TILE_SIZE", TILE},
    {"SHEET_COLS", SHEET_COLS}, {"NTILES", NTILES}, {"SLOTS", NSLOTS},
    {"MAP_SCREEN_W", SCREEN_CELLS_W}, {"MAP_SCREEN_H", SCREEN_CELLS_H}, {"FONT_W", FONT_W}, {"FONT_H", FONT_H},
    {"FLIP_X", 1}, {"FLIP_Y", 2},
    {"NOISE", WAVE_NOISE}, {"VOICES", NVOICES},

    {"BTN_LEFT", BTN_LEFT}, {"BTN_RIGHT", BTN_RIGHT}, {"BTN_UP", BTN_UP}, {"BTN_DOWN", BTN_DOWN},
    {"BTN_A", BTN_A}, {"BTN_B", BTN_B}, {"BTN_C", BTN_C}, {"BTN_START", BTN_START},

    {"COL_BLACK", COL_BLACK}, {"COL_WHITE", COL_WHITE}, {"COL_GREY", COL_GREY},
    {"COL_DARK_GREY", COL_DARK_GREY}, {"COL_RED", COL_RED}, {"COL_GREEN", COL_GREEN},
    {"COL_BLUE", COL_BLUE}, {"COL_YELLOW", COL_YELLOW}, {"COL_CYAN", COL_CYAN},
    {"COL_MAGENTA", COL_MAGENTA}, {"COL_ORANGE", COL_ORANGE}, {"COL_BROWN", COL_BROWN},
    {"COL_PINK", COL_PINK}, {"COL_PURPLE", COL_PURPLE}, {"COL_NAVY", COL_NAVY},
    {"COL_DARK_GREEN", COL_DARK_GREEN}, {"COL_SKY", COL_SKY}, {"COL_LIME", COL_LIME},

    K(A, A), K(B, B), K(C, C), K(D, D), K(E, E), K(F, F), K(G, G), K(H, H), K(I, I),
    K(J, J), K(K, K), K(L, L), K(M, M), K(N, N), K(O, O), K(P, P), K(Q, Q), K(R, R),
    K(S, S), K(T, T), K(U, U), K(V, V), K(W, W), K(X, X), K(Y, Y), K(Z, Z),
    K(0, 0), K(1, 1), K(2, 2), K(3, 3), K(4, 4), K(5, 5), K(6, 6), K(7, 7), K(8, 8), K(9, 9),
    K(SPACE, SPACE), K(ENTER, RETURN), K(TAB, TAB), K(BACKSPACE, BACKSPACE),
    K(LEFT, LEFT), K(RIGHT, RIGHT), K(UP, UP), K(DOWN, DOWN),
    K(LSHIFT, LSHIFT), K(RSHIFT, RSHIFT), K(LCONTROL, LCTRL), K(RCONTROL, RCTRL),
    {"MOUSE_LEFT", 1}, {"MOUSE_RIGHT", 2}, {"MOUSE_MIDDLE", 3},
    {NULL, 0},
};
