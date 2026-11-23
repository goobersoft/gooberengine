/* The only file that talks to SDL: window, frame timing, input, screenshots. */
#include "engine.h"
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

Options opt = {.scale = 3};
Input in;
long frame_count;

static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Texture *tex;
static SDL_Texture *fx_tex;      /* the frame with post-processing, at fx_s x the screen */
static int fx_s, pixel_scale = 1;
static SDL_GameController *pad;
static Uint64 next_frame;
static Uint64 frame_start;          /* when the cart got control back for this frame */
static SDL_AudioDeviceID audio_dev;

/* ---- audio: the synth renders straight into SDL's buffer ---- */
static void audio_callback(void *ud, Uint8 *stream, int len) {
    (void)ud;
    synth_mix((int16_t *)stream, len / 2);
}

void audio_lock(void) { if (audio_dev) SDL_LockAudioDevice(audio_dev); }
void audio_unlock(void) { if (audio_dev) SDL_UnlockAudioDevice(audio_dev); }

/* headless: one frame's worth of samples per frame, kept for --wav */
static int16_t *wav_buf;
static size_t wav_len, wav_cap;

static void headless_audio(void) {
    int16_t tmp[SYNTH_RATE / FPS];
    synth_mix(tmp, SYNTH_RATE / FPS);
    if (!opt.wav) return;
    if (wav_len + SYNTH_RATE / FPS > wav_cap) {
        wav_cap = wav_cap ? wav_cap * 2 : SYNTH_RATE * 4;
        wav_buf = realloc(wav_buf, wav_cap * sizeof *wav_buf);
    }
    memcpy(wav_buf + wav_len, tmp, sizeof tmp);
    wav_len += SYNTH_RATE / FPS;
}

static void put32(FILE *f, uint32_t v) { for (int i = 0; i < 4; i++) fputc((int)(v >> (8 * i)) & 255, f); }
static void put16(FILE *f, uint16_t v) { fputc(v & 255, f); fputc(v >> 8, f); }

static void write_wav(void) {
    if (!opt.wav) return;
    FILE *f = fopen(opt.wav, "wb");
    if (!f) { fprintf(stderr, "gooberengine: could not write %s\n", opt.wav); return; }
    uint32_t bytes = (uint32_t)(wav_len * 2);
    fwrite("RIFF", 1, 4, f); put32(f, 36 + bytes); fwrite("WAVEfmt ", 1, 8, f);
    put32(f, 16); put16(f, 1); put16(f, 1); put32(f, SYNTH_RATE); put32(f, SYNTH_RATE * 2); put16(f, 2); put16(f, 16);
    fwrite("data", 1, 4, f); put32(f, bytes);
    for (size_t i = 0; i < wav_len; i++) put16(f, (uint16_t)wav_buf[i]);
    fclose(f);
}
static bool fullscreen;

