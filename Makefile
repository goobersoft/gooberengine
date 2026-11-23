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
#   make web          the whole engine for the browser, in build/web (web-build is the same)
GB      ?= ../gooberbasic
CC      ?= cc
CFLAGS  ?= -O2 -g
WARN    := -std=c11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wno-unused-parameter -Wno-format-truncation
CFLAGS  += $(WARN)
CPPFLAGS += -I$(GB)/src -Isrc
SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS   := $(shell pkg-config --libs sdl2)
LDLIBS  += $(SDL_LIBS) -lm

GB_SRC := lexer.c compiler.c vm.c objects.c builtins.c net.c
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

# the in-console manual (F1): help.txt's pages as C strings, so every build
# (the web and Windows ones too) has them without reading a file. The result
# is kept in the repository, so building doesn't need this rule to run.
src/help_pages.c: src/help.txt
	{ echo '/* made from help.txt by make: edit that, not this */'; \
	  echo 'typedef struct { const char *names, *title, *body; } HelpPage;'; \
	  echo 'const HelpPage help_pages[] = {'; \
	  sed -e '/^;/d' -e 's/\\/\\\\/g' -e 's/"/\\"/g' \
	      -e 's/^@ *\([^|]*[^| ]\) *| *\(.*\)$$/}, {"\1", "\2", ""/' \
	      -e '/^}, {"/!s/.*/    "&\\n"/' $< | sed -e '1s/^}, //'; \
	  echo '}};'; \
	  echo 'const int help_npages = sizeof help_pages / sizeof help_pages[0];'; } > $@

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
	      -lshell32 -lsetupapi -lversion -luuid -lcfgmgr32 -lws2_32 -lpthread -s

# ---------------------------------------------------------------------------
# The browser. Emscripten comes from the emsdk in tools/emsdk
# (https://emscripten.org); pass EMSDK=/path/to/emsdk to use another one.
# ASYNCIFY lets the engine's frame loop hand control back to the browser every
# frame without restructuring it.
EMSDK  ?= $(abspath tools/emsdk)
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

# the web engine exports games too: it fetches runtimes.zip (runtimes/, zipped) from its server
# $(1) = the zip to write
runtimes_zip = rm -rf $(dir $(1))runtimes $(1) && (cd runtimes && zip -qr9 $(abspath $(1)) .)

web: build/web/index.html
	@if [ -d runtimes ]; then $(call runtimes_zip,build/web/runtimes.zip); \
	 else echo "(no runtimes/ yet: EXPORT in the web engine needs: make runtimes)"; fi

web-build: web

# ---------------------------------------------------------------------------
# Release zips (for itch.io and the like), in dist/:
#   gooberengine-linux-x86_64.zip    the engine, SDL2 linked in, + carts/ + runtimes/
#   gooberengine-windows-x86_64.zip  the same for Windows
#   gooberengine-web.zip             the browser engine: index.html at the top, as itch wants
# The desktop engines find carts/ and runtimes/ beside themselves (cart.c, export.c).
DIST_LINUX := build/dist/linux/gooberengine
DIST_WIN   := build/dist/windows/gooberengine
DIST_SKIP  ?= test untitled
DIST_CARTS := $(filter-out %.unsaved %.saving %.old $(DIST_SKIP:%=carts/%/%),$(CARTS))

$(DIST_LINUX)/gooberengine: $(ALL_SRC) $(HDRS) $(SDL_LINUX)/lib/libSDL2.a
	mkdir -p $(@D)
	$(CC) -O2 $(WARN) $(CPPFLAGS) -I$(SDL_LINUX)/include -D_REENTRANT -o $@ $(ALL_SRC) \
	      $(SDL_LINUX)/lib/libSDL2.a -lm -ldl -lpthread -lrt -static-libgcc -s

$(DIST_WIN)/gooberengine.exe: $(ALL_SRC) $(HDRS) $(SDL_WIN)/lib/libSDL2.a
	mkdir -p $(@D)
	$(MINGW)-gcc -O2 $(WARN) $(CPPFLAGS) -I$(SDL_WIN)/include -o $@ $(ALL_SRC) \
	      -static -L$(SDL_WIN)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows \
	      -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
	      -lshell32 -lsetupapi -lversion -luuid -lcfgmgr32 -lws2_32 -lpthread -s

# the carts that ship, copied once into build/dist/carts
build/dist/carts.stamp: $(DIST_CARTS)
	rm -rf build/dist/carts && mkdir -p build/dist/carts
	cd carts && cp --parents $(DIST_CARTS:carts/%=%) ../build/dist/carts/
	touch $@

# $(1) = the folder that becomes the top of the zip, $(2) = the zip
define dist_zip
	rm -rf $(1)/carts $(1)/runtimes
	mkdir -p dist
	cp -r build/dist/carts $(1)/carts
	cp -r runtimes $(1)/runtimes
	cp README.md $(1)/
	rm -f $(2) && cd $(dir $(1)) && zip -qr9 $(abspath $(2)) $(notdir $(1))
endef

dist/gooberengine-linux-x86_64.zip: $(DIST_LINUX)/gooberengine runtimes build/dist/carts.stamp README.md
	$(call dist_zip,$(DIST_LINUX),$@)

dist/gooberengine-windows-x86_64.zip: $(DIST_WIN)/gooberengine.exe runtimes build/dist/carts.stamp README.md
	$(call dist_zip,$(DIST_WIN),$@)

# its own web build, so only the shipped carts are preloaded (make web keeps them all)
build/dist/web/index.html: $(ALL_SRC) $(HDRS) web/engine.html build/dist/carts.stamp
	@test -x $(EMSDK)/upstream/emscripten/emcc || { echo "need Emscripten: install emsdk and set EMSDK=/path/to/emsdk"; exit 1; }
	mkdir -p build/dist/web
	$(EMCC) $(WEB_FLAGS) --shell-file web/engine.html --preload-file build/dist/carts@/bundled-carts \
	      -o $@ $(ALL_SRC)

dist/gooberengine-web.zip: build/dist/web/index.html runtimes
	$(call runtimes_zip,build/dist/web/runtimes.zip)
	mkdir -p dist && rm -f $@
	cd build/dist/web && zip -q9 $(abspath $@) index.html index.js index.wasm index.data runtimes.zip

dist: dist/gooberengine-linux-x86_64.zip dist/gooberengine-windows-x86_64.zip dist/gooberengine-web.zip
	@ls -l dist

clean:
	rm -rf build gooberengine

# also forget the built runtimes and release zips
distclean: clean
	rm -rf runtimes exports dist

.PHONY: all test clean distclean vscode runtimes web web-build dist
