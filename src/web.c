/* The browser build's bridges to JavaScript: downloads, uploads and dropped
 * files. Elsewhere these are no-ops (a native window gets SDL_DROPFILE). */
#include "engine.h"
#include <stdio.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

/* the file at `path` (in the in-browser file system) becomes a download */
EM_JS(void, js_download, (const char *path), {
    var p = UTF8ToString(path);
    var data = FS.readFile(p);
    var blob = new Blob([data], {type: "application/octet-stream"});
    var a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = p.split("/").pop();
    document.body.appendChild(a);
    a.click();
    setTimeout(function() { URL.revokeObjectURL(a.href); a.remove(); }, 1000);
});

/* the page's upload / drag-and-drop code writes files to /tmp/upload and
 * queues their paths in Module.geDropped; we take one per frame */
EM_JS(int, js_take_dropped, (char *out, int n), {
    var q = Module.geDropped;
    if (!q || !q.length) return 0;
    stringToUTF8(q.shift(), out, n);
    return 1;
});

EM_JS(void, js_pick_file, (void), {
    if (Module.gePickFile) Module.gePickFile();
});

void web_offer_download(const char *path) { js_download(path); }
bool web_take_dropped(char *path, size_t n) { return js_take_dropped(path, (int)n) != 0; }
void web_pick_file(void) { js_pick_file(); }

#else

void web_offer_download(const char *path) { (void)path; }
bool web_take_dropped(char *path, size_t n) { (void)path; (void)n; return false; }
void web_pick_file(void) {}

#endif
