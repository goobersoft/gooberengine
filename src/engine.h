/* GooberEngine - the fixed virtual machine every cartridge runs on.
 *
 * Hardware (v1):
 *   screen   400 x 240 pixels, one byte per pixel
 *   colours  64 fixed colours, 6-bit RGB (2 bits each of red, green, blue)
 *   tiles    a tileset is a 400 x 240 sheet: 960 tiles of 10 x 10 pixels, 40 per row
 *   maps     1-10 screens (40 x 24 cells) each way, one tile number per cell
 *   slots    10 slots each for tilesets, maps, sound banks and songs (load_*, swap_*)
 *   dots     at most dot_limit() pixels drawn per frame (default 100,000)
 *   text     6 x 10 font cells (66 x 24 characters on screen), 5-wide letters
 *   input    pad: d-pad + A B C + Start (keyboard or game controller), keyboard, mouse
 *   frame    60 frames per second
 */
#ifndef GE_ENGINE_H
#define GE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "compat.h"

#define GE_VERSION "0.1.0"

#define SCREEN_W 400
#define SCREEN_H 240
#define NCOLORS 64
#define TILE 10
#define NTILES 960
#define SHEET_COLS 40
#define SHEET_ROWS 24
#define SHEET_W (SHEET_COLS * TILE)
#define SHEET_H (SHEET_ROWS * TILE)
#define SCREEN_CELLS_W (SCREEN_W / TILE)  /* a map screen: 40 x 24 cells */
#define SCREEN_CELLS_H (SCREEN_H / TILE)
#define MAP_MAX_SCREENS 10
#define MAP_MAX_W (SCREEN_CELLS_W * MAP_MAX_SCREENS)
#define MAP_MAX_H (SCREEN_CELLS_H * MAP_MAX_SCREENS)
#define MAP_EMPTY 0xFFFF      /* a map cell with no tile ('..' on disk); tile 0 is a real tile */
#define NSLOTS 10
#define DOT_LIMIT_DEFAULT 100000
#define DOT_LIMIT_MAX 1000000
#define FONT_W 6               /* a character of the built-in font: 6 x 10 */
#define FONT_H 10
#define FONT_INK_W 5           /* the built-in font's letters use 5 of its 6 columns (for centring) */
#define FONT_FIRST 32          /* a font has glyphs for characters 32-127 */
#define NGLYPHS 96
#define FONT_MIN 4             /* a font's cell: 4 x 4 to 20 x 20 pixels */
#define FONT_MAX 20
#define TEXT_COLS (SCREEN_W / FONT_W)
#define TEXT_ROWS (SCREEN_H / FONT_H)
#define CLEAR 0xFF            /* transparent tile pixel */
#define FPS 60

/* the pad: a d-pad, three buttons A B C, and Start */
enum { BTN_LEFT, BTN_RIGHT, BTN_UP, BTN_DOWN, BTN_A, BTN_B, BTN_C, BTN_START, NBUTTONS };

/* ---- sound (v1): 8 voices playing 32-sample waveforms ---- */
#define NVOICES 8
#define NWAVES 16
#define WAVE_LEN 32
#define WAVE_NOISE 16          /* the wave number that means "noise" */
#define NSFX 64
#define SFX_STEPS 32
#define NPITCH 64              /* pitch 0 = C2 (MIDI 36) ... 63 = D#7 */
#define TICK_HZ 120            /* sfx steps last `speed` ticks */
enum { FX_NONE, FX_SLIDE, FX_VIBRATO, FX_DROP, FX_FADE_IN, FX_FADE_OUT, FX_ARPEGGIO, NFX };
extern const char fx_chars[NFX + 1];   /* ". s v d < > a": how the editors (and the old .txt files) show effects */

