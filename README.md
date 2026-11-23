# GooberEngine

An all-in-one fantasy console whose programming language is **GooberBASIC**.
It's a fixed virtual machine with a 400×240 screen, 64 colors and 10×10 tiles.
It boots into a shell and runs *folder cartridges*. The editors for code, tiles,
maps and sound are built into the console. You can still use your own text
editor on a cart folder at any time.

```
./gooberengine                   boot into the shell
./gooberengine examples/seedling boot straight into a cart (Esc returns to the shell)
```

## The machine

| Part | Spec |
|---|---|
| Screen | 400 × 240 pixels, 60 frames per second, scaled ×3 in a resizable window (F11 = fullscreen) |
| Color | 64 fixed colors: 6-bit RGB, 2 bits per channel. Index = `r*16 + g*4 + b`, each channel 0–3 |
| Tiles | a tileset is a 400 × 240 sheet: 960 tiles of 10 × 10 pixels, 40 per row. A pixel is a color or transparent |
| Map | 1 to 10 screens each way (a screen is 40 × 24 cells), so up to 400 × 240 cells. A cell holds a tile number 0–959 or nothing; tile 0 is a tile like any other |
| Slots | 10 slots each for tilesets, maps, sound banks and songs. A cart loads files into slots and swaps between them while it runs |
| Dots | a cart may draw 100,000 pixels a frame (`dot_limit` changes it, up to 1,000,000). The screen alone is 96,000, so redrawing only what changed pays off |
| Text | built-in 6×10 font, letters 5 wide: 66 × 24 characters. A cart can bring fonts of its own, 4×4 to 20×20. Capitals and lowercase letters, with descenders (see [The font](#the-font)) |
| Input | a pad with a d-pad, three buttons **A B C** and **Start** (keyboard or game controller), plus raw keyboard and mouse |
| Sound | 8 voices. Each plays one of 8 drawable waveforms (32 samples each), one of 8 instruments (sound effects 0–7 played as a note) or noise, at 64 pitches (C-2 to D#7) and 8 volumes. There are 64 sound effects |

| Pad button | Keyboard | Game controller (Xbox-style names) |
|---|---|---|
| d-pad | arrow keys | d-pad or left stick |
| **A** (`BTN_A`) | **Z** or Space | **X** (left face button) |
| **B** (`BTN_B`) | **X** | **A** (bottom face button) |
| **C** (`BTN_C`) | **C** | **B** (right face button) |
| **Start** (`BTN_START`) | **Enter** | **Start** |

The controller's Y (top) button is not used.
Esc always stops the cart and returns to the shell, and F12 saves a screenshot.

## Building

GooberEngine needs a C compiler, `make`, `pkg-config`, and SDL2 (`libsdl2-dev`). It writes its own PNGs, so SDL2_image is no longer needed.
It also needs the **GooberBASIC** checkout next to it, at `../gooberbasic`. To use a checkout somewhere else, pass `make GB=/path/to/gooberbasic`.

```bash
make
make test
```

| Target | What it builds | Needs |
|---|---|---|
| `make` | `./gooberengine` for this machine | the above |
| `make test` | runs the test suite (`GE_TEST_WINE=1 make test` adds the Windows game under wine) | the above |
| `make runtimes` | the player runtimes that `EXPORT` packs games into, in `runtimes/` | `cmake`, `unzip`, `mingw-w64`, Emscripten |
| `make web` | the whole engine for the browser, in `build/web` (`make web-build` is the same) | Emscripten |

Emscripten comes from [emsdk](https://emscripten.org/docs/getting_started/downloads.html). The Makefile uses the copy in `tools/emsdk` (set it up once with `tools/emsdk/emsdk install latest && tools/emsdk/emsdk activate latest`); pass `EMSDK=/path/to/emsdk` for another place. Windows builds use the `mingw-w64` cross compiler (`sudo apt install mingw-w64`). SDL2 for the runtimes is built from `third_party/SDL2-2.32.10.zip` the first time, which takes a minute or two.

`make vscode` regenerates GooberBASIC's VS Code grammar (in `../gooberbasic/editors/vscode`) with the console API added. Commands like `rect_fill` and `music`, and constants like `BTN_A` and `COL_NAVY`, are then highlighted in `.gb` files. This rewrites the shared extension's grammar, so engine names light up in plain GooberBASIC programs too. Running GooberBASIC's own `make vscode` puts the plain grammar back.

### How it reuses GooberBASIC

GooberEngine is not a fork of the language. The Makefile compiles GooberBASIC's `lexer.c`, `compiler.c`, `vm.c`, `objects.c`, `builtins.c` and `net.c` straight from `../gooberbasic/src`. A fix to the language therefore reaches both programs.

GooberBASIC finds native commands in three tables:

- `gb_core_natives`: print, math, strings, lists and so on.
- `gb_gfx_natives` / `gb_gfx_consts`: the standalone `goober` binary gets these from its SDL layer, `gfx.c`.
- `gb_net_natives` / `gb_net_consts`: networking (`net.c`), the same in both programs. GooberEngine polls it from `flip()`, and closes the game when a cart stops.

GooberEngine does **not** link `gfx.c` or `main.c`. It supplies [src/api.c](src/api.c) as the graphics table instead. So a cart sees the core language plus the console hardware, and nothing else: no windows, image files or TTF fonts.

One change was made to the shared code. It adds a small embedding hook in `common.h` / `objects.c`:

- `gb_diag_hook` redirects diagnostics.
- `gb_error_jmp` lets lexer and compiler errors `longjmp` back to the host instead of calling `exit(1)`.

Both are NULL by default, so `goober` behaves exactly as before. This is how a cart with a syntax error drops you back at the shell prompt, error shown in red, instead of killing the console.

For the code editor, GooberBASIC also exports two read-only lookups: `lex_is_keyword()` in `lexer.c` and `compiler_builtin_word()` in `compiler.c`. With `native_find()` and the console's constant table, these give the editor its syntax colors straight from the language's own tables, so they can't drift apart.

## Shell

| Command | What it does |
|---|---|
| `DIR` | list the carts in the folder you're in, the folders of carts under it (`examples/`), and the [zipped carts](#sharing-carts) not unpacked yet, with their title, version and author |
| `CD folder` | go into a folder of carts inside the carts folder: `CD examples`. `CD ..` goes back up, `CD` alone (or `CD /`) to the top. `DIR`, `LOAD`, `RUN`, `NEW`, `SAVE` and `BROWSE` then work in that folder, and the prompt shows its name |
| `LOAD name` | load a cart by name, or by path to its folder. With no folder of that name, a `name.zip` (`.tar.gz`, `.tgz`, `.tar`) is unpacked into one first: see [Sharing carts](#sharing-carts) |
| `RUN [name]` | save any code being edited, then run the loaded cart (or load + run) |
| `EDIT` | open the code editor (or Esc on an empty prompt) |
| `TILES` | open the tile editor |
| `MAP` | open the map editor |
| `SOUND` | open the sound editor |
| `MUSIC` | open the music editor |
| `FONT` | open the font editor |
| `NEW [name]` | make a cart folder with a starter `main.gb` and open it in the editor. Without a name: a fresh untitled cart |
| `SAVE [name]` | save everything being edited: code, tiles, map, sound. On an untitled cart, `SAVE mygame` makes `carts/mygame`, and `SAVE` alone makes `carts/untitled` (asking first if that name is taken) |
| `BROWSE` | pick a cart from a list: Up/Down, PgUp/PgDn, Home/End or the mouse (the wheel scrolls), Enter (or a second click) runs, L loads |
| `EXPORT [file]` | zip the loaded cart folder: `carts/<name>.zip`, or `file` (`.zip` is added; `file.tar.gz` writes a gzipped tar). SAVE first. See [Sharing carts](#sharing-carts) |
| `EXPORT target [folder]` | make a standalone game: `LINUX`, `WINDOWS`, `WEB` or `ALL` (several may be given). Written to `exports/<name>/` unless a folder is given. See [Exporting games](#exporting-games) |
| `UPLOAD` / `DOWNLOAD` | web version only: open a cart `.zip` from your computer / save the cart to it as a `.zip` |
| `IMPORT file [name]` | unpack a `.zip` (or `.tar.gz`, `.tgz`, `.tar`) into a new cart folder and load it. `LOAD file.zip` does the same, named after the file |
| `INFO` | title, author, tiles used, folder |
| `THEME [name]` | the editors' colors: `blue` (the default), `green`, `orange` or `grey` (`white` works too). Remembered for next time. `THEME` alone shows the one in use |
| `SETTINGS` | open the [settings](#settings) screen (F10): theme, volume, fullscreen, window size, indent, right click, effects |
| `HELP [word]` | list the commands; with a word, open its page in the [manual](#help) (`DOCS` alone opens the contents) |
| `FOLDER` | where carts are read from |
| `CLS` | clear the screen |
| `QUIT` | leave |

GooberEngine boots into a new **untitled cart**, so you can start typing code (`EDIT`) right away. Nothing is written to the carts folder until you save it with `SAVE name` (or `SAVE` alone, which names it `untitled`). Until then Ctrl+S in the editors only reminds you to, `RUN` works as usual, and `EXPORT` asks for a name first. Closing the window with unsaved untitled work keeps it in a scratch folder in `$TMPDIR` and prints where.

**Themes** recolor the shell, the editors and the file tree: their bars, highlights and backgrounds. Carts, the tileset and map you're editing, and the code editor's syntax colors look the same in every theme. Pick one with `THEME` or on the [settings](#settings) screen.

Up/Down recalls history. If anything being edited has unsaved changes, `LOAD`, `NEW` and `RUN name` ask `discard them? (Y/N)` first. Press Y to discard the changes and go ahead, or N (or Esc) to keep them. `QUIT` asks `save changes? (Y/N)` instead: Y saves everything and quits, N quits without saving, and Esc stays in the shell. If the window is closed with unsaved work, it is kept next to each file as `NAME.unsaved` (`main.gb.unsaved`, `tiles/tileset.g64.unsaved` …).

To switch between the code, tile, map, sound, music and font editors, pick a file in the [file tree](#the-file-tree) or click the tabs at the top of any editor. From the shell, `EDIT`, `TILES`, `MAP`, `SOUND`, `MUSIC` and `FONT` open them.

| Key | |
|---|---|
| F1 | [help](#help): the page for what's under the cursor; in the help, the index |
| F9 | show or hide the [file tree](#the-file-tree) |
| F10 | the [settings](#settings) screen |
| F11 (or Alt+Enter) | fullscreen |
| F12 | screenshot |

## Help

F1 opens the manual inside the console, from anywhere. Its pages cover the console, its editors, the GooberBASIC language and every command.

- **In the code editor**, F1 opens the page for the word under the cursor (or just before it): a keyword (`if`, `eachin`), a type (`float`, `string`), a command (`rect_fill`, `left$`), or a constant (`KEY_SPACE`, `COL_RED`). On a number, a string, a comment or an operator it opens the page about those. For a name the console doesn't know (one of the cart's own), it says so and shows the pages whose names are close.
- **In the shell**, F1 opens the index of every page. With a command typed (`load`, `export` …), the index is narrowed to the names that hold it, the command first, so Enter opens its page. Esc goes straight back to the prompt. `HELP word` opens a word's page.
- **In the other editors**, F1 opens that editor's page, with its keys.

In the manual, the arrows, PgUp/PgDn, Home/End and the wheel scroll. Tab / Shift+Tab pick a link, and Enter or a click follows it. The top bar has buttons for the mouse: **< back** and **forward >** step through the places you've been, the index included (Backspace or Alt+Left and Alt+Right do the same; back from the first place leaves the manual), **contents**, **index**, and **close**. **F1 in the manual shows the index**: every page's names, A to Z, each with its page's title (`HELP index` opens it from the shell). Typing narrows it to the names with what you typed, from anywhere in the manual. Up/Down pick a name, and Enter or a click opens its page; F1 again goes back to the page you were on. Esc returns to where you were, with nothing lost.

The pages are written in [src/help.txt](src/help.txt), in a small markup described at its top. `make` turns it into `src/help_pages.c`, so the pages are built in and the web and Windows builds have them too. `./gooberengine --help-check` (part of `make test`) checks that every keyword, type, command and constant has a page and every link leads somewhere.

## Settings

F10, `SETTINGS` in the shell, or the **settings** button at the bottom left of the file tree opens the settings screen. It works with or without a cart loaded. Up / Down pick a line, and Left / Right (or the mouse) change it. Enter turns fullscreen on or off. Esc, or F10 again, goes back to where you were.

| Setting | |
|---|---|
| theme | the shell's and editors' colors: blue, green, orange or grey (click one) |
| volume | all sound, 0–10. Changing it plays a short note at the new level |
| music | the music, 0–10, on top of the volume |
| sound fx | sound effects and `note()`s, 0–10, on top of the volume |
| fullscreen | on or off (F11, or Alt+Enter, does the same) |
| window size | 1× to 8× (`--scale` on the command line wins for that run) |
| indent | how Enter indents in the code editor: **smart** (keep the indent, and add one after `if`, `for` …), **auto** (just keep the indent; Backspace straight after Enter takes the whole newline back), **none** |
| right click | the tile editor's right button: **eyedrop** (pick a color up into A, the default) or **color B** (draw with color B) |
| effects | opens the post-processing effects page (below) |

### Post-processing effects

The **effects** line opens a second page of sliders that change how the screen is *shown*, never what is drawn: carts, `pget()` and F12 screenshots still see the plain picture. Drag a slider, or use Up / Down and Left / Right (Shift moves by 1, Home / End jump to the ends). Esc goes back to the settings.

| Effect | |
|---|---|
| crt bend | 0–100%: the picture bulges like the glass of a tube. The mouse is bent the same way, so it still points at the pixel you see under it |
| tint | 0–359°: a hue shift; every color turns around the color wheel, brightness kept |
| saturation | 0–100%: 100 is the console's own colors, 0 is grey |
| scanlines | 0–100%: darkens the screen row under each row of console pixels (100 makes it black). There's no such row at 1×, so it needs 2× or more |
| screen burn | 0–80%: a pixel that goes dark fades out over a few frames instead of snapping off, like phosphor |
| reset | every effect off |

Every change takes effect at once and is kept for next time in `settings.txt`, beside the save-data folder. That's `~/.local/share/gooberengine/settings.txt`, `%APPDATA%\GooberEngine\settings.txt` on Windows, the browser's storage on the web, or `$GOOBERENGINE_SETTINGS`. The file is `key = value` lines (`theme`, `volume`, `music_volume`, `sfx_volume`, `fullscreen`, `scale`, `indent`, `tile_right`, `fx_crt`, `fx_tint`, `fx_saturation`, `fx_scanlines`, `fx_burn`), and other lines in it are left alone. Exported games don't read it: the settings belong to the console, not to the games it makes.

## The file tree

Every editor has the cart's files down its left side. The editor fills the rest of the screen.

```
files     <        < hides the tree (so does F9, or the button left of the tabs); right-click a file: rename, delete
v code    +        + makes a new file: code-1, tiles-1, map-1, sfx-1, music-1, font-1 ...
 - main            main.gb: no slot, it always runs
 1 enemies         the slot a file loads into by itself when the game runs (see below)
   scratch         not in a slot: only load_... brings it in
v tiles   +        click a file to open it in its editor; click a section's name to fold it
 0 tileset
   tiles-2*        * = changed, not saved
> maps 2  +         a folded section: how many files it holds (2* if one isn't saved)
v sfx     +
 sfx
v music   +
 music
v fonts   +
 font
settings           bottom left: the settings screen
```

Code is always a `.gb` file, so the tree and the code editor leave the `.gb` off: `main` is `main.gb`, `enemies` is `enemies.gb`. On disk, in `include "enemies.gb"`, and in error messages the files keep their full names.

Click a section's name (`code`, `tiles`, `maps` …) to fold its files away, and again to bring them back. A folded section shows how many files it holds, with a `*` if any of them has unsaved changes. Its **+** still makes a new file, and unfolds the section so you see it.

Each editor has one file open, and the tree highlights it. Clicking a file opens it in its editor, which switches to that editor. `SAVE` saves every changed file. New files appear in the tree straight away and are written by the next save.

**Slots:** the number left of a file is the [slot](#slots) it loads into by itself when the game runs, so a cart needn't call `load_tileset` and friends for what it always uses (it still can, to change slots while it runs). Click the number's column to put the file in the lowest free slot, or to take it out of its slot. Right-click there to move it on to the next slot; the file already in that slot takes its old place. Files in slots are listed first, in slot order, then the rest by name. Code works the same way, except for `main.gb`: it is always listed first and shows `-` instead of a number, because it has no slot. It always runs (a cart starts there), so there's nothing to toggle, and clicking the `-` does nothing. Code files in slots are compiled in ahead of it, in slot order, as if `main.gb` began with an `include` of each: their top-level code runs first, and their `global` functions and variables are ready when `main.gb` starts. A code file without a slot runs only if something includes it. The slots are written to `cart.txt` straight away, and renaming a file keeps its slot.

**Right-click** a file for a small menu:

- **rename:** type the new name in place (letters, digits, `-` and `_`), then Enter. Esc, or a click elsewhere, cancels. The file on disk is renamed at once, and an open file keeps its unsaved changes. A cart that loads the file by name (`load_map(1, "cave")`) needs that name changed too.
- **delete:** the row asks `delete? y/n`. Y deletes the file from disk and closes it in its editor, unsaved changes and all. It can't be undone. N or Esc keeps it.

`main.gb` can't be renamed or deleted, because a cart starts there. Carts are read from `$GOOBERENGINE_CARTS`, then from `./carts`, and then from `carts/` next to the executable.

Command-line options, mostly for scripting and tests:

| Option | What it does |
|---|---|
| `--check cart` | compile only, no window |
| `--exec "cmd"` | run a shell command at boot |
| `--frames N` | quit after N frames |
| `--shot file.png` | with `--frames`: save the last frame |
| `--headless` | no window, frames run unthrottled, and shell output is echoed to stdout |
| `--wav file.wav` | with `--headless`: record everything the synth plays |
| `--type keys` | scripted input, one key per frame: plain characters, or `{enter}`, `{esc}`, `{ctrl+s}`, `{shift+down}`… (SDL key names). Mouse: `{click:x,y}`, `{rclick:x,y}`, `{mclick:x,y}`, `{drag:x,y}` (button still held), `{mouse:x,y}`, `{wheel:n}` |
| `--scale N` | window scale |
| `--export targets cart [folder]` | export a game without opening a window: `linux`, `windows`, `web`, `all`, or a comma-separated list |

Dropping a cart `.zip` or a cart folder on the window loads it, like `LOAD`.

## Code editor

`EDIT` opens the loaded cart's `main.gb`, with line numbers and syntax colors. Other `.gb` files of the cart (the ones `main.gb` includes, for example) open from the file tree. Each keeps its own cursor and undo history, and the bottom bar shows which file you're in. Variables and functions are shared by every file unless declared `local` (`local x`, `local function f()`), which keeps them to their own file. See GooberBASIC's README, under Files.
The colors mark keywords, console and core commands, type names, constants, numbers, strings and comments.

| Keys | |
|---|---|
| **Ctrl+R** | save and run. Esc in the cart comes back here. On an error, the editor jumps to the line, marks it red and shows the message. Click the message to read all of it in a panel (a message too long for the bar ends in `...`); click again or Esc to close |
| **Ctrl+S** | save |
| **Esc** | back to the shell (the code stays open; `EDIT` returns to it) |
| arrows, Home/End, PgUp/PgDn | move. Ctrl+arrows jump by word, Ctrl+Home/End go to the start / end. Home goes to the first character of the code, then to column 1 |
| Shift + any move, mouse drag | select (Shift+click extends) |
| Ctrl+A / C / X / V | select all, copy, cut, paste. With nothing selected, copy and cut take the whole line |
| Ctrl+Z / Ctrl+Y (Ctrl+Shift+Z) | undo / redo. Typing on one line is one undo step |
| Tab / Shift+Tab | indent (with a multi-line selection: the lines) / unindent |
| Ctrl+/ | comment or uncomment the line or the selected lines |
| Ctrl+D | duplicate the line |
| Ctrl+F, Ctrl+N | find (case-insensitive, wraps), find the next one |
| Ctrl+G | go to line |
| mouse wheel | scroll |

- **Indenting** (with the *smart* indent setting, the default; see [settings](#settings) for *auto* and *none*): Enter keeps the indent of the line above. After a line that opens a block (`function`, `if … ` with nothing after `then`, `for`, `while`, `repeat`, `select`, `case`, `type`, `else`), the new line is indented 2 more spaces.
- **Saving:** tabs become 2 spaces when a file is opened. Trailing spaces are dropped when saving.
- **Other files:** an error in an included file opens that file at the error line, the same as for `main.gb`. Ctrl+S saves every changed `.gb` file.

## Tile editor

`TILES` edits a tileset: 400 × 240 pixels, 960 tiles. That's more than fits beside the file tree, so the editor works like a paint program:

- **The tileset** is where you paint. Middle-drag (or Space + drag) pans it, and the wheel zooms it (1× to 16×, starting at 6×) around the mouse.
- **The selection:** a yellow box marks the tile being edited, or a 2×2 or 4×4 block of tiles with **Z**, for sprites bigger than one tile. Only the selection can be painted: a stroke, line, rectangle or fill stops at its edge, so the tiles around it are safe. Click outside it to select another tile; tile 0 is selected when the editor opens and when it opens another tileset. The lines between tiles aren't drawn across it, so a block reads as one picture; at 6× and above a faint pixel grid shows inside it instead.
- **The map's brush:** the selection is what the map editor paints with. Select a tile, switch to the map (the `map` tab or the file tree), and the pen draws that tile; select a 2×2 or 4×4 block and one click stamps the whole block.
- **The preview window** shows the selection by itself, at the biggest size that fits. It's only for looking. Drag it by its title bar, and drag its bottom-right corner to make it bigger or smaller.
- **The tools window** holds the tools (P F L R I), the block actions (Z H V T X), the 64 colors, transparent, and colors A and B. Drag it by its title bar too. The bottom bar names what's under the mouse.
- **Transparent** pixels show as a pink checkerboard (colors 50 and 34).
- **Mouse:** in the selection the left button draws with color **A**. The right button is an eyedropper: it picks the color under the mouse up into A, from anywhere on the tileset. With the **right click** [setting](#settings) on **color B**, the right button draws with color **B** instead; B starts as transparent, so it erases. Clicking a color in the palette (4 colors to a row) sets A (left) or B (right); X swaps them. A middle click (without dragging) picks a color up into A, from anywhere on the tileset, and so does the pick tool (I): left into A, right into B.
- **Running:** edits go straight into the open tileset, so Ctrl+R runs them without saving. A running cart's own `tile_plot` / `map_set` changes are undone when it stops, so every run starts from the edited files.

| Keys | |
|---|---|
| P / F / L / R / I | pencil, flood fill, line, rectangle (hold Shift for a filled one), pick a color |
| Z | edit size: 1×1, 2×2 or 4×4 tiles |
| H / V / T | flip across, flip up-down, turn a quarter clockwise (the whole block) |
| Del | clear the block |
| arrows | move the selection to another tile (the view follows it) |
| Shift+arrows | nudge the pixels one step (they wrap around) |
| = / -, mouse wheel | zoom the tileset in / out |
| Home | zoom, view and windows back to where they started |
| X | swap colors A and B |
| G | grid on/off |
| Ctrl+C / X / V | copy, cut, paste the block |
| Ctrl+Z / Ctrl+Y | undo / redo (64 steps) |
| Ctrl+S, Ctrl+R, Esc | save `tiles/NAME.g64`, run, back to the shell |

Saving writes only as much of the sheet as is used, so a tileset with ten tiles in its top row is a small file.

## Map editor

`MAP` edits a map. The view fills the editor.

- **Painting:** the left button paints with the brush and the right button picks up the tile under the mouse.
- **The brush:** one tile, or a block of tiles. It follows the tile editor: the tile selected there (tile 0 at first) is the brush, and a 2×2 or 4×4 block selected there becomes a block brush, laid out as it is on the tileset. A tile or block picked here (with the picker, the right button, I or [ / ]) is selected in the tile editor too; a picked block that isn't 2×2 or 4×4 selects its top-left tile there. **Tab** opens the tile picker, which shows the whole tileset (scroll it with the wheel, Shift+wheel, arrows or a middle drag). Click a tile, or drag across several for a block, such as a 2×2 tree. The select tool turns part of the map into a brush with Ctrl+C. **E** makes the brush the eraser, which leaves cells empty. Tile 0 is an ordinary tile.
- **Blocks:** a block brush is laid in step with where you started, so dragging tiles it neatly without overlapping. Rectangles and fills repeat it as a pattern.
- **Size:** a map is 1–10 screens wide and 1–10 high. Ctrl+→ / Ctrl+← add or remove a column of screens on the right, and Ctrl+↓ / Ctrl+↑ add or remove a row at the bottom. A new map is 4 × 4 screens.
- **Tiles:** the map is drawn with the tileset the tile editor has open. **T** switches to the next tileset.
- **Zoom:** 20, 10, 5, 2 or 1 pixels per cell. Below 5, each tile is drawn as its most common color.
- **Guides:** blue lines mark screen-sized areas (40×24 cells), and the map's edge is outlined. Outside the map is striped.
- **Running:** edits go straight into the open map, so Ctrl+R runs them without saving.

| Keys / mouse | |
|---|---|
| P / R / F / S | pen, rectangle, flood fill, select |
| Tab | tile picker |
| right button, I | pick up the tile under the mouse |
| E | brush = empty (erases) |
| [ / ] | brush = the previous / next tile |
| T | draw with the next tileset |
| Ctrl+arrows | add or remove a column / row of screens |
| mouse wheel, = / - | zoom in / out (the wheel zooms around the mouse) |
| middle drag, Space + drag, arrows (Shift: 10 cells), Home | move around the map |
| Ctrl+C / Ctrl+X, Del | copy / cut the selection into the brush, clear the selection |
| G | guides on/off |
| Ctrl+Z / Ctrl+Y | undo / redo (64 steps) |
| Ctrl+S, Ctrl+R, Esc | save `maps/NAME.g64`, run, back to the shell (Esc first drops a selection) |

## Sound editor

`SOUND` edits a sound bank: 64 sound effects and the 8 waveforms they play. Sound effects 0–7 double as instruments 8–f (see [Instruments](#instruments)).

- **Layout:**
  - On the left is the list of effects; white ones have notes.
  - Across the top are the speed, the length and the loop, plus a play button.
  - Then, for each of the 32 steps: its pitch (a bar), its volume, its wave (`0`–`7` drawn, `8`–`f` an instrument, `n` = noise) and its effect.
  - At the bottom is the wave being drawn, the 8 waves, the 8 instruments (each showing its sound effect's notes), noise, and the effect pens. With an instrument picked, click the wave box to edit its sound effect.
- **Mouse:** draw bars in the pitch and volume graphs with the left button. The right button makes a step a rest. Clicking the wave or effect row stamps the current pen; right-clicking picks it up. Draw a waveform in the wave box. Click a wave or effect pen to select it.
- **Keyboard:** typing works like a small musical keyboard. Each note you enter sounds, uses the wave pen, and moves to the next step.
- **Playing:** Space plays the effect, and the step being played lights up. Leaving the editor stops all sound.

| Keys | |
|---|---|
| Z S X D C V G B H N J M | the 12 notes of the octave (Q W E R T Y U: the white notes above) |
| [ / ] | octave down / up |
| 1–7, 0 | the step's volume, rest |
| ← → (Shift: 8 steps), Home / End | move along the steps |
| ↑ ↓ (Shift: an octave) | the step's pitch |
| Tab / Shift+Tab | the step's effect: `.` none, `s` slide to the next note, `v` vibrato, `d` drop an octave, `<` fade in, `>` fade out, `a` arpeggio (chord) |
| , / . | wave pen down / up, also applied to the step |
| Del / Backspace | rest here and move on / move back and rest |
| - / = | speed (ticks per step; 120 ticks a second) |
| Ctrl+L | the effect ends after this step |
| Ctrl+B / Ctrl+E | loop from this step / to this step (Ctrl+E again: no loop) |
| PgUp / PgDn | previous / next effect |
| Space, Esc | play / stop, stop or back to the shell |
| Ctrl+C / Ctrl+V | copy / paste a whole effect |
| Ctrl+Z / Ctrl+Y, Ctrl+S, Ctrl+R | undo / redo, save `sfx/NAME.g64`, run |

Beside the file tree there's no room for the list of effects. PgUp / PgDn switch effects, and the bottom bar shows which one is open. Hide the tree (F9) to get the list back.

### Instruments

Waves `8`–`f` are instruments: wave `8` plays sound effect 0, `9` plays 1, … `f` plays 7. A music note (or a sound-effect step, or `note()`) on an instrument plays that whole sound effect at its own speed, starting again with each note:

- **Pitch:** the sound effect is moved by the note. A step at C-4 sounds at the note's pitch, one at C-5 an octave above it.
- **Volume:** its volumes are scaled by the note's.
- **Effects:** its own effects play, and the note's effect goes on top.
- **Length:** a loop holds the note for as long as it lasts; without one the instrument falls silent at its end, like a plucked string.
- Inside an instrument, waves `8`–`f` play drawn waves `0`–`7`: instruments don't nest.

Sound banks from before instruments held 16 drawn waves; waves 8–15 of those are skipped when they're read, and saving writes 8.

## Music editor

`MUSIC` is a tracker for a song. Beside the file tree, six of the eight tracks fit. The view scrolls to follow the cursor, and `<` / `>` show that more tracks are there.

- **The song:** an order list of up to 64 entries along the top. Each entry plays a pattern, jumps to another entry (`>N`, so a song can loop) or stops (`.`). `music(n)` starts playing at entry `n`, so one cart can hold several tunes separated by stops.
- **A pattern:** 32 rows × 8 tracks. A cell is a note, a wave, a volume and an effect, the same as a sound-effect step. `...` means nothing new: the last note carries on. `===` turns the note off.
- **Voices:** track *t* plays on voice *t*. A sound effect started on a voice the music is using takes over that voice, and the track picks up again at its next note after the effect ends. `sfx()` with no voice chooses a free voice first, then one of the music's.
- **Tempo:** set with the song speed, in ticks per row (120 ticks a second).
- **The cursor:** it sits on one field of a cell: note, wave, volume or effect. On the note field, the musical-keyboard keys enter notes, as in the sound editor. On the other fields, type the value. The cursor then moves down a row.
- **Playing:** Space plays the song from the selected entry, and the screen follows it as it plays. Shift+Space loops the pattern on screen.
- **Piano roll:** Ctrl+P (or the **roll** button in the bottom bar) shows one track as a piano roll, so you can draw a tune without reading note names. Time runs left to right, one column a row; every 4th row has a line and every 8th a brighter one. Pitch runs up the screen, with a keyboard down the left and each C labelled. The lane under the mouse lights up its key, and the bottom bar names the note. **grid** (or Ctrl+P again) goes back to the tracker.
  - **Drawing:** drag with the left button to draw a note as long as the drag. Moving up and down while you drag changes its pitch, and you hear each new pitch. The right button rubs notes out. Clicking a key plays that note.
  - **Pens:** new notes take the wave and volume pens in the header (click `-` / `+`). Redrawing an existing note keeps its wave, volume and effect.
  - **Other tracks:** they show as faint lines, so you can see what fits with them. Notes of this track that are above or below the view leave an orange mark at that edge.
  - **Keys:** ←/→ move along the rows (Shift: 8), ↑/↓ scroll the pitches (Shift: an octave), and so does the mouse wheel. Tab / Shift+Tab or the numbers `1`–`8` in the header pick the track. The musical-keyboard keys enter notes at the cursor as in the grid.
  - **What it writes:** the same cells the grid shows: the note where the bar starts, `...` along it, `===` after it.

| Keys | |
|---|---|
| Z…M, Q…U (note field) | enter a note; 1 = note off |
| hex digit or n / 0–7 / `. s v d < > a` | the wave / volume / effect, on those fields |
| arrows, PgUp/PgDn, Home/End, Tab / Shift+Tab | move between rows, fields and tracks |
| Del / Backspace | clear the cell and move down / move up and clear |
| [ / ] | octave down / up |
| - / = (note field) | song speed |
| Ctrl+← / → | select an order entry (or click it) |
| Ctrl+↑ / ↓, mouse wheel on the entry | change the entry's pattern (or jump target) |
| Ctrl+J, Ctrl+. , right-click | make the entry a jump, a stop, or cycle its kind |
| Ctrl+I / Ctrl+Del | insert / delete an entry (jumps are renumbered to match) |
| Space, Shift+Space, Esc | play the song / loop the pattern, stop or back to the shell |
| Ctrl+P | the piano roll / back to the grid |
| Ctrl+C / X / V | copy / cut / paste the pattern |
| Ctrl+Z / Ctrl+Y, Ctrl+S, Ctrl+R | undo / redo, save `music/NAME.g64`, run |

## Font editor

`FONT` edits a font: the 96 glyphs, for characters 32–127, that a cart's `text()` draws with. A cart that has no font yet starts with a copy of the built-in one, so you change only the glyphs you want. Saving writes `fonts/NAME.g64`.

- **Size:** every character of a font is the same size, from 4 × 4 to 20 × 20 pixels (the built-in font is 6 × 10). Ctrl+→ / Ctrl+← make characters wider or narrower, Ctrl+↓ / Ctrl+↑ taller or shorter. Growing adds blank pixels on the right and at the bottom. Shrinking cuts off the pixels that no longer fit (Ctrl+Z brings them back).
- **Spacing is yours:** a glyph fills its whole cell, and text steps one cell across for each character. The blank column and rows that keep characters apart are part of each glyph, like the built-in font's column on the right. Leave them blank, make them wider, or draw into them for joined-up lettering.
- **Layout:** the glyphs sit at the top left, twice their size if they fit, else as they are; for big fonts the grid scrolls (the mouse wheel, or move off its edge). Beside them is the glyph being edited, big, with its size under it. Under them, sample text is drawn in the font being edited.
- **Mouse:** click a glyph to edit it. On the big glyph, the left button sets pixels and the right button clears them; drag to draw.
- **Keyboard:** typing a character picks its glyph. So `a` edits lowercase a, and Shift+A capital A. Everything else is on keys that don't type.
- **Running:** edits go straight into the open font, so Ctrl+R runs them without saving.

| Keys | |
|---|---|
| any character | edit that character's glyph |
| arrows, PgUp / PgDn | move to another glyph |
| Shift+arrows | nudge the glyph's pixels one step (they wrap around inside the cell) |
| Ctrl+arrows | make every character wider / narrower / taller / shorter (4 × 4 to 20 × 20) |
| Del / Backspace | clear the glyph |
| Ctrl+D | put the glyph back as the built-in font has it (at the top left of the cell) |
| Ctrl+C / X / V | copy, cut, paste a glyph |
| Ctrl+Z / Ctrl+Y | undo / redo (64 steps) |
| Ctrl+S, Ctrl+R, Esc | save `fonts/NAME.g64`, run, back to the shell |

## Cartridges

A cart is a folder:

```
mycart/
  main.gb          the program (may `include` other .gb files beside it)
  cart.txt         title = ..., author = ..., version = ...        optional
  tiles/NAME.g64   tilesets                                         optional
  maps/NAME.g64    maps                                             optional
  sfx/NAME.g64     sound banks: waveforms and sound effects         optional
  music/NAME.g64   songs: order list and patterns                   optional
  fonts/NAME.g64   fonts: the 96 glyphs text() draws with            optional
```

A cart can have any number of each. Each file can have a slot it loads into by itself when the cart runs: the file tree sets them, and `cart.txt` keeps them as `slot tiles 0 = tileset`, `slot code 1 = enemies` and so on. A cart that hasn't set any gets the files named `tileset`, `map`, `sfx`, `music` and `font` in slot 0 (see [Slots](#slots)). Every file is text, so carts can be diffed and edited outside the console.

**The .g64 files** are base-64 text. Each character of `0-9 a-z A-Z + *` stands for a number 0–63, in that order: `0` is 0, `a` is 10, `A` is 36, `*` is 63. A color is its palette index, so `a` is color 10 everywhere. Spaces and line breaks are ignored. GooberEngine writes one row per line.

**tiles/NAME.g64**: `XY` then the pixels, row by row. `X` and `Y` give the width and height as (digit + 1) × 10 pixels, so `00` is 10 × 10 and `jn` is 200 × 240. A tileset holds at most 400 × 240 (`dn`). A pixel is a color character, or `.` for transparent. Tile *n* is at column *n* mod 40, row *n* / 40 of the sheet.

```
10                       20 x 10 pixels: tiles 0 and 1
..........*.........     tile 1's top-left pixel is white (63)
....................
...
```

**maps/NAME.g64**: `XY` then two characters per cell, row by row. `X` and `Y` give the size in screens as digit + 1 (`0` = 1 screen, `9` = 10). A screen is 40 × 24 cells. A cell is a tile number, high digit first (`05` is tile 5, `eF` is 14 × 64 + 41 = 937), or `..` for no tile.

```
33                       4 x 4 screens: 160 x 96 cells
03030303030303...        tile 3 along the top row
03010101..0101...        .. = an empty cell
```

**sfx/NAME.g64**: `WS`, then the waves, then the sound effects. `W` and `S` give the number of waves and of sound effects as digit + 1. GooberEngine writes all 8 waves (`7`), and sound effects up to the last one that isn't empty. Waves and sound effects that aren't in the file are the defaults.

- **A wave** is 32 samples `0`–`f`, with 7–8 the middle. GooberEngine writes one wave per line.
- **A sound effect** is one line: speed − 1, length − 1, loop start, loop end, then all 32 steps. A step is four characters: the pitch (0–63: `0` is C-2, `c` C-3, `o` C-4, `A` C-5, `M` C-6), the wave (`0`–`7`, `8`–`f` for instruments, or `g` for noise), the volume 0–7, and the effect (`0` none, `1` slide, `2` vibrato, `3` drop, `4` fade in, `5` fade out, `6` arpeggio). A rest is volume 0, so an empty step reads `o000`.

```
72                                     8 waves, 3 sound effects
ffffffffffffffff0000000000000000       wave 0, a square
...
2300A360E360H350M345o000o000...        sfx 0: speed 3, length 4, no loop; C-5 E-5 G-5 C-6 on wave 3, fading out
```

**music/NAME.g64**: `SOPT`, then the order list, then the patterns. `S` is the song speed − 1 (ticks per row), and `O`, `P` and `T` give the number of order entries, patterns and tracks, each as digit + 1. GooberEngine writes the order list up to its last entry that isn't a stop, and the patterns up to the last one with notes, all as wide as the widest.

- **An order entry** is two characters: `0N` plays pattern `N`, `1N` jumps to entry `N` (so a song can loop), and `..` stops.
- **A pattern** is 32 rows, one per line, each with `T` cells of four characters: pitch, wave, volume and effect, as in a sound effect. `....` means nothing new (the note carries on), and `====` turns the note off. A blank line comes before each pattern.

```
8211                       speed 9, 3 order entries, 2 patterns, 2 tracks
000110                     play pattern 0, then 1, then jump back to entry 0

A140c365                   C-5 on wave 1 and C-3 on wave 3 fading out
........                   nothing new: the notes carry on
====7365                   track 1 stops, track 2 plays G-2
```

**fonts/NAME.g64**: `WH`, then 96 glyphs for characters 32–127 (space to the block, character 127), one per line. `W` and `H` give a character's size as digit + 1, from 4 × 4 (`33`) to 20 × 20 (`jj`); the built-in font is 6 × 10 (`59`). A glyph fills its whole cell, so the space between characters is part of the glyph. Each glyph is its `H` rows from the top. A row is one character per 6 pixels (two for 7–12 wide, and so on), and each character's top bit, 32, is the leftmost of its pixels. `w` (32) is a pixel on the left, `+` (62) the first five pixels.

```
59                       6 x 10 characters
0000000000               space
0888880800               ! : 8 is the third pixel of a row
...
000s2uyu00               a : s = .###.. 2 = ....#. u = .####. y = #...#.
```

**Old carts** have `tiles.txt`, `map.txt`, `sfx.txt` and `music.txt` at the top of the folder. Those still load, as the default tileset, map, sound bank and song. Sound and music saved before they became `.g64` are text files, `sfx/NAME.txt` and `music/NAME.txt`, and those load too. The first `SAVE` writes each one as `.g64` in its folder and renames the old file to `*.old`. Tile numbers stay the same. In an old map, tile 0 meant "nothing here", so those cells become empty. The comments of an old sound or music file are not carried over: `.g64` files hold only data.

## Console API

These are added on top of the GooberBASIC core: `print`, math, strings, lists, `rnd`, `millisecs` and the rest.
Arguments in `[brackets]` are optional.
A color is a palette index 0–63. If you leave a color out, the pen color is used.
All drawing goes through `camera` and `clip`.

### Screen

| Command | |
|---|---|
| `cls([c])` | clear the screen, or the clip rectangle if one is set (default color 0) |
| `pen(c)` | set the default color |
| `plot(x, y [, c])` / `pget(x, y)` | write / read one pixel |
| `line(x1, y1, x2, y2 [, c])` | draw a line |
| `rect(x, y, w, h [, c])` / `rect_fill(...)` | outline / filled rectangle |
| `rect_fill_fast(x, y, w, h [, c])` | filled rectangle drawn the way `cls` clears: free of the dot limit, but no blending or depth test, and its depth goes back to the far plane |
| `circle(x, y, r [, c])` / `circle_fill(...)` | outline / filled circle |
| `text(s, x, y [, c])` | draw text (`\n` breaks lines). Returns the x after the text |
| `text_width(s)` | width of a string in pixels, in the font in use (6 per character in the built-in one) |
| `font_w()` / `font_h()` | the size of a character of the font in use: `text` steps this far across, and down at a line break |
| `rgb(r, g, b)` | color index from channels 0–3 |
| `camera([x, y])` | offset all drawing by (−x, −y). With no arguments, resets |
| `pal([a, b])` | from now on, drawing in color `a` puts color `b`: recolor sprites, flash a hit enemy white. `pal()` puts every color back |
| `clip([x, y, w, h])` | limit drawing to a rectangle. With no arguments, resets |
| `spray(x, y, w, h, n [, c])` | `n` dots at random places in a rectangle. It uses `rnd()`'s numbers, so `seed_rnd()` repeats a spray |
| `tex_quad(x0, y0, x1, y1, x2, y2, x3, y3, spr, sw, sh)` | stretch the `sw` × `sh` block of tiles at tile `spr` over any four-cornered shape: top-left, top-right, bottom-right, bottom-left. The edges don't have to be parallel. Order the corners differently to flip or turn it |
| `tex_quad_3d(x0, y0, z0, ..., x3, y3, z3, spr, sw, sh)` | the same, with a depth at each corner: x and y go straight to the screen (orthographic), and z, blended across the quad, goes into the depth buffer, so quads hide each other correctly whatever order they are drawn in |

### Depth and blending

Every pixel has a depth (−32768 to 32767). **Lower numbers are nearer.** A pixel is drawn only where the depth already there is the same or further away, and then the depth there becomes its own. All drawing uses the drawing depth, except `tex_quad_3d`, which uses its corners' depths.

`cls()` pushes everything it clears to the far plane (32767), so anything drawn afterwards shows, whatever its depth. It also sets the drawing depth back to 0.

| Command | |
|---|---|
| `set_depth([z])` | draw at depth `z` from now on. `set_depth()` is 0 |
| `set_blend([b])` | how drawn colors mix with the screen, per red / green / blue channel (0–3): `BLEND_NORMAL` replaces, `BLEND_ADD` adds (up to 3), `BLEND_SUB` subtracts from the screen (down to 0), `BLEND_MAX` / `BLEND_MIN` keep the higher / lower, `BLEND_XOR` XORs. `set_blend()` is `BLEND_NORMAL`. `cls()` ignores it |

### The font

In the built-in font every character is 6 × 10 pixels: a letter 5 pixels wide, with a column of space to its right and a row above. Capitals are 7 pixels tall. Lowercase letters are 5, with ascenders (b d f h k l t) as tall as the capitals, and g j p q y hang two rows below the line, into the cell's bottom row.

This is the **built-in font**. The shell and the editors always use it. A cart's `text()` uses it too, until the cart has a font of its own: `fonts/font.g64`, made in the [font editor](#font-editor), or others loaded with `load_font`.

### Tiles and map

These use the tileset and map in use: the one swapped in last (slot 0 to start with).

| Command | |
|---|---|
| `sprite(n, x, y [, w, h, flags])` | draw tile `n` (0–959), or a w×h block of tiles from the sheet (40 per row). Flags: `FLIP_X`, `FLIP_Y` |
| `tile_pget(n, x, y)` / `tile_plot(n, x, y, c)` | read / write a tile pixel (−1 = transparent) |
| `map_draw([mx, my, sx, sy, w, h])` | draw map cells `mx,my` .. `+w,+h` at screen `sx,sy`. Empty cells are not drawn |
| `map_get(x, y)` / `map_set(x, y, t)` | read / change a map cell. An empty cell, or one outside the map, reads as −1; `map_set(x, y, -1)` empties a cell |
| `map_w()` / `map_h()` | the map's size in cells |

Changes a cart makes with `tile_plot` and `map_set` last until it stops. The next run starts from the cart's files again.

### Slots

Each kind of data has 10 slots (0–9). Load files into them, then swap between them. A level can bring its own map and tileset, and a boss fight its own song.

| Command | |
|---|---|
| `load_tileset(slot, "name")` | load `tiles/name.g64` into a slot |
| `load_tileset(slot)` | empty a slot, freeing its memory |
| `swap_tileset(slot)` | use that slot from now on. Returns the slot. `swap_tileset()` alone returns the slot in use |
| `load_map` / `swap_map` | the same for `maps/name.g64` |
| `load_sfx` / `swap_sfx` | the same for sound banks, `sfx/name.g64` (`sfx()` and the waves music plays with) |
| `load_music` / `swap_music` | the same for songs, `music/name.g64`. Swapping stops the song; `music()` starts the new one |
| `load_font` / `swap_font` | the same for fonts, `fonts/name.g64`: what `text()` and `text_width()` use. With the font slot in use empty, `text()` uses the built-in font |

When a cart starts, every slot holds the file the file tree put in it (a slotted file that's missing is reported in the shell and skipped). In a cart that hasn't set any slots, slot 0 of each kind holds the file with the default name (`tileset`, `map`, `sfx`, `music`, `font`). An empty slot 0 gets an empty file (for a font, the built-in one). Slot 0 is in use. A file open in an editor is loaded with its unsaved changes, so Ctrl+R runs what you see. Swapping to an empty slot is an error. Emptying the slot in use leaves nothing to draw: `map_get` reads −1, and `tile_plot` / `map_set` are errors.

```
load_map(1, "cave") : load_tileset(1, "cave-tiles")
swap_map(1) : swap_tileset(1)          ' into the cave
```

### The dot limit

GooberEngine draws in software, so each frame has a budget of pixels ("dots"). Every pixel a cart draws counts: shapes, text, sprites and maps. Pixels outside the clip rectangle, a tile's transparent pixels, `cls()` and `rect_fill_fast()` don't count (those two fill whole rows at once, so they're free). Past the limit, further pixels are dropped until the next `flip()`, so an over-budget frame shows up unfinished.

| Command | |
|---|---|
| `dot_limit([n])` | set the limit (0 to 1,000,000; 100,000 to start with). Returns the limit; `dot_limit()` alone reads it |
| `dots_used()` | pixels drawn since the last `flip()` |

The screen is 96,000 pixels, so filling it with a map or a full-screen `rect_fill` uses almost the whole budget. Draw only what changes: clear with `cls()` instead of drawing a background, use `rect_fill_fast()` for solid boxes like panels and bars, and use `clip()` to keep drawing to the part that needs it. [Seedling](carts/examples/seedling/main.gb) does both.

### Sound

| Command | |
|---|---|
| `sfx(n [, voice])` | play sound effect `n` (0–63) on a voice (0–7), or on any free voice. Returns the voice it used |
| `note(voice, pitch, wave, vol [, ticks])` | play one note: pitch 0–63 (24 = C-4, 33 = A-4 at 440 Hz), wave 0–7, `INSTRUMENT` + 0–7 or `NOISE`, volume 0–7. It lasts `ticks` (120 a second), or until stopped |
| `voice_stop([voice])` | stop one voice, or all of them |
| `voice_sfx(voice)` | the effect a voice is playing: −1 for nothing, −2 for a `note` |

| `music([n])` | play the song from order entry `n` (default 0) |
| `music_stop()` | stop the music |
| `music_pos()` / `music_row()` | the order entry and row playing, −1 when stopped |

`voice_sfx` gives −3 for a voice the music is playing. All sound and music stop when the cart does. Constants: `VOICES` (8), `INSTRUMENT` (8, the first instrument "wave") and `NOISE` (16, the noise "wave").

### Input and system

| Command | |
|---|---|
| `btn(b)` / `btn_hit(b)` | pad button held / pressed this frame: `BTN_LEFT` `BTN_RIGHT` `BTN_UP` `BTN_DOWN` `BTN_A` `BTN_B` `BTN_C` `BTN_START` |
| `key_down(k)` / `key_hit(k)` | raw keyboard, same `KEY_` constants as GooberBASIC |
| `mouse_x()` `mouse_y()` `mouse_down(b)` `mouse_hit(b)` | mouse in screen pixels; `MOUSE_LEFT` / `MOUSE_RIGHT` / `MOUSE_MIDDLE` |
| `mouse_btn(b)` | 1 while mouse button `b` is held: 0 left, 1 right, 2 middle |
| `mouse_wheel()` | wheel steps since the last `flip()`: positive scrolls down, negative up |
| `mouse_dx()` `mouse_dy()` | how far the mouse moved since the last `flip()`, in screen pixels |
| `flip()` | show the frame and wait for the next one (60 fps). The screen is **not** cleared |
| `frame_count()` | frames since boot |
| `cpu()` | how much of this frame's time (1/60 s) has been used so far, as a percentage. Call it just before `flip()` to see the whole frame; over 100 means frames are being dropped |
| `cart_title()` | the cart's title |

Constants: `SCREEN_W` `SCREEN_H` `TILE_SIZE` `SHEET_COLS` (40) `NTILES` (960) `SLOTS` (10) `MAP_SCREEN_W` (40) `MAP_SCREEN_H` (24) `FONT_W` (6) `FONT_H` (10). (`MAP_W` and `MAP_H` are gone: maps vary in size, so use `map_w()` and `map_h()`.) Named colors:

| Constant | Index | Constant | Index |
|---|---|---|---|
| `COL_BLACK` | 0 | `COL_ORANGE` | 56 |
| `COL_NAVY` | 1 | `COL_BROWN` | 36 |
| `COL_BLUE` | 3 | `COL_PINK` | 58 |
| `COL_DARK_GREEN` | 4 | `COL_PURPLE` | 35 |
| `COL_GREEN` | 12 | `COL_SKY` | 27 |
| `COL_CYAN` | 15 | `COL_LIME` | 44 |
| `COL_DARK_GREY` | 21 | `COL_MAGENTA` | 51 |
| `COL_GREY` | 42 | `COL_YELLOW` | 60 |
| `COL_RED` | 48 | `COL_WHITE` | 63 |

### Save data

| Command | |
|---|---|
| `save_data(key, value)` | keep a value for next time: a high score, settings, progress. An empty value forgets the key |
| `load_data(key [, default])` | what was saved, or `default` (or `""`) if nothing was |

- **Keys:** 1–64 letters, digits, `_` `-` `.`. A cart can keep 256 keys.
- **Values:** text up to 1024 characters. Numbers convert both ways on their own: `best = load_data("best", "0")`.
- **Where it lives:** each cart's data is a text file outside the cart, in `~/.local/share/gooberengine/saves/<cart>.txt` (or `$XDG_DATA_HOME`, or `$GOOBERENGINE_SAVES`). Playing never changes the cart, and `EXPORT` doesn't hand your scores to others.
- **Writing:** every `save_data` writes the file at once.

### Network

Up to 8 players share one game: one **hosts** (player 0) and the others **join** (players 1–7). They share an array of `NET_SLOTS` (256) numbers, and can send each other text messages. [carts/examples/net](carts/examples/net/main.gb) is a small one to start from.

| Command | |
|---|---|
| `net_host(port [, mode])` | start a game others can join; 1 if it worked (else see `net_error()`) |
| `net_join(address, port [, mode])` | join one: an IP address or a host name. 1 if it got started; then `net_status()` is `NET_CONNECTING` until the host answers |
| `net_status()` | `NET_OFF`, `NET_CONNECTING`, `NET_CONNECTED` (hosting counts) or `NET_LOST` (it failed, or the game ended: `net_error()` says why) |
| `net_error()` | why the last thing failed, e.g. `port 4040 is in use`, `the host ended the game` |
| `net_close()` | leave (a host ends the game for everyone) |
| `net_id()` | your player number: 0 for the host, 1–7 for a client, -1 when not in a game |
| `net_is_host()` | 1 if you are hosting |
| `net_peers()` / `net_peer(id)` | how many players are in the game, you included / 1 if player `id` is |
| `net_set(slot, value)` / `net_get(slot)` | change / read a shared number, slots 0–255. Values are floats, so ints come back exactly |
| `net_send(text [, to])` | a message (1–255 characters) to everyone (`NET_ALL`, the default) or to player `to`. 1 if it was queued |
| `net_messages()` / `net_recv()` / `net_from()` | messages waiting / the next one (`""` if none) / who sent the one `net_recv()` just gave you |
| `net_update()` | send and receive now. `flip()` and `delay()` already do this, so you rarely need it |

`mode` is `NET_TCP` (the default) or `NET_UDP`. Constants: `NET_SLOTS` (256), `NET_MAX_PLAYERS` (8), `NET_ALL` (-1).

- **The host owns the slots.** Anyone may `net_set` any slot. Your own copy changes at once. A client's change goes to the host, which applies it and passes it on to everyone. If two players write the same slot, whoever reaches the host last wins, so it is simplest to give each player slots of their own (`net_id() * 4` and up) and leave the shared ones, like a ball or the score, to the host.
- **Only changes travel.** A slot is sent when it changes and isn't resent once the other side has it, so setting every slot every frame costs nothing when the values don't change.
- **Messages** arrive in order, exactly once, over both TCP and UDP. They are for events: "I fired", "round over", chat.
- **TCP or UDP?** Both give the same result. TCP is simplest and works everywhere. Over UDP a lost packet doesn't hold up the ones behind it: the missing slots just go again in the next packet, so on a bad connection the game stays smooth. Host and players must pick the same one.
- **Timing:** packets go out at most once a frame, plus a keepalive five times a second. A player the host hasn't heard from for 5 seconds is dropped. Joining gives up after 5 seconds.
- **Where:** players on the same network join the host's local address (e.g. `192.168.1.20`). Over the internet the host has to forward the port on their router. `127.0.0.1` is the same machine: run two GooberEngines to try a game alone. IPv4 only.
- **When the cart stops,** its game is closed. In the browser there is no networking: `net_host` and `net_join` fail with `networking is not available in the browser`.
- **Testing a bad connection:** `GOOBER_NET_LOSS=30 ./gooberengine` drops 30% of the UDP packets the engine sends.

A cart that draws once and ends without calling `flip()` keeps its picture on screen until you press a key. `print` still writes to the terminal, which is handy for debugging.

## Example carts

The examples are in their own folder: `CD examples`, then `DIR`, `RUN depth` and so on (`CD ..` comes back).

| Cart | Shows |
|---|---|
| [carts/hello](carts/hello/main.gb) | the palette, shapes, text, pad input |
| [carts/examples/depth](carts/examples/depth/main.gb) | the depth buffer: circles and squares orbiting a planet, drawn in a shuffled order and still sorted by `set_depth`. A turns depth off to compare with plain draw order |
| [carts/examples/rotate](carts/examples/rotate/main.gb) | sprites of 1×1, 2×2 and 4×4 tiles turning around their centres with `tex_quad`, big and at their real size |
| [carts/examples/mouse](carts/examples/mouse/main.gb) | grab a bouncing ball and throw it: `mouse_dx` / `mouse_dy` become its speed, the right button pulls it to the pointer, the middle one changes its color and the wheel its size |
| [carts/examples/cube](carts/examples/cube/main.gb) | a dice cube (a 2×2-tile texture per face) spinning in perspective with `tex_quad_3d`: back-face culling, and a second cube cutting through the first |
| [carts/examples/net](carts/examples/net/main.gb) | a network game for up to 8: host or join (TCP or UDP), move a square around while the host bounces a ball everyone sees, B waves. Two GooberEngines on one machine can play with `127.0.0.1` |
| [carts/examples/seedling](carts/examples/seedling/main.gb) | a small game with a tileset, map, sound bank and song. Collisions with `map_get`, `map_set`, sound effects over a looping tune, sprite flipping, a following camera, a HUD, and staying inside the dot limit |

## Sharing carts

A cart is shared as one `.zip`. `EXPORT` zips the loaded cart folder, as compressing the folder in a file manager would: `rocket.zip` holds `rocket/main.gb`, `rocket/cart.txt`, `rocket/tiles/tileset.g64` and so on. `LOAD rocket.zip` (or `IMPORT`, a drop on the window, or `gooberengine rocket.zip`) unpacks it back into a cart folder. `.tar.gz` (`.tgz`) and plain `.tar` work too, and GooberEngine reads and writes the compression itself, so no library is needed.

- **EXPORT [file]:** with no file, `carts/<name>.zip`. A name without `.zip` gets it (`EXPORT ~/rocket` writes `~/rocket.zip`), and `EXPORT rocket.tar.gz` writes a gzipped tar instead. SAVE first: it zips the files on disk. The zip is deflate-compressed, and the same cart always gives the same bytes (dates are fixed).
- **What's in it:** the cart's `.gb` and `.txt` files, and the `.g64` files in `tiles/`, `maps/`, `sfx/`, `music/` and `fonts/`, plus any old `.txt` sound or music not yet converted. Not `*.unsaved` or `*.old` leftovers, or save data.
- **Loading unpacks it.** `LOAD rocket` (or `LOAD rocket.zip`) unpacks `rocket.zip` into a `rocket/` cart folder beside it. From then on the folder is the cart: the editors and `SAVE` work on the folder, and the zip is left as it was. If `rocket/` is there already, `LOAD rocket.zip` just loads it. `IMPORT rocket.zip othername` unpacks a second copy, and won't overwrite a cart that exists. A browser's `rocket (1).zip` still becomes `rocket`.
- **Where the cart is in it:** in one folder (`rocket/main.gb` …), as `EXPORT` writes it, or at the top (`main.gb` …). Either way it comes from any zip or tar tool.
- **What comes out:** only the cart's own files. Anything else in the archive is skipped, along with any name that would leave the cart folder (`../x.gb`, `/x.gb`). Encrypted zips, ZIP64 and compressions other than deflate are refused. So is a damaged or cut-short archive (CRCs are checked), and nothing is left behind.
- **The stub cache:** `DIR` and `BROWSE` show a zip's title, version and author without unpacking it. They read its `cart.txt` once and keep it in `cartcache.txt`, beside the save-data folder (or `$GOOBERENGINE_CACHE`). The entry is used while the zip's size and time stay the same, and read again when they change. A zip whose folder is unpacked is listed by its folder only.
- **`.gbcart`**, the old one-file text bundle, is gone. To bring one over, unpack it with an older GooberEngine and `EXPORT` it again. `.7z` isn't read yet.

## GooberEngine in the browser

`make web` builds the whole engine (shell, editors and all) with Emscripten into `build/web`. It doesn't serve it: the files work from any static web server, for instance `python3 -m http.server 8000 --bind 127.0.0.1 -d build/web`. `make web` zips `runtimes/` into `runtimes.zip` beside them, so `EXPORT` works in the browser too.

- **Your carts stay in the browser.** They live in its IndexedDB storage, saved every couple of seconds and when you leave the page. The example carts are copied in on the first visit; a new example in a later build appears, and your own carts are never overwritten.
- **Moving carts in and out:** drop a cart `.zip` on the page, use the **open cart** button, or type `UPLOAD`. `DOWNLOAD` (or `EXPORT`) saves the loaded cart to your computer. `EXPORT LINUX / WINDOWS / WEB` downloads a finished game; the web one comes as a `.zip`.
- **Sound** starts on your first click or key press: browsers keep pages quiet until then.
- **Game controllers** work once you press a button on the pad (the browser only shows pads after that). They're mapped the same as on the desktop.
- `QUIT` stops the engine; reload the page to start again. Browser shortcuts like Ctrl+W and Ctrl+T still go to the browser.

## Exporting games

`EXPORT LINUX`, `EXPORT WINDOWS`, `EXPORT WEB` or `EXPORT ALL` in the shell (or `gooberengine --export all mycart` on the command line) turns the loaded cart into a game that runs without GooberEngine. It boots straight into the cart: no shell, no editors, and Esc doesn't stop it. When a cart ends, it starts again. If it stops with an error, the error stays on screen.

| Target | You get | Runs on |
|---|---|---|
| `LINUX` | `exports/<name>/<name>-linux-x86_64`, one file | 64-bit Linux with glibc 2.38 or newer (Ubuntu 24.04, Debian 13, Fedora 39, Mint 22, current Arch). Nothing to install: SDL is inside, and it uses whichever of X11, Wayland, PulseAudio, PipeWire or ALSA the machine has |
| `WINDOWS` | `exports/<name>/<name>-windows.exe`, one file | 64-bit Windows 10 and 11. Only Windows' own DLLs are used |
| `WEB` | `exports/<name>/<name>-web/` (`index.html`, `player.js`, `player.wasm`) and the same three files as `<name>-web.zip` | any modern browser, from any static host (itch.io, GitHub Pages, your own server). Not from a `file://` page: browsers won't load the `.wasm` that way |

**What's inside.** An exported game is a prebuilt **player runtime** with the cart packed in. The player is GooberEngine built without the shell and editors. Nothing is compiled when you export, so it's quick and works the same from Linux, from the web version, or from anywhere `runtimes/` has been copied to.

- The cart travels zipped, exactly as `EXPORT` writes it: its source and data files. The player compiles `main.gb` as it starts, which takes milliseconds. So the code of an exported game can be read by anyone who looks for it, just as with the `.zip`.
- **Linux and Windows:** the `.zip` is appended to the player executable, followed by an 80-byte trailer: the cart name (64 bytes), the `.zip` length (8 bytes, little-endian) and the magic `GOOBERZP`. A program ignores bytes after its own end, so the result is an ordinary executable. At start the player reads its own file, unpacks the cart into a temporary folder, and removes it on exit.
- **Web:** the `.zip` is embedded, base64, in a `<script>` tag in `index.html`, next to the unchanged `player.js` and `player.wasm`.
- **Save data** belongs to the player's machine: `~/.local/share/gooberengine/saves/` on Linux, `%APPDATA%\GooberEngine\saves\` on Windows, and the browser's storage on the web (per site).

**Things to know:**

- Linux browsers' downloads lose the "executable" mark: `chmod +x mygame-linux-x86_64` once.
- The Windows `.exe` isn't code-signed, so Windows SmartScreen asks "run anyway?" the first time.
- A player run with `--headless --frames N --shot file.png` draws N frames without a window, which is what the tests use. `--cart file.zip` plays a cart `.zip` with a bare runtime.

**Where the runtimes come from.** `make runtimes` builds them into `runtimes/`:

| Runtime | Built with |
|---|---|
| `player-linux-x86_64` | the system compiler, with SDL2 built from source as a static library whose back ends are loaded at run time. Only glibc is linked dynamically. Because it's built against this machine's glibc, it needs glibc 2.38 or newer; building it on an older distribution reaches older systems |
| `player-windows-x86_64.exe` | `x86_64-w64-mingw32-gcc`, with SDL2 cross-compiled and linked in statically |
| `web/player.js`, `player.wasm`, `index.html` | Emscripten's SDL2 port, with `-sASYNCIFY` so the frame loop can hand control back to the browser |

GooberEngine finds the runtimes in `$GOOBERENGINE_RUNTIMES`, then in `runtimes/` next to the executable, then in `./runtimes`. The web version fetches them as one `runtimes.zip` from its own server, the first time it exports.

**Your game is yours.** You can sell an exported game, or give it away, under any license you like. You don't have to publish its code or GooberEngine's source with it, as long as you leave the player runtime unmodified. See [License](#license).

## Milestone plan

1. **Console core** ✅
   - SDL window with an integer-scaled 400×240 framebuffer and a fixed 60 fps loop.
   - A boot shell (DIR / LOAD / RUN / INFO).
   - Folder carts with `tiles.txt` and `map.txt`.
   - The drawing, tile, map and input API.
   - Compile and runtime errors reported in the shell.
   - Two example carts.
   - A headless test runner.
2. **Code editor** ✅
   - A full-screen GooberBASIC editor inside the console: syntax colors from the compiler's own tables, scrolling, mouse and keyboard selection, and the system clipboard.
   - Undo/redo, find, go to line, comment toggling, and block indenting.
   - Ctrl+R to save and run, and jump-to-line on errors.
   - `NEW name` creates a cart and `SAVE` writes `main.gb`.
   - Scripted-input tests (`--type`).
   - Still to do: editing the `include`d files, find-and-replace, a scrollbar.
3. **Tile editor** ✅
   - A pixel editor for 1×1, 2×2 or 4×4 tile blocks, with a 64-color palette (A/B colors) and the whole sheet beside it.
   - Pencil, fill, line, rectangle, color pick, flip/turn, nudge, copy/paste of blocks, and 64-step undo.
   - Writes `tiles.txt`, keeping its legend and comments.
   - Each run of a cart now starts from the cart's own tiles and map.
   - F1/F2 tabs switch between the editors.
   - Still to do: a "used by the map" marker, swapping two tiles everywhere, and an animation preview.
4. **Map editor** ✅
   - Paint the 160×96 map with tiles or block brushes (from the picker or copied from the map).
   - Pen, rectangle, fill, select, and right-click pick.
   - Four zoom levels down to a whole-map overview, panning, screen-sized guides, and 64-step undo.
   - Writes `map.txt`, keeping its comments.
   - Still to do: map layers or flags (e.g. "solid"), and a "show where this tile is used" view.
5. **Sound** ✅
   - An 8-voice wavetable synth (`src/synth.c`, no SDL inside) fed to the SDL audio device.
   - 8 drawable waveforms, 8 instruments (sound effects 0–7 transposed to the note), and pitched noise, with click-free volume changes.
   - 64 sound effects of up to 32 steps. Each step has its own effect: slide, vibrato, drop, fades, arpeggio. Effects can loop.
   - A sound editor with musical-keyboard entry, and `sfx.txt` that keeps comments and saves unchanged files byte-for-byte.
   - API: `sfx`, `note`, `voice_stop`, `voice_sfx`.
   - Headless `--wav` recording, so tests measure the pitch and timing of real output.
   - Seedling now has sound.
   - Still to do: stereo/panning, per-voice volume, and sound changes from a running cart.
6. **Music** ✅
   - A tracker on the same voices: 32 patterns of 32 rows × 8 tracks, and a 64-entry order list with jumps and stops.
   - Sound effects take priority over the music on a shared voice.
   - Notes carry on through empty rows, with their vibrato and arpeggio still going.
   - Follow-play, single-pattern loop, and 32-step undo.
   - `music.txt` saves unchanged songs byte-for-byte.
   - API: `music`, `music_stop`, `music_pos`, `music_row`.
   - Seedling has a looping tune.
   - Still to do: per-pattern lengths, transposing a selection, and a music volume control.
7. **Polish** ✅
   - Palette swap for drawing (`pal(a, b)`).
   - Per-cart save data (`save_data` / `load_data`), kept outside the cart.
   - A `BROWSE` list to pick carts from.
   - One-file `.gbcart` `EXPORT` / `IMPORT` (since replaced by zipped carts).
   - VS Code highlighting for the console API: `make vscode`, which uses a new `--with` option in GooberBASIC's grammar generator.

8. **Web and export** ✅
   - The whole engine in the browser (`make web`): Emscripten with ASYNCIFY, carts kept in IndexedDB, upload, download and drag-and-drop of `.gbcart` files, sound unlocked on the first gesture, and game controllers through SDL.
   - `EXPORT LINUX / WINDOWS / WEB / ALL` and `--export`: prebuilt player runtimes with the cart packed in (appended plus a trailer for executables, embedded in `index.html` for the web).
   - `make runtimes`: SDL2 built from source for a self-contained Linux binary and a static mingw-w64 `.exe`.
   - The engine writes its own indexed-color PNGs, so SDL2_image is no longer used. Dropping a cart on the desktop window loads it.
   - Still to do: a Linux runtime built against an older glibc for older distributions; macOS; a game icon and version info in the `.exe`; touch controls for phones.
9. **Many files** ✅
   - Tilesets of 960 tiles (400 × 240) and maps of 1–10 screens each way, in compact base-64 `.g64` files under `tiles/` and `maps/`. Sound banks and songs live under `sfx/` and `music/`.
   - 10 run-time slots per kind: `load_tileset` / `swap_tileset`, `load_map` / `swap_map`, `load_sfx` / `swap_sfx`, `load_music` / `swap_music`.
   - The dot limit: `dot_limit()` and `dots_used()`, 100,000 pixels a frame by default.
   - A file tree beside every editor, several `.gb` files in the code editor, and a tile editor with a pannable, zoomable tileset and floating tile and tools windows.
   - Old carts' `tiles.txt` / `map.txt` / `sfx.txt` / `music.txt` are read and converted by the next `SAVE`.
   - Right-click in the tree to rename or delete a file.
   - A settings screen (F9, or the button at the bottom of the file tree): theme, overall / music / sound-effect volume, fullscreen, window size and the code editor's indent mode, kept in `settings.txt`.
   - Fonts are a resource like the rest (`fonts/NAME.g64`, `load_font` / `swap_font`), with a font editor. A font's characters can be 4 × 4 to 20 × 20, spacing included; `font_w()` / `font_h()` tell a cart the size. The built-in font is every cart's default.
   - The tile editor paints straight on the tileset, inside a selection box that keeps strokes off the tiles around it. The tile window became a preview of the selection.
   - Sound banks and songs are `.g64` too (`sfx/NAME.g64`, `music/NAME.g64`). Their old text files are read and converted by the next `SAVE`.
   - Still to do: the web player runtime (`runtimes/web`) needs rebuilding with Emscripten to play carts in the new layout.

### What could come next

- Polish left over from earlier milestones: listed under each milestone above.
- Find-and-replace.
- A display palette (`pal` at show time) for fades.
- Stereo and per-voice volume.
- Interrupting a cart stuck in a loop with no `flip()`. This needs a small hook in the GooberBASIC VM.

## Source map

| File | What |
|---|---|
| `src/engine.h` | machine constants and the shared state |
| `src/compat.h` | the few file calls that differ on Windows and in the browser |
| `src/platform.c` | window, timing, input, sound device, screenshots (PNG) |
| `src/video.c` | framebuffer drawing: shapes, text, tiles, map, the dot limit |
| `src/font.c` | the system font |
| `src/cart.c` | folder cart loading, the sfx / music text formats and the old file formats, compile + run on the GooberBASIC VM |
| `src/res.c` | resource files (.g64 tilesets and maps), the files open in the editors, the run-time slots |
| `src/tree.c` | the file tree, and running each editor beside it |
| `src/api.c` | the console API native table |
| `src/shell.c` | the boot shell, the editors' tabs, and switching between shell, editors and running cart |
| `src/editor.c` | the code editor |
| `src/help.c`, `src/help.txt` | the manual (F1): the viewer, and its pages (`make` turns `help.txt` into `src/help_pages.c`) |
| `src/tiles_editor.c` | the tile editor |
| `src/map_editor.c` | the map editor |
| `src/synth.c` | the sound chip: voices, waves, effects, mixer |
| `src/sound_editor.c` | the sound editor |
| `src/music_editor.c` | the music tracker |
| `src/save.c` | each cart's save data |
| `src/archive.c` | zipped carts: `EXPORT`, `.zip` / `.tar.gz` / `.tar` reading and writing, unpacking on LOAD, the stub cache |
| `src/zlib.c` | deflate and gzip, both ways, without a library |
| `src/browser.c` | the `BROWSE` cart picker |
| `src/export.c` | game export (packing carts into runtimes, the .zip writer) and the player's start-up |
| `src/web.c` | the browser build's downloads, uploads and dropped files |
| `src/main.c` | command line |
| `web/engine.html`, `web/player.html` | the pages around the web engine and around an exported web game |
| `third_party/SDL2-2.32.10.zip` | SDL2's source, for building the runtimes |

## License

GooberEngine is free software, under the [GNU General Public License, version 3](LICENSE), with the [GooberEngine Game Exception](GAME-EXCEPTION.txt). GooberBASIC is under the same terms.

- **The engine stays free.** If you share GooberEngine or a modified version of it, you must do so under the GPLv3, with its source.
- **Your carts and games are yours.** A cart isn't covered by the GPLv3 just because it's written in GooberBASIC or runs on GooberEngine. License it however you want. An exported game can be sold or shared under any terms, without its source or the engine's, as long as the player runtime inside it is unmodified.
- **The example carts** in `carts/` can be copied into your own carts freely.

SDL2, which is built into the player runtimes, is under the [zlib license](https://www.libsdl.org/license.php).
