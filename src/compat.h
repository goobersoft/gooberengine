/* The few file-system calls that differ between Linux, Windows (mingw-w64) and
 * the browser (Emscripten). Everything else in the engine is plain C11 + SDL2. */
#ifndef GE_COMPAT_H
#define GE_COMPAT_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
/* mkdir takes no mode, realpath is _fullpath, and rename must be told it may
 * replace the file (POSIX rename always does; the engine's saves rely on it) */
#define mkdir(path, mode) _mkdir(path)
#define realpath(path, out) _fullpath((out), (path), PATH_MAX)
int ge_rename(const char *from, const char *to);
#define rename(from, to) ge_rename((from), (to))
#endif

/* the running executable's own path (for reading a packed cart from its end) */
bool ge_exe_path(char *out, size_t n);
/* a fresh, empty, private folder for unpacking a cart; NULL on failure */
const char *ge_temp_dir(void);

#endif