typedef struct { uint8_t pitch, wave, vol, fx; } SfxStep;   /* vol 0-7, 0 = rest */
typedef struct {
    uint8_t speed;             /* ticks per step, 1-64 */
    uint8_t length;            /* steps used, 1-32 */
    uint8_t loop_start, loop_end;   /* loop_end > loop_start: repeat those steps */
    SfxStep steps[SFX_STEPS];
} Sfx;

/* ---- music: a song is an order list of patterns; a pattern is 32 rows x 8 tracks,
 * track t plays on voice t (a sound effect on that voice takes priority) ---- */
#define NPATTERNS 32
#define PATTERN_ROWS 32
#define NTRACKS NVOICES
#define ORDER_LEN 64
enum { CELL_EMPTY, CELL_NOTE, CELL_OFF };          /* empty: the last note carries on */
typedef struct { uint8_t kind, pitch, wave, vol, fx; } Cell;
typedef struct { Cell cells[PATTERN_ROWS][NTRACKS]; } Pattern;
enum { ORDER_PATTERN, ORDER_JUMP, ORDER_STOP };
typedef struct { uint8_t kind, value; } OrderEntry; /* play pattern `value` / jump to entry `value` / stop */

/* ---- resources: the data a cart keeps in RAM ----
 * Each kind lives in its own folder of the cart and can be loaded into one
 * of NSLOTS slots while the cart runs:
 *   tiles/NAME.g64   a tileset      load_tileset(slot, "NAME"), swap_tileset(slot)
 *   maps/NAME.g64    a map          load_map / swap_map
 *   sfx/NAME.g64     waves + sfx    load_sfx / swap_sfx
 *   music/NAME.g64   a song         load_music / swap_music
 *   fonts/NAME.g64   a font         load_font / swap_font
 * Slot 0 starts out holding the file with the default name (tileset, map,
 * sfx, music, font) when the cart has one, and an empty one when it doesn't
 * (for a font: the built-in one). */
enum { RES_TILES, RES_MAP, RES_SFX, RES_MUSIC, RES_FONT, NRES };

typedef struct { uint8_t t[NTILES][TILE * TILE]; } Tileset;   /* palette index, or CLEAR */
typedef struct { int w, h; uint16_t *cells; } TileMap;        /* w, h in cells; MAP_EMPTY = no tile */
typedef struct {
    uint8_t waves[NWAVES][WAVE_LEN];      /* samples 0-15 (7.5 is silence) */
    Sfx sfx[NSFX];
} SoundBank;
typedef struct {
    Pattern patterns[NPATTERNS];
    OrderEntry order[ORDER_LEN];
    uint8_t song_speed;                   /* ticks per row */
} Song;
/* a font: every glyph fills a w x h cell, its spacing included (text steps w across, h down) */
typedef struct { uint8_t w, h; uint32_t g[NGLYPHS][FONT_MAX]; } Font;   /* a row of a glyph: bit x = pixel x */

/* a resource file open in the editors (possibly changed and not saved) */
typedef struct {
    int kind;
    char name[64];                        /* file name without folder or extension */
    bool dirty;
    char legacy[80];                      /* read from an old file (tiles.txt, sfx/NAME.txt ...): SAVE moves it */
    void *data;                           /* Tileset / TileMap / SoundBank / Song / Font */
} Doc;
#define MAX_DOCS 64

extern const char *const res_dir[NRES];      /* "tiles", "maps", "sfx", "music", "fonts" */
extern const char *const res_ext[NRES];      /* ".g64" for every kind */
extern const char *const res_default[NRES];  /* "tileset", "map", "sfx", "music", "font" */
extern const char *const res_word[NRES];     /* "tileset", "map", "sound", "music", "font": for messages */

/* the current resource of each kind: what drawing, sprite(), map_get(), sfx()
 * and music() use. While a cart runs: its swapped-in slot (NULL if that slot
 * is empty). Otherwise: the file each editor has open. */
extern Tileset *cur_tiles;
extern TileMap *cur_map;
extern SoundBank *cur_snd;
extern Song *cur_song;
extern Font *cur_font;                       /* what a cart's text() draws with (the shell and editors use sys_font) */

