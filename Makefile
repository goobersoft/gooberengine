# GooberEngine build
#
# The GooberBASIC language (lexer, compiler, VM, core natives) is compiled
# straight from the sibling gooberbasic checkout. Its SDL layer (gfx.c) and
# command-line main.c are NOT used: src/api.c provides the console API instead.
#
#   make              the engine, for this machine (uses the system SDL2)
#   make test         the test suite
#   make runtimes     the player runtimes that EXPORT packs games into:
#                       runtimes/player-linux-x86_64        (SDL2 linked in)
#                       runtimes/player-windows-x86_64.exe  (mingw-w64, SDL2 linked in)
#                       runtimes/web/{index.html,player.js,player.wasm}  (Emscripten)
#   make web          the whole engine for the browser, in build/web, then serve it
#   make web-build    ...just build it
GB      ?= ../gooberbasic
CC      ?= cc
CFLAGS  ?= -O2 -g
WARN    := -std=c11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wno-unused-parameter -Wno-format-truncation
CFLAGS  += $(WARN)
CPPFLAGS += -I$(GB)/src -Isrc
SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS   := $(shell pkg-config --libs sdl2)
LDLIBS  += $(SDL_LIBS) -lm

GB_SRC := lexer.c compiler.c vm.c objects.c builtins.c
GB_OBJ := $(GB_SRC:%.c=build/gb/%.o)
GE_SRC := $(wildcard src/*.c)
GE_OBJ := $(GE_SRC:src/%.c=build/%.o)
HDRS   := $(wildcard src/*.h) $(wildcard $(GB)/src/*.h)

all: gooberengine

gooberengine: $(GE_OBJ) $(GB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.c $(HDRS) | build
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -c $< -o $@

build/gb/%.o: $(GB)/src/%.c $(HDRS) | build
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

build:
	mkdir -p build/gb

# every cart must compile; then boot the shell and each cart headless for a few frames
test: gooberengine
	./tests/run_tests.sh

# GooberBASIC's VS Code extension, with the console API highlighted too
vscode:
	python3 $(GB)/editors/vscode/generate.py --with src/api.c

# ---------------------------------------------------------------------------
# SDL2 for the runtimes, built from source (third_party/SDL2-2.32.10.zip), so
# the games don't need SDL installed: on Linux its video and sound back ends
# (X11, Wayland, PulseAudio, PipeWire, ALSA) are loaded when the game starts,
# whichever the player's machine has; on Windows everything is in the .exe.
SDL_ZIP   ?= third_party/SDL2-2.32.10.zip
SDL_SRC   := build/sdl-src/SDL-release-2.32.10
SDL_LINUX := build/sdl-linux/install
SDL_WIN   := build/sdl-win64/install
SDL_CMAKE := -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF -DSDL_STATIC_PIC=ON \
             -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
MINGW     ?= x86_64-w64-mingw32
JOBS      ?= $(shell nproc 2>/dev/null || echo 4)

$(SDL_SRC)/CMakeLists.txt: $(SDL_ZIP)
	rm -rf build/sdl-src && mkdir -p build/sdl-src
	cd build/sdl-src && unzip -q ../../$(SDL_ZIP)
	touch $@

$(SDL_LINUX)/lib/libSDL2.a: $(SDL_SRC)/CMakeLists.txt
	cmake -S $(SDL_SRC) -B build/sdl-linux/obj $(SDL_CMAKE) -DSDL_LIBSAMPLERATE=OFF \
	      -DCMAKE_INSTALL_PREFIX=$(abspath $(SDL_LINUX)) > build/sdl-linux.log
	cmake --build build/sdl-linux/obj -j$(JOBS) >> build/sdl-linux.log
	cmake --install build/sdl-linux/obj >> build/sdl-linux.log

$(SDL_WIN)/lib/libSDL2.a: $(SDL_SRC)/CMakeLists.txt
	@command -v $(MINGW)-gcc >/dev/null || { echo "need mingw-w64: sudo apt install mingw-w64"; exit 1; }
	cmake -S $(SDL_SRC) -B build/sdl-win64/obj $(SDL_CMAKE) -DCMAKE_SYSTEM_NAME=Windows \
	      -DCMAKE_C_COMPILER=$(MINGW)-gcc -DCMAKE_CXX_COMPILER=$(MINGW)-g++ -DCMAKE_RC_COMPILER=$(MINGW)-windres \
	      -DCMAKE_INSTALL_PREFIX=$(abspath $(SDL_WIN)) > build/sdl-win64.log
	cmake --build build/sdl-win64/obj -j$(JOBS) >> build/sdl-win64.log
	cmake --install build/sdl-win64/obj >> build/sdl-win64.log

ALL_SRC := $(GE_SRC) $(GB_SRC:%=$(GB)/src/%)
PLAYER_FLAGS := -O2 $(WARN) -DGE_PLAYER $(CPPFLAGS)

runtimes/player-linux-x86_64: $(ALL_SRC) $(HDRS) $(SDL_LINUX)/lib/libSDL2.a
	mkdir -p runtimes
	$(CC) $(PLAYER_FLAGS) -I$(SDL_LINUX)/include -D_REENTRANT -o $@ $(ALL_SRC) \
	      $(SDL_LINUX)/lib/libSDL2.a -lm -ldl -lpthread -lrt -static-libgcc -s

runtimes/player-windows-x86_64.exe: $(ALL_SRC) $(HDRS) $(SDL_WIN)/lib/libSDL2.a
	mkdir -p runtimes
	$(MINGW)-gcc $(PLAYER_FLAGS) -I$(SDL_WIN)/include -o $@ $(ALL_SRC) \
	      -static -L$(SDL_WIN)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows \
	      -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
	      -lshell32 -lsetupapi -lversion -luuid -lcfgmgr32 -lpthread -s

# ---------------------------------------------------------------------------
# The browser. Emscripten comes from emsdk (https://emscripten.org); point
# EMSDK at its folder. ASYNCIFY lets the engine's frame loop hand control back
# to the browser every frame without restructuring it.
EMSDK  ?= $(HOME)/github/emsdk
EMCC   ?= EM_CONFIG=$(EMSDK)/.emscripten $(EMSDK)/upstream/emscripten/emcc
WEB_FLAGS := -O2 $(WARN) $(CPPFLAGS) -sUSE_SDL=2 -sASYNCIFY -sASYNCIFY_STACK_SIZE=131072 \
             -sSTACK_SIZE=4MB -sINITIAL_MEMORY=128MB -lidbfs.js \
             -sEXPORTED_RUNTIME_METHODS=FS,addRunDependency,removeRunDependency,callMain \
             -sENVIRONMENT=web

runtimes/web/player.js: $(ALL_SRC) $(HDRS) web/player.html
	@test -x $(EMSDK)/upstream/emscripten/emcc || { echo "need Emscripten: install emsdk and set EMSDK=/path/to/emsdk"; exit 1; }
	mkdir -p runtimes/web
	$(EMCC) $(WEB_FLAGS) -DGE_PLAYER -o $@ $(ALL_SRC)
	cp web/player.html runtimes/web/index.html

runtimes: runtimes/player-linux-x86_64 runtimes/player-windows-x86_64.exe runtimes/web/player.js

CARTS := $(shell find carts -type f -not -name '*.unsaved' -not -name '*.saving' 2>/dev/null)

build/web/index.html: $(ALL_SRC) $(HDRS) web/engine.html $(CARTS)
	@test -x $(EMSDK)/upstream/emscripten/emcc || { echo "need Emscripten: install emsdk and set EMSDK=/path/to/emsdk"; exit 1; }
	mkdir -p build/web
	$(EMCC) $(WEB_FLAGS) --shell-file web/engine.html --preload-file carts@/bundled-carts \
	      -o $@ $(ALL_SRC)

# the web engine exports games too: it fetches the runtimes from runtimes/ on its server
web-build: build/web/index.html
	@if [ -d runtimes ]; then rm -rf build/web/runtimes && cp -r runtimes build/web/runtimes; \
	 else echo "(no runtimes/ yet: EXPORT in the web engine needs: make runtimes)"; fi

PORT ?= 8000
web: web-build
	@echo "GooberEngine for the browser: http://localhost:$(PORT)/"
	python3 -m http.server $(PORT) --bind 127.0.0.1 -d build/web

clean:
	rm -rf build gooberengine

# also forget the built runtimes
distclean: clean
	rm -rf runtimes exports

.PHONY: all test clean distclean vscode runtimes web web-build
