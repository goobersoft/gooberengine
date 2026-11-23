/* The console API that cartridges call.
 *
 * GooberBASIC looks up native commands in two tables: gb_core_natives
 * (builtins.c: print, math, strings, lists...) and gb_gfx_natives, which the
 * standalone `goober` binary gets from its SDL layer (gfx.c). GooberEngine
 * links the same compiler and VM but supplies this file in place of gfx.c,
 * so a cart sees the core language plus the console hardware below and
 * nothing else: no windows, no image files, no TTF fonts.
 *
 * Color arguments are palette indices 0-63. Where a color is optional,
 * leaving it out uses the pen color (see pen()). */
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

/* called by the VM on loop back-edges: a cart spinning in a loop with no
 * flip() can still be stopped with Esc. Looks at most every 50 ms. */
void cart_poll_stop(void) {
    static Uint32 last;
    Uint32 now = SDL_GetTicks();
    if (!in_cart || now - last < 50) return;
    last = now;
    if (platform_poll_stop()) { cart_stopped = true; vm_exit(gb_vm, 0); }
}

void gfx_shutdown(void) {}

/* ---- screen ---- */

static Value a_cls(VM *vm, int argc, Value *args) { UNUSED; draw_cls(OPT_I(0, 0)); print_home(); RET_VOID; }
static Value a_pen(VM *vm, int argc, Value *args) { UNUSED; ds.pen = (uint8_t)(ARG_I(0) & 63); RET_VOID; }
static Value a_plot(VM *vm, int argc, Value *args) { UNUSED; draw_plot(ARG_I(0), ARG_I(1), OPT_I(2, -1)); RET_VOID; }
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
/* rect_fill_fast(x, y, w, h [, c]): filled like cls(), so free of the dot limit,
 * depth set to the far plane, no blending */
static Value a_rect_fill_fast(VM *vm, int argc, Value *args) {
    UNUSED;
    draw_rect_fill_fast(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3), OPT_I(4, -1));
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
/* text is drawn with the cart's font in use (the built-in one if its slot is empty) */
static Value a_text(VM *vm, int argc, Value *args) {
    UNUSED;
    ds.font = cur_font;
    int x = draw_text(ARG_S(0), ARG_I(1), ARG_I(2), OPT_I(3, -1));
    ds.font = NULL;
    return v_int(x);
}
/* print: text at a cursor that starts top-left and moves a line down each time.
 * It still writes the line to stdout too, as GooberBASIC's own print does. */
static int print_x, print_y;
void print_home(void) { print_x = print_y = 0; }
static Value a_print(VM *vm, int argc, Value *args) {
    UNUSED;
    const char *s = argc > 0 ? ARG_S(0) : "";
    if (argc > 1) print_x = ARG_I(1);
    if (argc > 2) print_y = ARG_I(2);
    ds.font = cur_font;
    draw_text(s, print_x, print_y, OPT_I(3, -1));
    int lines = 1;
    for (const char *p = s; *p; p++) lines += *p == '\n';
    print_y += lines * (cur_font ? cur_font->h : sys_font.h);
    ds.font = NULL;
    if (argc > 0) fwrite(s, 1, (size_t)str_len(args[0].as.s), stdout);
    fputc('\n', stdout);
    fflush(stdout);
    RET_VOID;
}
static Value a_text_width(VM *vm, int argc, Value *args) {
    UNUSED;
    ds.font = cur_font;
    int w = text_width(ARG_S(0));
    ds.font = NULL;
    return v_int(w);
}
/* the size of a character of the font in use: text steps this far across and down */
static Value a_font_w(VM *vm, int argc, Value *args) { UNUSED; return v_int(cur_font ? cur_font->w : sys_font.w); }
static Value a_font_h(VM *vm, int argc, Value *args) { UNUSED; return v_int(cur_font ? cur_font->h : sys_font.h); }

static Value a_rgb(VM *vm, int argc, Value *args) {
    UNUSED;
    int r = ARG_I(0), g = ARG_I(1), b = ARG_I(2);
    if (r < 0 || r > 3 || g < 0 || g > 3 || b < 0 || b > 3) vm_error(vm, "rgb: each channel is 0 to 3, got %d, %d, %d", r, g, b);
    return v_int(RGB6(r, g, b));
}