/* ---- the loaded cartridge ---- */
typedef struct {
    bool loaded;
    bool untitled;            /* the boot cart: in a scratch folder, saved only by SAVE name */
    char dir[4096];           /* the cart folder */
    char name[128];           /* folder name */
    char title[128];
    char author[128];
    char version[32];
    Doc *docs[MAX_DOCS];      /* resource files opened so far */
    int ndocs;
    Doc *active[NRES];        /* the file each editor works on */
} Cart;

extern Cart cart;

/* ---- video.c: framebuffer and drawing ---- */
extern uint8_t fb[SCREEN_H][SCREEN_W];
extern const uint32_t palette[NCOLORS];   /* 0xRRGGBB */

typedef struct {
    int cam_x, cam_y;
    int clip_x0, clip_y0, clip_x1, clip_y1;   /* clip_x1/y1 exclusive */
    uint8_t pen;
    uint8_t pal[NCOLORS];        /* drawing colour c puts pal[c] on screen (pal(a, b)) */
    bool count_dots;             /* a cart is drawing: pixels count against the dot limit */
    const Font *font;            /* what draw_char uses; NULL = sys_font */
    int16_t depth;               /* set_depth(): lower is nearer; a pixel is drawn if depth <= zbuf there */
    uint8_t blend;               /* set_blend(): BLEND_NORMAL ... */
} DrawState;

/* blend modes: how a drawn colour combines with the one already on screen,
 * per 2-bit RGB channel (ADD / SUB saturate at 3 / 0) */
enum { BLEND_NORMAL, BLEND_ADD, BLEND_SUB, BLEND_MAX, BLEND_MIN, BLEND_XOR, NBLENDS };
#define DEPTH_FAR INT16_MAX
extern int16_t zbuf[SCREEN_H][SCREEN_W];   /* the depth buffer: cls() puts it back to DEPTH_FAR */
extern DrawState ds;
extern int dot_limit, dots_used;  /* pixels a cart may draw per frame / drawn since the last flip() */
/* the part of the screen the UI draws in: the editors sit to the right of the
 * file tree. draw_reset() puts (0, 0) at its top-left and clips to it. */
extern int view_x, view_w;
#define UI_W view_w

void draw_reset(void);
void draw_cls(int c);
void draw_pset(int x, int y, int c);
int draw_pget(int x, int y);
void draw_line(int x0, int y0, int x1, int y1, int c);
void draw_rect(int x, int y, int w, int h, int c, bool fill);
void draw_circle(int cx, int cy, int r, int c, bool fill);
int draw_char(int ch, int x, int y, int c);
int draw_text(const char *s, int x, int y, int c);   /* returns end x; handles \n */
int text_width(const char *s);       /* with ds.font, like draw_text */
void draw_sprite(int n, int x, int y, int w, int h, int flags);
void draw_map(int mx, int my, int sx, int sy, int w, int h);
void draw_clip(int x, int y, int w, int h);
void draw_clip_reset(void);
void draw_set_depth(int z);          /* clamped to int16 */
void draw_spray(int x, int y, int w, int h, int n, int c, double (*unit_rng)(void));
/* a w x h tile block from the sheet at tile n, mapped onto the quad
 * p0 (top-left) p1 (top-right) p2 (bottom-right) p3 (bottom-left).
 * z == NULL: drawn at the current depth; otherwise z[i] is p[i]'s depth,
 * interpolated across the quad (orthographic 3D) */
void draw_tex_quad(const int px[4], const int py[4], const int *z, int n, int w, int h);

/* ---- font.c ---- */
extern Font sys_font;                    /* the built-in font: the shell's and editors', and a cart's default */
void font_init(void);