void platform_set_fullscreen(bool on) {
    fullscreen = on;
    if (win) SDL_SetWindowFullscreen(win, on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
bool platform_fullscreen(void) { return fullscreen; }

void platform_set_scale(int scale) {
    opt.scale = scale;
    if (win && !fullscreen) {
        SDL_SetWindowSize(win, SCREEN_W * scale, SCREEN_H * scale);
        SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

bool platform_init(void) {
    font_init();
    draw_reset();
    frame_start = SDL_GetPerformanceCounter();
    if (opt.headless) return true;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0 &&
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "gooberengine: could not start SDL: %s\n", SDL_GetError());
        return false;
    }
    win = SDL_CreateWindow("GooberEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           SCREEN_W * opt.scale, SCREEN_H * opt.scale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!win) { fprintf(stderr, "gooberengine: could not open a window: %s\n", SDL_GetError()); return false; }
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!ren) { fprintf(stderr, "gooberengine: no renderer: %s\n", SDL_GetError()); return false; }
    SDL_RenderSetLogicalSize(ren, SCREEN_W, SCREEN_H);
    SDL_RenderSetIntegerScale(ren, SDL_TRUE);
    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (!tex) { fprintf(stderr, "gooberengine: no texture: %s\n", SDL_GetError()); return false; }
    for (int i = 0; i < SDL_NumJoysticks() && !pad; i++)
        if (SDL_IsGameController(i)) pad = SDL_GameControllerOpen(i);
    SDL_StartTextInput();
    SDL_AudioSpec want = {0}, have;
    want.freq = SYNTH_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_dev) SDL_PauseAudioDevice(audio_dev, 0);
    else fprintf(stderr, "gooberengine: no sound: %s\n", SDL_GetError());
    next_frame = SDL_GetPerformanceCounter();
    return true;
}

void platform_shutdown(void) {
    if (opt.headless) { write_wav(); return; }
    if (audio_dev) SDL_CloseAudioDevice(audio_dev);
    if (pad) SDL_GameControllerClose(pad);
    if (tex) SDL_DestroyTexture(tex);
    if (fx_tex) SDL_DestroyTexture(fx_tex);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
}

/* ---- screenshots: an indexed PNG (the 64 colors are its palette), written
 * without a library: zlib "stored" blocks need no compressor ---- */
static uint32_t crc_table[256];

static uint32_t crc32_update(uint32_t c, const uint8_t *p, size_t n) {
    if (!crc_table[1])
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t v = i;
            for (int k = 0; k < 8; k++) v = v & 1 ? 0xEDB88320u ^ (v >> 1) : v >> 1;
            crc_table[i] = v;
        }
    c = ~c;
    while (n--) c = crc_table[(c ^ *p++) & 255] ^ (c >> 8);
    return ~c;
}

uint32_t ge_crc32(const void *p, size_t n) { return crc32_update(0, p, n); }

static void be32(uint8_t *p, uint32_t v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = (uint8_t)v; }

static bool png_chunk(FILE *f, const char *type, const uint8_t *data, uint32_t n) {
    uint8_t hdr[8];
    be32(hdr, n);
    memcpy(hdr + 4, type, 4);
    uint32_t crc = crc32_update(crc32_update(0, hdr + 4, 4), data, n);
    uint8_t tail[4];
    be32(tail, crc);
    return fwrite(hdr, 1, 8, f) == 8 && fwrite(data, 1, n, f) == n && fwrite(tail, 1, 4, f) == 4;
}

bool save_screenshot(const char *path) {
    enum { ROW = SCREEN_W + 1, RAW = ROW * SCREEN_H, BLOCK = 65535 };
    static uint8_t raw[RAW], z[2 + RAW + (RAW / BLOCK + 1) * 5 + 4];
    for (int y = 0; y < SCREEN_H; y++) {
        raw[y * ROW] = 0;                                  /* filter: none */
        memcpy(raw + y * ROW + 1, fb[y], SCREEN_W);
    }
    size_t zn = 0;
    z[zn++] = 0x78; z[zn++] = 0x01;
    for (size_t at = 0; at < RAW; at += BLOCK) {
        size_t len = RAW - at < BLOCK ? RAW - at : BLOCK;
        z[zn++] = at + len == RAW;                         /* last block? */
        z[zn++] = len & 255; z[zn++] = (uint8_t)(len >> 8);
        z[zn++] = ~len & 255; z[zn++] = (uint8_t)(~len >> 8);
        memcpy(z + zn, raw + at, len);
        zn += len;
    }
    uint32_t a = 1, b = 0;                                 /* adler-32 */
    for (size_t i = 0; i < RAW; i++) { a = (a + raw[i]) % 65521; b = (b + a) % 65521; }
    be32(z + zn, b << 16 | a);
    zn += 4;
    uint8_t ihdr[13] = {0}, plte[NCOLORS * 3];
    be32(ihdr, SCREEN_W);
    be32(ihdr + 4, SCREEN_H);
    ihdr[8] = 8;                                           /* 8 bits per index */
    ihdr[9] = 3;                                           /* palette color */
    for (int i = 0; i < NCOLORS; i++) {
        plte[i * 3] = (uint8_t)(palette[i] >> 16);
        plte[i * 3 + 1] = (uint8_t)(palette[i] >> 8);
        plte[i * 3 + 2] = (uint8_t)palette[i];
    }
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    bool ok = fwrite("\x89PNG\r\n\x1a\n", 1, 8, f) == 8 && png_chunk(f, "IHDR", ihdr, 13) &&
              png_chunk(f, "PLTE", plte, sizeof plte) && png_chunk(f, "IDAT", z, (uint32_t)zn) &&
              png_chunk(f, "IEND", NULL, 0);
    return fclose(f) == 0 && ok;
}