/* pal(a, b): from now on, drawing in color a puts color b. pal() puts every color back */
static Value a_pal(VM *vm, int argc, Value *args) {
    UNUSED;
    if (argc < 2) { for (int i = 0; i < NCOLORS; i++) ds.pal[i] = (uint8_t)i; RET_VOID; }
    int a = ARG_I(0), b = ARG_I(1);
    if (a < 0 || a >= NCOLORS || b < 0 || b >= NCOLORS) vm_error(vm, "pal: colors are 0-63, got %d, %d", a, b);
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

/* ---- depth, blending and the extra drawing commands ---- */

/* set_depth([z]): draw at depth z from now on (-32768..32767, lower is nearer).
 * A pixel is only drawn where the depth buffer is z or further. cls() clears the
 * buffer to the far plane (32767) and the drawing depth to 0 */
static Value a_set_depth(VM *vm, int argc, Value *args) {
    UNUSED;
    int z = OPT_I(0, 0);
    if (z < INT16_MIN || z > INT16_MAX) vm_error(vm, "set_depth: %d out of range %d to %d", z, INT16_MIN, INT16_MAX);
    draw_set_depth(z);
    RET_VOID;
}

static Value a_set_blend(VM *vm, int argc, Value *args) {
    UNUSED;
    int b = OPT_I(0, BLEND_NORMAL);
    if (b < 0 || b >= NBLENDS) vm_error(vm, "set_blend: %d is not a blend mode (use the BLEND_ constants)", b);
    ds.blend = (uint8_t)b;
    RET_VOID;
}

/* spray draws from the same random numbers as rnd(), so seed_rnd() repeats it */
static NativeFn rnd_fn;
static double spray_rng(void) { return rnd_fn(gb_vm, 0, NULL).as.f; }

static Value a_spray(VM *vm, int argc, Value *args) {
    UNUSED;
    int n = ARG_I(4);
    if (n < 0) vm_error(vm, "spray: %d dots", n);
    if (n > DOT_LIMIT_MAX) n = DOT_LIMIT_MAX;      /* more could never be drawn in one frame */
    if (!rnd_fn) rnd_fn = native_get(native_find("rnd"))->fn;
    draw_spray(ARG_I(0), ARG_I(1), ARG_I(2), ARG_I(3), n, OPT_I(5, -1), spray_rng);
    RET_VOID;
}

static void need_block(VM *vm, const char *what, int n, int w, int h) {
    if (n < 0 || n >= NTILES) vm_error(vm, "%s: tile %d out of range 0-%d", what, n, NTILES - 1);
    if (w < 1 || h < 1) vm_error(vm, "%s: a %d x %d tile block", what, w, h);
}

/* tex_quad(x0,y0, x1,y1, x2,y2, x3,y3, spr, sw, sh): corners top-left,
 * top-right, bottom-right, bottom-left; spr's sw x sh block of tiles is
 * stretched across them */
static Value a_tex_quad(VM *vm, int argc, Value *args) {
    UNUSED;
    need_block(vm, "tex_quad", ARG_I(8), ARG_I(9), ARG_I(10));
    int px[4], py[4];
    for (int i = 0; i < 4; i++) { px[i] = ARG_I(2 * i); py[i] = ARG_I(2 * i + 1); }
    draw_tex_quad(px, py, NULL, ARG_I(8), ARG_I(9), ARG_I(10));
    RET_VOID;
}

/* tex_quad_3d(x0,y0,z0, ... x3,y3,z3, spr, sw, sh): the same, with each
 * corner's depth: x, y go straight to the screen and z into the depth buffer */
static Value a_tex_quad_3d(VM *vm, int argc, Value *args) {
    UNUSED;
    need_block(vm, "tex_quad_3d", ARG_I(12), ARG_I(13), ARG_I(14));
    int px[4], py[4], pz[4];
    for (int i = 0; i < 4; i++) { px[i] = ARG_I(3 * i); py[i] = ARG_I(3 * i + 1); pz[i] = ARG_I(3 * i + 2); }
    draw_tex_quad(px, py, pz, ARG_I(12), ARG_I(13), ARG_I(14));
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

static Value a_tile_plot(VM *vm, int argc, Value *args) {
    UNUSED;
    int n = ARG_I(0), x = ARG_I(1), y = ARG_I(2), c = ARG_I(3);
    need_tile(vm, "tile_plot", n);
    Tileset *ts = need_tileset(vm, "tile_plot");
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
 * The same for maps, sfx, music and fonts. swap_*() alone returns the slot in use. ---- */

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
static Value a_load_font(VM *vm, int argc, Value *args) { slot_load(vm, RES_FONT, "load_font", argc, args); RET_VOID; }
static Value a_swap_font(VM *vm, int argc, Value *args) { return slot_swap(vm, RES_FONT, "swap_font", argc, args); }
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
/* mouse_btn numbers from 0 (left, right, middle); in.mouse[] is 1-based like MOUSE_LEFT.. */
static Value a_mouse_btn(VM *vm, int argc, Value *args) { UNUSED; int b = ARG_I(0); return v_int(b >= 0 && b <= 2 && in.mouse[b + 1]); }
/* SDL counts the wheel + = up; carts get + = down (scrolling down the page) */
static Value a_mouse_wheel(VM *vm, int argc, Value *args) { UNUSED; return v_int(-in.wheel); }
static Value a_mouse_dx(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.mouse_dx); }
static Value a_mouse_dy(VM *vm, int argc, Value *args) { UNUSED; return v_int(in.mouse_dy); }

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
    if (w < 0 || w > WAVE_NOISE) vm_error(vm, "note: wave %d out of range 0-%d (8-15 = INSTRUMENT + sfx 0-7, %d = NOISE)", w, WAVE_NOISE, WAVE_NOISE);
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

static Value a_flip(VM *vm, int argc, Value *args) {
    UNUSED;
    cart_flipped = true;
    net_poll();
    platform_frame();
    dots_used = 0;              /* a new frame, a new dot budget */
    if (in.quit || in.escape) { cart_stopped = true; vm_exit(vm, 0); }
    RET_VOID;
}
static Value a_frame_count(VM *vm, int argc, Value *args) { UNUSED; return v_int((int32_t)frame_count); }
static Value a_cpu(VM *vm, int argc, Value *args) { UNUSED; return v_int(platform_frame_load()); }
static Value a_cart_title(VM *vm, int argc, Value *args) { UNUSED; return v_str(str_from_c(vm, cart.title)); }

NativeDef gb_gfx_natives[] = {
    {"cls", 'v', "|i", a_cls},
    {"pen", 'v', "i", a_pen},
    {"plot", 'v', "ii|i", a_plot},
    {"pget", 'i', "ii", a_pget},
    {"line", 'v', "iiii|i", a_line},
    {"rect", 'v', "iiii|i", a_rect},
    {"rect_fill", 'v', "iiii|i", a_rect_fill},
    {"rect_fill_fast", 'v', "iiii|i", a_rect_fill_fast},
    {"circle", 'v', "iii|i", a_circle},
    {"circle_fill", 'v', "iii|i", a_circle_fill},
    {"text", 'i', "sii|i", a_text},
    {"text_width", 'i', "s", a_text_width},
    {"print", 'v', "|siii", a_print},
    {"font_w", 'i', "", a_font_w},
    {"font_h", 'i', "", a_font_h},
    {"rgb", 'i', "iii", a_rgb},
    {"camera", 'v', "|ii", a_camera},
    {"pal", 'v', "|ii", a_pal},
    {"clip", 'v', "|iiii", a_clip},
    {"set_depth", 'v', "|i", a_set_depth},
    {"set_blend", 'v', "|i", a_set_blend},
    {"spray", 'v', "iiiii|i", a_spray},
    {"tex_quad", 'v', "iiiiiiiiiii", a_tex_quad},
    {"tex_quad_3d", 'v', "iiiiiiiiiiiiiii", a_tex_quad_3d},

    {"sprite", 'v', "iii|iii", a_sprite},
    {"tile_pget", 'i', "iii", a_tile_pget},
    {"tile_plot", 'v', "iiii", a_tile_plot},
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
    {"load_font", 'v', "i|s", a_load_font},
    {"swap_font", 'i', "|i", a_swap_font},
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
    {"mouse_btn", 'i', "i", a_mouse_btn},
    {"mouse_wheel", 'i', "", a_mouse_wheel},
    {"mouse_dx", 'i', "", a_mouse_dx},
    {"mouse_dy", 'i', "", a_mouse_dy},

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
    {"cpu", 'i', "", a_cpu},
    {"cart_title", 's', "", a_cart_title},
    {NULL, 0, NULL, NULL},
};

#define K(name, sc) {"KEY_" #name, SDL_SCANCODE_##sc}
ConstDef gb_gfx_consts[] = {
    {"SCREEN_W", SCREEN_W}, {"SCREEN_H", SCREEN_H}, {"TILE_SIZE", TILE},
    {"SHEET_COLS", SHEET_COLS}, {"NTILES", NTILES}, {"SLOTS", NSLOTS},
    {"MAP_SCREEN_W", SCREEN_CELLS_W}, {"MAP_SCREEN_H", SCREEN_CELLS_H}, {"FONT_W", FONT_W}, {"FONT_H", FONT_H},
    {"FLIP_X", 1}, {"FLIP_Y", 2},
    {"BLEND_NORMAL", BLEND_NORMAL}, {"BLEND_ADD", BLEND_ADD}, {"BLEND_SUB", BLEND_SUB},
    {"BLEND_MAX", BLEND_MAX}, {"BLEND_MIN", BLEND_MIN}, {"BLEND_XOR", BLEND_XOR},
    {"NOISE", WAVE_NOISE}, {"INSTRUMENT", WAVE_INSTR}, {"VOICES", NVOICES},

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