/* ---- platform.c: window, timing, input ---- */
typedef struct {
    const char *script;   /* scripted input, one token per frame: text, {enter}, {ctrl+s}, ... */
    bool headless;        /* no window: run frames as fast as possible */
    int max_frames;       /* > 0: quit after this many frames */
    const char *shot;     /* screenshot file written on quit (with max_frames) */
    const char *wav;      /* headless: write everything the synth played to this WAV file */
    int scale;
    bool player;          /* an exported game: boots straight into its cart, no shell or editors */
} Options;
extern Options opt;

/* key presses and typed text, in the order they happened (editors want order) */
enum { EV_KEY = 1, EV_TEXT = 2 };
enum { MOD_CTRL = 1, MOD_SHIFT = 2, MOD_ALT = 4 };
typedef struct { uint8_t type; uint8_t mod; uint16_t sc; char text[8]; } KeyEvent;
#define MAX_EVENTS 64

typedef struct {
    KeyEvent events[MAX_EVENTS];
    int nevents;
    int wheel;                    /* mouse wheel steps this frame (+ = up) */
    bool btn[NBUTTONS], btn_prev[NBUTTONS];
    uint8_t key[512];             /* held, by SDL scancode */
    uint8_t key_hit[512];         /* pressed since the last frame */
    int mouse_x, mouse_y;
    bool mouse[4], mouse_hit[4];
    char text[64];                /* typed text since the last frame (shell) */
    bool quit;                    /* window closed */
    bool escape;                  /* Esc pressed this frame */
} Input;
extern Input in;
extern long frame_count;

bool platform_init(void);
void platform_shutdown(void);
void platform_frame(void);        /* present, wait for the next frame, read input */
void input_consume(void);         /* forget this frame's key presses (after a cart returns) */
bool save_screenshot(const char *path);
void platform_title(const char *title);
void platform_set_fullscreen(bool on);
bool platform_fullscreen(void);
void platform_set_scale(int scale);   /* the window's size: scale x the screen (not in fullscreen) */
bool platform_take_dropped(char *path, size_t n);   /* a file dropped on the window (or uploaded) */
uint32_t ge_crc32(const void *p, size_t n);
char *clipboard_get(void);            /* malloc'd, never NULL */
void clipboard_set(const char *text);

/* ---- cart.c ---- */
bool cart_load(const char *nameOrPath, char *err, int errlen);
const char *carts_root(void);
void carts_root_set(const char *dir);     /* the player unpacks its cart into a folder of its own */
bool carts_cd(const char *arg, char *err, size_t errn);   /* CD: move between folders inside the carts folder */
const char *carts_subdir(void);           /* where CD is, below the carts folder: "" at the top */
int cart_check(const char *nameOrPath);   /* compile only; prints errors; returns exit code */
bool cart_valid_name(const char *s);      /* letters, digits, - and _ */

/* runs the loaded cart; returns 0 if it ended normally, 1 on error, 2 if stopped with Esc.
 * Error text is appended to the shell through shell_print. */
int cart_run(void);
extern bool in_cart;

/* where the last failed run stopped, e.g. "main.gb", 12, "Array index out of bounds" */
extern char cart_err_file[256];
extern int cart_err_line;
extern char cart_err_msg[256];
bool cart_create(const char *name, char *err, int errlen);   /* NEW: a folder with a starter main.gb */
bool cart_new_untitled(char *err, int errlen);             /* a starter cart in a scratch folder */
bool cart_save_as(const char *name, bool overwrite, char *err, int errlen); /* SAVE name: the untitled cart joins the carts folder */
bool cart_exists(const char *name);                         /* a cart folder with this name is in the carts folder */
void cart_discard_scratch(void);                            /* remove the untitled cart's scratch folder */
const char *cart_scratch_dir(void);                         /* "" when there is none */