void platform_title(const char *title) { if (win) SDL_SetWindowTitle(win, title); }


int platform_pixel_scale(void) { return opt.headless ? opt.scale : pixel_scale; }

static void present(void) {
    int ow = 0, oh = 0;
    if (SDL_GetRendererOutputSize(ren, &ow, &oh) == 0) {
        pixel_scale = ow / SCREEN_W < oh / SCREEN_H ? ow / SCREEN_W : oh / SCREEN_H;
        if (pixel_scale < 1) pixel_scale = 1;
    }
    SDL_Texture *shown = tex;
    void *pixels;
    int pitch;
    int s = postfx_scale(pixel_scale);
    if (s && s != fx_s) {
        if (fx_tex) SDL_DestroyTexture(fx_tex);
        fx_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W * s, SCREEN_H * s);
        fx_s = fx_tex ? s : 0;
    }
    if (s && fx_tex && SDL_LockTexture(fx_tex, NULL, &pixels, &pitch) == 0) {
        postfx_render(pixels, pitch, s);
        SDL_UnlockTexture(fx_tex);
        shown = fx_tex;
    } else if (SDL_LockTexture(tex, NULL, &pixels, &pitch) == 0) {
        for (int y = 0; y < SCREEN_H; y++) {
            Uint32 *row = (Uint32 *)((Uint8 *)pixels + y * pitch);
            for (int x = 0; x < SCREEN_W; x++) row[x] = 0xFF000000u | palette[fb[y][x] & 63];
        }
        SDL_UnlockTexture(tex);
    }
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, shown, NULL, NULL);
    SDL_RenderPresent(ren);
}

static void wait_for_frame(void) {
#ifdef __EMSCRIPTEN__
    /* the browser must get control back every frame (ASYNCIFY unwinds to it here);
     * it paces us with the display, so only sleep off any time left over */
    {
        static double next;
        double now = emscripten_get_now();
        next = now > next + 4000.0 / FPS ? now : next + 1000.0 / FPS;
        emscripten_sleep(next > now ? (unsigned)(next - now) : 0);
        return;
    }
#endif
    Uint64 freq = SDL_GetPerformanceFrequency(), step = freq / FPS;
    next_frame += step;
    Uint64 now = SDL_GetPerformanceCounter();
    if (now > next_frame + step * 4) { next_frame = now; return; }   /* fell behind: don't race to catch up */
    while ((now = SDL_GetPerformanceCounter()) < next_frame) {
        Uint64 left_ms = (next_frame - now) * 1000 / freq;
        if (left_ms > 1) SDL_Delay((Uint32)(left_ms - 1));
    }
}

static int pad_button(int b) {
    if (!pad) return 0;
    /* SDL names buttons by position on an Xbox-style pad: A bottom, B right, X left, Y top.
     * The three face buttons left to right along the lower arc are X, A, B, so they
     * become the console's A, B, C in that order. Y is not used. */
    static const int map[NBUTTONS][2] = {
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, -1}, {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, -1},
        {SDL_CONTROLLER_BUTTON_DPAD_UP, -1}, {SDL_CONTROLLER_BUTTON_DPAD_DOWN, -1},
        {SDL_CONTROLLER_BUTTON_X, -1}, {SDL_CONTROLLER_BUTTON_A, -1},
        {SDL_CONTROLLER_BUTTON_B, -1}, {SDL_CONTROLLER_BUTTON_START, -1},
    };
    for (int k = 0; k < 2; k++)
        if (map[b][k] >= 0 && SDL_GameControllerGetButton(pad, (SDL_GameControllerButton)map[b][k])) return 1;
    int ax = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
    int ay = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
    const int dead = 12000;
    return (b == BTN_LEFT && ax < -dead) || (b == BTN_RIGHT && ax > dead) ||
           (b == BTN_UP && ay < -dead) || (b == BTN_DOWN && ay > dead);
}

