/* A picture on the system clipboard, as PNG bytes: what Ctrl+V in the tile
 * editor pastes when another program (Aseprite, GIMP, a browser) copied one.
 *
 * SDL2's clipboard is text only, so this asks for the clipboard's "image/png"
 * itself: on X11 straight from the X server (libX11 is loaded when it's first
 * needed, the way SDL loads it, so building needs only its headers), else, on
 * Wayland, from wl-paste if it's installed. Its own file because Xlib's names
 * (Window, Display, Status, ...) would clash with the engine's. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

uint8_t *clip_image_png(SDL_Window *own, size_t *n);
#define MAX_PNG (64u << 20)

#if defined(__linux__) && !defined(GE_PLAYER) && !defined(__EMSCRIPTEN__) && __has_include(<X11/Xlib.h>)
#include <SDL2/SDL_syswm.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <dlfcn.h>
#include <limits.h>
#define HAVE_X11 1

static struct {
    bool tried;
    Display *d;
    Window w;
    Atom clipboard, png, incr, prop;
    Display *(*OpenDisplay)(const char *);
    Atom (*InternAtom)(Display *, const char *, Bool);
    Window (*CreateSimpleWindow)(Display *, Window, int, int, unsigned, unsigned, unsigned, unsigned long, unsigned long);
    Window (*GetSelectionOwner)(Display *, Atom);
    int (*ConvertSelection)(Display *, Atom, Atom, Atom, Window, Time);
    int (*GetWindowProperty)(Display *, Window, Atom, long, long, Bool, Atom, Atom *, int *, unsigned long *, unsigned long *, unsigned char **);
    int (*DeleteProperty)(Display *, Window, Atom);
    int (*SelectInput)(Display *, Window, long);
    Bool (*CheckTypedWindowEvent)(Display *, Window, int, XEvent *);
    int (*Flush)(Display *);
    int (*Free)(void *);
} X;

/* the X connection and an unseen window of our own that the picture is sent to */
static bool x11_open(void) {
    if (X.tried) return X.d != NULL;
    X.tried = true;
    void *lib = dlopen("libX11.so.6", RTLD_NOW | RTLD_LOCAL);
    if (!lib) return false;
#define LOAD(f) if (!(*(void **)&X.f = dlsym(lib, "X" #f))) return false
    LOAD(OpenDisplay); LOAD(InternAtom); LOAD(CreateSimpleWindow); LOAD(GetSelectionOwner);
    LOAD(ConvertSelection); LOAD(GetWindowProperty); LOAD(DeleteProperty); LOAD(SelectInput);
    LOAD(CheckTypedWindowEvent); LOAD(Flush); LOAD(Free);
#undef LOAD
    Display *d = X.OpenDisplay(NULL);
    if (!d) return false;
    X.w = X.CreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, 1, 1, 0, 0, 0);
    X.SelectInput(d, X.w, PropertyChangeMask);
    X.clipboard = X.InternAtom(d, "CLIPBOARD", False);
    X.png = X.InternAtom(d, "image/png", False);
    X.incr = X.InternAtom(d, "INCR", False);
    X.prop = X.InternAtom(d, "GOOBERENGINE_CLIP", False);
    X.d = d;
    return true;
}

/* waits up to a second for an event of this type on our window */
static bool x11_wait(int type, XEvent *ev) {
    for (int i = 0; i < 200; i++) {
        if (X.CheckTypedWindowEvent(X.d, X.w, type, ev)) return true;
        SDL_Delay(5);
    }
    return false;
}

/* appends the property's bytes (and deletes it, which asks an INCR sender for more); -1 if it isn't there */
static long x11_take(uint8_t **buf, size_t *len, Atom *type) {
    int fmt;
    unsigned long items, after;
    unsigned char *data = NULL;
    if (X.GetWindowProperty(X.d, X.w, X.prop, 0, LONG_MAX / 4, True, AnyPropertyType, type, &fmt, &items, &after, &data) != Success)
        return -1;
    size_t got = fmt == 8 ? items : fmt == 16 ? items * 2 : items * 4;
    if (*type == X.incr) got = 0;
    if (got && *len + got <= MAX_PNG) {
        uint8_t *nb = realloc(*buf, *len + got);
        if (nb) { memcpy(nb + *len, data, got); *buf = nb; *len += got; }
    }
    if (data) X.Free(data);
    return (long)got;
}

static uint8_t *x11_png(SDL_Window *own, size_t *n) {
    if (!x11_open()) return NULL;
    Window owner = X.GetSelectionOwner(X.d, X.clipboard);
    if (owner == None) return NULL;
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    /* our own window has only text (and SDL can't answer while we wait here) */
    if (own && SDL_GetWindowWMInfo(own, &wm) && wm.subsystem == SDL_SYSWM_X11 && wm.info.x11.window == owner) return NULL;
    XEvent ev;
    while (X.CheckTypedWindowEvent(X.d, X.w, PropertyNotify, &ev)) {}   /* old news */
    X.ConvertSelection(X.d, X.clipboard, X.png, X.prop, X.w, CurrentTime);
    X.Flush(X.d);
    if (!x11_wait(SelectionNotify, &ev) || ev.xselection.property == None) return NULL;   /* no picture */
    uint8_t *buf = NULL;
    size_t len = 0;
    Atom type;
    if (x11_take(&buf, &len, &type) < 0) return NULL;
    if (type == X.incr) {                     /* a big one comes in pieces, ending with an empty one */
        X.Flush(X.d);
        for (;;) {
            do { if (!x11_wait(PropertyNotify, &ev)) { free(buf); return NULL; } }
            while (ev.xproperty.atom != X.prop || ev.xproperty.state != PropertyNewValue);
            long got = x11_take(&buf, &len, &type);
            X.Flush(X.d);
            if (got <= 0) break;
        }
    }
    if (!len) { free(buf); return NULL; }
    *n = len;
    return buf;
}
#endif

#if defined(__linux__) && !defined(GE_PLAYER) && !defined(__EMSCRIPTEN__)
/* Wayland: the wl-clipboard tool, if it's there */
static uint8_t *wl_png(size_t *n) {
    if (!getenv("WAYLAND_DISPLAY")) return NULL;
    FILE *p = popen("wl-paste --no-newline --type image/png 2>/dev/null", "r");
    if (!p) return NULL;
    uint8_t *buf = NULL;
    size_t len = 0, cap = 0;
    for (;;) {
        if (len == cap) {
            if (cap >= MAX_PNG) break;
            cap = cap ? cap * 2 : 1 << 16;
            uint8_t *nb = realloc(buf, cap);
            if (!nb) break;
            buf = nb;
        }
        size_t got = fread(buf + len, 1, cap - len, p);
        if (!got) break;
        len += got;
    }
    pclose(p);
    if (len < 8 || memcmp(buf, "\x89PNG", 4)) { free(buf); return NULL; }
    *n = len;
    return buf;
}
#endif

uint8_t *clip_image_png(SDL_Window *own, size_t *n) {
    uint8_t *png = NULL;
#ifdef HAVE_X11
    if (!png) png = x11_png(own, n);
#endif
#if defined(__linux__) && !defined(GE_PLAYER) && !defined(__EMSCRIPTEN__)
    if (!png) png = wl_png(n);
#endif
    (void)own;
    return png;
}