/* ---- res.c: resource files, documents and the run-time slots ---- */
bool res_parse_legacy_sound(const char *path, SoundBank *b, char *err, int errlen);   /* cart.c: the old sfx .txt */
bool res_parse_legacy_music(const char *path, Song *m, char *err, int errlen);        /* cart.c: the old music .txt */
bool res_parse_legacy_tiles(const char *path, Tileset *t, char *err, int errlen);
bool res_parse_legacy_map(const char *path, TileMap *m, char *err, int errlen);
void *res_new(int kind);                       /* an empty resource */
void *res_copy(int kind, const void *p);       /* a deep copy */
void res_free(int kind, void *p);
bool res_name_ok(const char *name);
void res_path(int kind, const char *name, char *out, size_t n);   /* cart.dir/tiles/NAME.g64 ... */
bool map_resize(TileMap *m, int w, int h);     /* in cells; keeps what still fits */
static inline uint16_t map_cell(const TileMap *m, int x, int y) {
    return m && x >= 0 && y >= 0 && x < m->w && y < m->h ? m->cells[y * m->w + x] : MAP_EMPTY;
}

void docs_reset(void);                          /* forget every document (a new cart is loading) */
bool docs_load_defaults(char *err, int errlen); /* open the default-named files, converting old ones */
Doc *doc_find(int kind, const char *name);      /* already open */
Doc *doc_open(int kind, const char *name, char *err, int errlen);   /* open, reading the file if needed */
Doc *doc_create(int kind, const char *name, char *err, int errlen); /* a new, empty, unsaved file */
Doc *doc_active(int kind);                      /* the editor's file (made empty with the default name if none) */
void doc_set_active(Doc *d);
void doc_touch(int kind);                       /* the editor changed its file */
bool doc_save(Doc *d, char *err, int errlen);
bool doc_rename(int kind, const char *name, const char *to, char *err, int errlen);
bool doc_delete(int kind, const char *name, char *err, int errlen);
bool docs_dirty(int kind);                      /* kind -1: any */
bool docs_save_all(int kind, char *msg, int msglen);   /* kind -1: all; msg lists what was saved or the error */
void docs_emergency_save(void);
void docs_bind(void);                           /* point cur_* at the editors' files */
int docs_list(int kind, const char **names, int max);   /* open and on-disk names, sorted */

void rt_begin(void);                            /* a cart starts: slot 0 gets the default files */
void rt_end(void);                              /* the cart stopped: free every slot */
bool rt_load(int kind, int slot, const char *name, char *err, int errlen);   /* name NULL: free the slot */
bool rt_swap(int kind, int slot, char *err, int errlen);
int rt_current(int kind);

/* ---- synth.c: the sound chip (no SDL here; platform.c feeds it to the device) ---- */
void synth_defaults(SoundBank *b);          /* default waves and empty sfx */
void synth_default_wave(int w, uint8_t *out);
void synth_default_sfx(Sfx *s);
void pitch_name(int pitch, char out[4]);    /* 24 -> "C-4" */
void synth_mix(int16_t *out, int n);        /* render n mono samples at SYNTH_RATE */
#define SYNTH_RATE 44100
int synth_sfx(int n, int voice);            /* voice -1 = any free one; returns the voice */
void synth_note(int voice, int pitch, int wave, int vol, int ticks);   /* ticks 0 = until stopped */
void synth_stop(int voice);                 /* -1 = all */
int synth_voice_sfx(int voice);             /* the sfx a voice plays, -1 none, -2 a note */
int synth_voice_step(int voice);            /* the step a voice's sfx is on */
void synth_music(int pos, int loop_pattern);  /* play from order entry pos; loop_pattern >= 0: just that pattern, forever */
void synth_music_stop(void);
void synth_set_volume(float all, float music, float sfx);   /* 0-1 each (callers hold audio_lock) */
int synth_music_pos(void);                  /* order entry playing, -1 = stopped */
int synth_music_pattern(void);              /* pattern playing, -1 = stopped */
int synth_music_row(void);
void music_defaults(Song *m);
void synth_forget(const void *bank);        /* a bank is being freed: stop what plays from it */
void audio_lock(void);                      /* platform.c: hold off the audio thread */
void audio_unlock(void);
extern const char ge_pixel_chars[];