static void read_buttons(void) {
    static const int keys[NBUTTONS][2] = {
        {SDL_SCANCODE_LEFT, SDL_SCANCODE_LEFT}, {SDL_SCANCODE_RIGHT, SDL_SCANCODE_RIGHT},
        {SDL_SCANCODE_UP, SDL_SCANCODE_UP}, {SDL_SCANCODE_DOWN, SDL_SCANCODE_DOWN},
        {SDL_SCANCODE_Z, SDL_SCANCODE_SPACE}, {SDL_SCANCODE_X, SDL_SCANCODE_X},
        {SDL_SCANCODE_C, SDL_SCANCODE_C}, {SDL_SCANCODE_RETURN, SDL_SCANCODE_RETURN},
    };
    for (int b = 0; b < NBUTTONS; b++) {
        in.btn_prev[b] = in.btn[b];
        /* a key pressed this frame counts even if it was already let go: a quick tap still registers */
        in.btn[b] = in.key[keys[b][0]] || in.key[keys[b][1]] || in.key_hit[keys[b][0]] || in.key_hit[keys[b][1]] ||
                    pad_button(b);
    }
}

static void add_event(int type, int sc, int mod, const char *text) {
    if (in.nevents == MAX_EVENTS) return;
    KeyEvent *ev = &in.events[in.nevents++];
    ev->type = (uint8_t)type;
    ev->sc = (uint16_t)sc;
    ev->mod = (uint8_t)mod;
    snprintf(ev->text, sizeof ev->text, "%s", text ? text : "");
}

/* a key press the UI asked for (clicking "^S save" in the tab bar): it arrives
 * next frame like a real one, so whichever editor is open handles it its own way */
static int queued_sc, queued_mod;

void platform_queue_key(int sc, int mod) { queued_sc = sc; queued_mod = mod; }

static void clear_frame_input(void) {
    memset(in.key_hit, 0, sizeof in.key_hit);
    memset(in.mouse_hit, 0, sizeof in.mouse_hit);
    in.text[0] = 0;
    in.escape = false;
    in.nevents = 0;
    in.wheel = 0;
    if (queued_sc > 0 && queued_sc < 512) {
        in.key_hit[queued_sc] = 1;
        add_event(EV_KEY, queued_sc, queued_mod, NULL);
    }
    queued_sc = 0;
}

void input_consume(void) { clear_frame_input(); }

static void typed(const char *t) {
    size_t n = strlen(in.text);
    if (n + strlen(t) < sizeof in.text) strcat(in.text, t);
    add_event(EV_TEXT, 0, 0, t);
}

static void pressed(int sc, int mod) {
    if (sc > 0 && sc < 512) in.key_hit[sc] = 1;
    if (sc == SDL_SCANCODE_ESCAPE && !opt.player) in.escape = true;   /* a game can't be "stopped" */
    add_event(EV_KEY, sc, mod, NULL);
}

/* Scripted input (--type): one token per frame. A token is one character of
 * text, or {name} / {ctrl+shift+name} for a key, using SDL key names plus
 * the short forms enter, esc, bs, del, pgup, pgdn. */
static const char *script_pos;
static int script_release;     /* button a {click} holds until the next token that isn't {drag} */
static int script_key_held;    /* a scripted key is held down for one frame (so btn() sees it) */