/* ---- bundle.c: one-file carts ---- */
char *cart_pack(size_t *len, char *err, int errlen);   /* the loaded cart as .gbcart text, malloc'd */
bool cart_export(const char *path, char *out_path, int outlen, char *err, int errlen);
bool cart_import(const char *file, const char *name, char *out_dir, int outlen, char *err, int errlen);
/* ---- export.c: standalone games (a player runtime with the cart packed in) ---- */
typedef void (*ExportLog)(const char *line, bool error);
/* targets: "linux", "windows", "web" or "all" (several may be given, space separated);
 * outdir NULL = exports/<cart>. Returns false if any target failed. */
bool game_export(const char *targets, const char *outdir, ExportLog log);
int player_main(int argc, char **argv);
extern char player_errors[2048];   /* what the cart printed as errors (the player shows it) */

/* ---- web.c: the browser build (no-ops elsewhere) ---- */
void web_offer_download(const char *path);   /* hand a file to the browser as a download */
bool web_take_dropped(char *path, size_t n); /* a file the user dropped or uploaded, if any */
void web_pick_file(void);                    /* open the browser's file picker (UPLOAD) */

/* ---- save.c: each cart's save data, kept outside the cart ---- */
const char *save_dir(void);
bool save_key_ok(const char *key);
char *save_load(const char *key);                          /* malloc'd, NULL if not saved */
const char *save_store(const char *key, const char *value);   /* "" removes; returns an error or NULL */

/* ---- shell.c ---- */
enum { MODE_SHELL, MODE_EDITOR, MODE_TILES, MODE_MAP, MODE_SOUND, MODE_MUSIC, MODE_FONT, MODE_SETTINGS, MODE_BROWSE };
extern int ui_mode;
void shell_init(void);
void shell_print(const char *s, int color);
void shell_exec(const char *line);
void shell_exec_discarding(const char *line);  /* shell_exec, answering Y to "discard them?" */
void shell_frame(void);
void ui_run_from_editor(void);   /* save, run, then come back to the editor (on the error line) */
extern bool shell_quit;
bool ui_global_key(const KeyEvent *ev);   /* F1 code ... F5 music, F7 font: true if it switched */
int ui_draw_tabs(void);                   /* the editors' top bar; returns the x after the tabs */
void ui_switch(int mode);
bool ui_unsaved(void);                    /* code or any resource changed and not saved */
bool ui_save_all(void);

/* ---- tree.c: the file tree left of the editors ---- */
extern bool tree_open;
#define TREE_W 72
void ui_editor_frame(void);               /* the tree and the editor of ui_mode */
void tiles_doc_changed(void);             /* the tile editor's file was switched */
void map_doc_changed(void);
void sound_doc_changed(void);
void music_doc_changed(void);
void font_doc_changed(void);

/* ---- tiles_editor.c, map_editor.c ---- */
void tiles_frame(void);
void map_frame(void);
void sound_frame(void);
void music_frame(void);
void font_frame(void);                    /* font_editor.c */
void browse_frame(void);
void browse_open(void);

/* ---- editor.c ---- */
bool editor_open(void);          /* the editor shows a file of the cart (main.gb at first) */
bool editor_open_file(const char *name);  /* edit another .gb file of the cart (beside main.gb) */
const char *editor_file(void);            /* the .gb file being edited, "" if none */
const char *code_name(const char *file);  /* "main.gb" -> "main": code is always .gb, so the editors don't show it */
bool editor_file_modified(const char *name);
bool editor_rename_file(const char *name, const char *to);   /* a buffer follows its file's new name */
void editor_close_file(const char *name);                    /* drop a buffer (its file was deleted) */
void editor_frame(void);
bool editor_save(void);
bool editor_modified(void);
bool editor_flush(void);                  /* write main.gb for a run, without counting it as saved */
void editor_follow_cart(void);            /* the cart moved folders (SAVE name) */
void editor_forget(void);
void editor_show_error(int line, const char *msg);
void editor_emergency_save(void);

/* ---- named colours (indices into the 6-bit palette: r*16 + g*4 + b) ---- */
#define RGB6(r, g, b) ((r) * 16 + (g) * 4 + (b))
enum {
    COL_BLACK = RGB6(0, 0, 0), COL_WHITE = RGB6(3, 3, 3), COL_GREY = RGB6(2, 2, 2),
    COL_DARK_GREY = RGB6(1, 1, 1), COL_RED = RGB6(3, 0, 0), COL_GREEN = RGB6(0, 3, 0),
    COL_BLUE = RGB6(0, 0, 3), COL_YELLOW = RGB6(3, 3, 0), COL_CYAN = RGB6(0, 3, 3),
    COL_MAGENTA = RGB6(3, 0, 3), COL_ORANGE = RGB6(3, 2, 0), COL_BROWN = RGB6(2, 1, 0),
    COL_PINK = RGB6(3, 2, 2), COL_PURPLE = RGB6(2, 0, 3), COL_NAVY = RGB6(0, 0, 1),
    COL_DARK_GREEN = RGB6(0, 1, 0), COL_SKY = RGB6(1, 2, 3), COL_LIME = RGB6(2, 3, 0),
};

/* ---- settings.c: the console's settings (settings.txt) and the editors' colour theme ----
 * The shell and editors are drawn in one family of blues: RGB6(0, 0, 1) for
 * backgrounds, (0, 1, 2) for bars, (1, 2, 3) for highlights, (2, 3, 3) for text.
 * UI_RGB is RGB6 for that chrome: a theme turns each of those blues into its
 * own shade of another colour (swapping channels keeps every shade distinct).
 * Colours outside the family (yellow, red, the code editor's syntax colours)
 * and everything a cart draws are left alone. */
enum { THEME_BLUE, THEME_GREEN, THEME_ORANGE, THEME_GREY, NTHEMES };
extern int ui_theme;
extern const char *const theme_names[NTHEMES];
static inline int ui_rgb(int r, int g, int b) {
    if (!(b >= g && g >= r && b > r)) return RGB6(r, g, b);
    switch (ui_theme) {
        case THEME_GREEN: return RGB6(r, b, g);
        case THEME_ORANGE: return RGB6(b, g, r);
        case THEME_GREY: return RGB6(g, g, g);
        default: return RGB6(r, g, b);
    }
}
#define UI_RGB(r, g, b) ui_rgb(r, g, b)
static inline int ui_col(int c) { return ui_rgb(c >> 4, (c >> 2) & 3, c & 3); }   /* a palette index, themed */
int theme_find(const char *name);         /* -1 if not a theme ("white" and "gray" mean grey) */
bool theme_set(int theme, char *err, int errlen);   /* use it from now on, and keep it in settings.txt */
#define VOLUME_MAX 10
typedef struct {
    int volume, music_volume, sfx_volume;   /* 0-10: all sound, then music and sound effects on top */
    bool fullscreen;
    int scale;                              /* the window: 1-8 screen pixels per console pixel */
    int indent;                             /* the code editor's Enter: INDENT_SMART, _AUTO or _NONE */
} Settings;
enum { INDENT_SMART, INDENT_AUTO, INDENT_NONE, NINDENTS };
extern const char *const indent_names[NINDENTS];
extern Settings settings;
void settings_load(void);                 /* read settings.txt (before the window opens), and use its sound levels */
bool settings_save(char *err, int errlen);   /* write settings.txt, and use the sound levels from now on */
void settings_frame(void);                /* settings_screen.c: the settings screen (F9) */
void settings_open(void);                 /* from wherever you are; settings_frame_close() goes back there */
void settings_frame_close(void);

#endif