static void script_press(int sc, int mod) {
    pressed(sc, mod);
    if (sc > 0 && sc < 512) { in.key[sc] = 1; script_key_held = sc; }
}
static void script_step(void) {
    if (!opt.script) return;
    if (!script_pos) script_pos = opt.script;
    if (script_key_held) { in.key[script_key_held] = 0; script_key_held = 0; }
    if (script_release && strncmp(script_pos, "{drag:", 6)) {
        in.mouse[script_release] = false;
        script_release = 0;
    }
    if (!*script_pos) return;
    /* mouse tokens: {click:x,y} {rclick:x,y} {mclick:x,y} press there; {drag:x,y} moves with the
     * button still held; {mouse:x,y} just moves; {wheel:n} scrolls. {down:key} holds a key
     * (as SDL names it: {down:left alt}) until {up:key}, for a modifier held while the mouse works */
    char kn[32];
    bool down = sscanf(script_pos, "{down:%31[^}]}", kn) == 1;
    if (down || sscanf(script_pos, "{up:%31[^}]}", kn) == 1) {
        script_pos = strchr(script_pos, '}') + 1;
        SDL_Scancode k = SDL_GetScancodeFromName(kn);
        if (k == SDL_SCANCODE_UNKNOWN) fprintf(stderr, "gooberengine: unknown key {%s}\n", kn);
        else in.key[k] = down;
        return;
    }
    int mx, my, b = 0;
    if (sscanf(script_pos, "{click:%d,%d}", &mx, &my) == 2) b = 1;
    else if (sscanf(script_pos, "{rclick:%d,%d}", &mx, &my) == 2) b = 2;
    else if (sscanf(script_pos, "{mclick:%d,%d}", &mx, &my) == 2) b = 3;
    else if (sscanf(script_pos, "{drag:%d,%d}", &mx, &my) == 2 || sscanf(script_pos, "{mouse:%d,%d}", &mx, &my) == 2) b = -1;
    else if (sscanf(script_pos, "{wheel:%d}", &mx) == 1) { in.wheel = mx; b = -2; }
    if (b) {
        script_pos = strchr(script_pos, '}') + 1;
        if (b == -2) return;
        in.mouse_x = mx;
        in.mouse_y = my;
        if (b > 0) { in.mouse[b] = true; in.mouse_hit[b] = true; script_release = b; }
        return;
    }
    if (*script_pos != '{' || script_pos[1] == '{') {
        char t[2] = {*script_pos, 0};
        script_pos += *script_pos == '{' ? 2 : 1;      /* {{ types a literal { */
        /* like a real keyboard: the key goes down, then the text arrives */
        int c = (unsigned char)t[0], sc = 0, mod = 0;
        if (c >= 'a' && c <= 'z') sc = SDL_SCANCODE_A + (c - 'a');
        else if (c >= 'A' && c <= 'Z') { sc = SDL_SCANCODE_A + (c - 'A'); mod = MOD_SHIFT; }
        else if (c >= '1' && c <= '9') sc = SDL_SCANCODE_1 + (c - '1');
        else if (c == '0') sc = SDL_SCANCODE_0;
        else {
            static const struct { char c; int sc; } punct[] = {
                {' ', SDL_SCANCODE_SPACE}, {',', SDL_SCANCODE_COMMA}, {'.', SDL_SCANCODE_PERIOD},
                {'-', SDL_SCANCODE_MINUS}, {'=', SDL_SCANCODE_EQUALS}, {'/', SDL_SCANCODE_SLASH},
                {';', SDL_SCANCODE_SEMICOLON}, {'\'', SDL_SCANCODE_APOSTROPHE},
                {'[', SDL_SCANCODE_LEFTBRACKET}, {']', SDL_SCANCODE_RIGHTBRACKET}};
            for (size_t i = 0; i < sizeof punct / sizeof punct[0]; i++) if (punct[i].c == c) sc = punct[i].sc;
        }
        if (sc) script_press(sc, mod);
        typed(t);
        return;
    }
    const char *end = strchr(script_pos, '}');
    if (!end) { script_pos += strlen(script_pos); return; }
    char tok[64];
    snprintf(tok, sizeof tok, "%.*s", (int)(end - script_pos - 1), script_pos + 1);
    script_pos = end + 1;
    int mod = 0;
    char *name = tok, *plus;
    while ((plus = strchr(name, '+')) && plus[1]) {
        *plus = 0;
        if (!strcasecmp(name, "ctrl")) mod |= MOD_CTRL;
        else if (!strcasecmp(name, "shift")) mod |= MOD_SHIFT;
        else if (!strcasecmp(name, "alt")) mod |= MOD_ALT;
        name = plus + 1;
    }
    static const char *alias[][2] = {{"enter", "Return"}, {"esc", "Escape"}, {"bs", "Backspace"},
                                     {"del", "Delete"}, {"pgup", "PageUp"}, {"pgdn", "PageDown"}};
    for (size_t i = 0; i < sizeof alias / sizeof alias[0]; i++)
        if (!strcasecmp(name, alias[i][0])) name = (char *)alias[i][1];
    SDL_Scancode sc = SDL_GetScancodeFromName(name);
    if (sc == SDL_SCANCODE_UNKNOWN) { fprintf(stderr, "gooberengine: unknown key {%s}\n", tok); return; }
    script_press(sc, mod);
}

static char dropped[1024];

bool platform_take_dropped(char *path, size_t n) {
    if (web_take_dropped(path, n)) return true;
    if (!dropped[0]) return false;
    snprintf(path, n, "%s", dropped);
    dropped[0] = 0;
    return true;
}

static void pump(void) {
    clear_frame_input();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: in.quit = true; break;
            case SDL_KEYDOWN: {
                int sc = e.key.keysym.scancode;
                SDL_Keymod m = e.key.keysym.mod;
                int mod = (m & KMOD_CTRL ? MOD_CTRL : 0) | (m & KMOD_SHIFT ? MOD_SHIFT : 0) | (m & KMOD_ALT ? MOD_ALT : 0);
                if (sc >= 0 && sc < 512 && !e.key.repeat) in.key[sc] = 1;
                if (sc != SDL_SCANCODE_ESCAPE || !e.key.repeat) pressed(sc, mod);   /* repeats count: editors want them */
                bool alt = e.key.keysym.mod & KMOD_ALT;
                if (sc == SDL_SCANCODE_F11 || (alt && sc == SDL_SCANCODE_RETURN)) {
                    platform_set_fullscreen(!fullscreen);
                    if (!opt.player) {          /* the engine remembers it; an exported game doesn't keep settings */
                        char err[256];
                        settings.fullscreen = fullscreen;
                        settings_save(err, sizeof err);
                    }
                }
                if (sc == SDL_SCANCODE_F12 && !e.key.repeat) {
                    char path[64];
#ifdef __EMSCRIPTEN__
                    snprintf(path, sizeof path, "/tmp/screenshot-%ld.png", frame_count);
                    if (save_screenshot(path)) web_offer_download(path);
#else
                    snprintf(path, sizeof path, "screenshot-%ld.png", frame_count);
                    if (save_screenshot(path)) fprintf(stderr, "saved %s\n", path);
#endif
                }
                break;
            }
            case SDL_KEYUP: {
                int sc = e.key.keysym.scancode;
                if (sc >= 0 && sc < 512) in.key[sc] = 0;
                break;
            }
            case SDL_TEXTINPUT:
                if (!(SDL_GetModState() & KMOD_CTRL)) typed(e.text.text);
                break;
            case SDL_MOUSEWHEEL: in.wheel += e.wheel.y; break;
            case SDL_MOUSEMOTION:
                in.mouse_x = e.motion.x; in.mouse_y = e.motion.y;
                postfx_map_mouse(&in.mouse_x, &in.mouse_y);     /* the pixel you see under it on a bent screen */
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int b = e.button.button == SDL_BUTTON_LEFT ? 1 : e.button.button == SDL_BUTTON_RIGHT ? 2
                      : e.button.button == SDL_BUTTON_MIDDLE ? 3 : 0;
                if (!b) break;
                in.mouse[b] = e.type == SDL_MOUSEBUTTONDOWN;
                if (in.mouse[b]) in.mouse_hit[b] = true;
                in.mouse_x = e.button.x; in.mouse_y = e.button.y;
                postfx_map_mouse(&in.mouse_x, &in.mouse_y);
                break;
            }
            case SDL_DROPFILE:          /* a zipped cart or a cart folder dropped on the window */
                if (e.drop.file) {
                    snprintf(dropped, sizeof dropped, "%s", e.drop.file);
                    SDL_free(e.drop.file);
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (!pad) pad = SDL_GameControllerOpen(e.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (pad && e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad))) {
                    SDL_GameControllerClose(pad);
                    pad = NULL;
                }
                break;
        }
    }
    read_buttons();
}

/* how far the pointer moved since the last frame; the first frame reports 0 */
static void mouse_motion(void) {
    static int last_x, last_y;
    static bool seen;
    in.mouse_dx = seen ? in.mouse_x - last_x : 0;
    in.mouse_dy = seen ? in.mouse_y - last_y : 0;
    last_x = in.mouse_x; last_y = in.mouse_y;
    seen = true;
}

/* --frames ran out: the screenshot, then quit */
static void frame_limit(void) {
    if (opt.shot && !save_screenshot(opt.shot)) fprintf(stderr, "gooberengine: could not write %s\n", opt.shot);
    in.quit = true;
}

/* A cart stuck in a loop that never reaches flip(): look for Esc or a window
 * close without running a frame. Events are only peeked, so a cart that is
 * merely busy loses no input; frame state (key_hit, btnp) is left alone. */
bool platform_poll_stop(void) {
    if (opt.headless) {
        /* scripted input: an {esc} up next stops a stuck cart as well */
        const char *p = script_pos ? script_pos : opt.script;
        if (p && !opt.player && !strncasecmp(p, "{esc}", 5)) { script_pos = p + 5; return true; }
        /* --frames: time spent stuck counts as the frames a window would have shown in it,
         * so a cart that never calls flip() still ends the run */
        if (opt.max_frames > 0) {
            Uint64 stuck = (SDL_GetPerformanceCounter() - frame_start) * FPS / SDL_GetPerformanceFrequency();
            if (frame_count + (long)stuck >= opt.max_frames) { frame_limit(); return true; }
        }
        return false;
    }
#ifdef __EMSCRIPTEN__
    emscripten_sleep(0);            /* the browser only delivers input while it has control */
#endif
    SDL_PumpEvents();
    if (SDL_HasEvent(SDL_QUIT)) { in.quit = true; return true; }   /* closing the window always works */
    if (opt.player) return false;   /* a game can't be "stopped" */
    SDL_Event ev[64];
    int n = SDL_PeepEvents(ev, 64, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN);
    for (int i = 0; i < n; i++)
        if (ev[i].key.keysym.scancode == SDL_SCANCODE_ESCAPE && !ev[i].key.repeat) {
            SDL_FlushEvents(SDL_KEYDOWN, SDL_TEXTINPUT);   /* this Esc is used up: the shell mustn't see it too */
            return true;
        }
    return false;
}

void platform_frame(void) {
    frame_count++;
    if (opt.max_frames > 0 && frame_count >= opt.max_frames) frame_limit();
    if (opt.headless) {
        headless_audio();
        clear_frame_input();
        script_step();
        read_buttons();
        mouse_motion();
        frame_start = SDL_GetPerformanceCounter();
        return;
    }
    present();
    wait_for_frame();
    pump();
    script_step();
    mouse_motion();
    frame_start = SDL_GetPerformanceCounter();
}

/* how much of one frame's time (1/FPS s) has gone since the cart got control back,
 * as a percentage: over 100 means this frame will be late */
int platform_frame_load(void) {
    Uint64 used = SDL_GetPerformanceCounter() - frame_start;
    Uint64 load = used * 100 * FPS / SDL_GetPerformanceFrequency();
    return load > 9999 ? 9999 : (int)load;
}

static char clip_mem[1 << 16];      /* headless stand-in for the system clipboard */

char *clipboard_get(void) {
    if (!opt.headless && SDL_HasClipboardText()) {
        char *s = SDL_GetClipboardText();
        char *r = strdup(s ? s : "");
        SDL_free(s);
        return r;
    }
    return strdup(opt.headless ? clip_mem : "");
}

void clipboard_set(const char *text) {
    if (opt.headless) snprintf(clip_mem, sizeof clip_mem, "%s", text);
    else SDL_SetClipboardText(text);
}
