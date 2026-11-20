# GooberEngine

An all-in-one fantasy console whose programming language is **GooberBASIC**.
It's a fixed virtual machine with a 400×240 screen, 64 colours and 10×10 tiles.
It boots into a shell and runs *folder cartridges*. The editors for code, tiles,
maps and sound are built into the console. You can still use your own text
editor on a cart folder at any time.

```
./gooberengine              boot into the shell
./gooberengine seedling     boot straight into a cart (Esc returns to the shell)
```

## The machine

| Part | Spec |
|---|---|
| Screen | 400 × 240 pixels, 60 frames per second, scaled ×3 in a resizable window (F11 = fullscreen) |
| Colour | 64 fixed colours: 6-bit RGB, 2 bits per channel. Index = `r*16 + g*4 + b`, each channel 0–3 |
| Tiles | a tileset is a 400 × 240 sheet: 960 tiles of 10 × 10 pixels, 40 per row. A pixel is a colour or transparent |
| Map | 1 to 10 screens each way (a screen is 40 × 24 cells), so up to 400 × 240 cells. A cell holds a tile number 0–959 or nothing; tile 0 is a tile like any other |
| Slots | 10 slots each for tilesets, maps, sound banks and songs. A cart loads files into slots and swaps between them while it runs |
| Dots | a cart may draw 100,000 pixels a frame (`dot_limit` changes it, up to 1,000,000). The screen alone is 96,000, so redrawing only what changed pays off |
| Text | built-in 6×10 font, letters 5 wide: 66 × 24 characters. A cart can bring fonts of its own, 4×4 to 20×20. Capitals and lowercase letters, with descenders (see [The font](#the-font)) |
| Input | a pad with a d-pad, three buttons **A B C** and **Start** (keyboard or game controller), plus raw keyboard and mouse |
| Sound | 8 voices. Each plays one of 16 drawable waveforms (32 samples each) or noise, at 64 pitches (C-2 to D#7) and 8 volumes. There are 64 sound effects |

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
| `make web` | the whole engine for the browser, in `build/web`, then serves it at http://localhost:8000 (`make web-build` only builds) | Emscripten |

Emscripten comes from [emsdk](https://emscripten.org/docs/getting_started/downloads.html). The Makefile uses the copy in `tools/emsdk` (set it up once with `tools/emsdk/emsdk install latest && tools/emsdk/emsdk activate latest`); pass `EMSDK=/path/to/emsdk` for another place. Windows builds use the `mingw-w64` cross compiler (`sudo apt install mingw-w64`). SDL2 for the runtimes is built from `third_party/SDL2-2.32.10.zip` the first time, which takes a minute or two.

`make vscode` regenerates GooberBASIC's VS Code grammar (in `../gooberbasic/editors/vscode`) with the console API added. Commands like `rect_fill` and `music`, and constants like `BTN_A` and `COL_NAVY`, are then highlighted in `.gb` files. This rewrites the shared extension's grammar, so engine names light up in plain GooberBASIC programs too. Running GooberBASIC's own `make vscode` puts the plain grammar back.

### How it reuses GooberBASIC

GooberEngine is not a fork of the language. The Makefile compiles GooberBASIC's `lexer.c`, `compiler.c`, `vm.c`, `objects.c` and `builtins.c` straight from `../gooberbasic/src`. A fix to the language therefore reaches both programs.

GooberBASIC finds native commands in two tables:

- `gb_core_natives`: print, maths, strings, lists and so on.
- `gb_gfx_natives` / `gb_gfx_consts`: the standalone `goober` binary gets these from its SDL layer, `gfx.c`.

GooberEngine does **not** link `gfx.c` or `main.c`. It supplies [src/api.c](src/api.c) as the graphics table instead. So a cart sees the core language plus the console hardware, and nothing else: no windows, image files or TTF fonts.

One change was made to the shared code. It adds a small embedding hook in `common.h` / `objects.c`:

- `gb_diag_hook` redirects diagnostics.
- `gb_error_jmp` lets lexer and compiler errors `longjmp` back to the host instead of calling `exit(1)`.

Both are NULL by default, so `goober` behaves exactly as before. This is how a cart with a syntax error drops you back at the shell prompt, error shown in red, instead of killing the console.

For the code editor, GooberBASIC also exports two read-only lookups: `lex_is_keyword()` in `lexer.c` and `compiler_builtin_word()` in `compiler.c`. With `native_find()` and the console's constant table, these give the editor its syntax colours straight from the language's own tables, so they can't drift apart.

## Shell

| Command | What it does |
|---|---|
| `DIR` | list the carts in the folder you're in, and the folders of carts under it (`examples/`) |
| `CD folder` | go into a folder of carts inside the carts folder: `CD examples`. `CD ..` goes back up, `CD` alone (or `CD /`) to the top. `DIR`, `LOAD`, `RUN`, `NEW`, `SAVE` and `BROWSE` then work in that folder, and the prompt shows its name |
| `LOAD name` | load a cart by name, or by path to its folder |
| `RUN [name]` | save any code being edited, then run the loaded cart (or load + run) |
| `EDIT` | open the code editor (F1, or Esc on an empty prompt) |
| `TILES` | open the tile editor (F2) |
| `MAP` | open the map editor (F3) |
| `SOUND` | open the sound editor (F4) |
| `MUSIC` | open the music editor (F5) |
| `FONT` | open the font editor (F7) |
| `NEW [name]` | make a cart folder with a starter `main.gb` and open it in the editor. Without a name: a fresh untitled cart |
| `SAVE [name]` | save everything being edited: code, tiles, map, sound. On an untitled cart, `SAVE mygame` makes `carts/mygame`, and `SAVE` alone makes `carts/untitled` (asking first if that name is taken) |
| `BROWSE` | pick a cart from a list: Up/Down, PgUp/PgDn, Home/End or the mouse (the wheel scrolls), Enter (or a second click) runs, L loads |
| `EXPORT [file]` | write the loaded cart as one `.gbcart` file (default `carts/<name>.gbcart`). SAVE first |
| `EXPORT target [folder]` | make a standalone game: `LINUX`, `WINDOWS`, `WEB` or `ALL` (several may be given). Written to `exports/<name>/` unless a folder is given. See [Exporting games](#exporting-games) |
| `UPLOAD` / `DOWNLOAD` | web version only: open a `.gbcart` from your computer / save the cart to it |
| `IMPORT file [name]` | unpack a `.gbcart` into a new cart folder and load it. `LOAD file.gbcart` does the same, named after the file |
| `INFO` | title, author, tiles used, folder |
| `THEME [name]` | the editors' colours: `blue` (the default), `green`, `orange` or `grey` (`white` works too). Remembered for next time. `THEME` alone shows the one in use |
| `SETTINGS` | open the [settings](#settings) screen (F9): theme, volume, fullscreen, window size, indent |
| `FOLDER` | where carts are read from |
| `CLS` | clear the screen |
| `QUIT` | leave |

GooberEngine boots into a new **untitled cart**, so you can start typing code (F1) right away. Nothing is written to the carts folder until you save it with `SAVE name` (or `SAVE` alone, which names it `untitled`). Until then Ctrl+S in the editors only reminds you to, `RUN` works as usual, and `EXPORT` asks for a name first. Closing the window with unsaved untitled work keeps it in a scratch folder in `$TMPDIR` and prints where.

**Themes** recolour the shell, the editors and the file tree: their bars, highlights and backgrounds. Carts, the tileset and map you're editing, and the code editor's syntax colours look the same in every theme. Pick one with `THEME` or on the [settings](#settings) screen.

Up/Down recalls history. If anything being edited has unsaved changes, `LOAD`, `NEW`, `RUN name` and `QUIT` ask `discard them? (Y/N)` first. Press Y to discard the changes and go ahead, or N (or Esc) to keep them. If the window is closed with unsaved work, it is kept next to each file as `NAME.unsaved` (`main.gb.unsaved`, `tiles/tileset.g64.unsaved` …).

F1–F5 and F7 switch between the code, tile, map, sound, music and font editors from anywhere, including the shell. You can also click the tabs at the top of any editor.

## Settings

F9, `SETTINGS` in the shell, or the **settings** button at the bottom left of the file tree opens the settings screen. It works with or without a cart loaded. Up / Down pick a line, and Left / Right (or the mouse) change it. Enter turns fullscreen on or off. Esc, or F9 again, goes back to where you were.

| Setting | |
|---|---|
| theme | the shell's and editors' colours: blue, green, orange or grey (click one) |
| volume | all sound, 0–10. Changing it plays a short note at the new level |
| music | the music, 0–10, on top of the volume |
| sound fx | sound effects and `note()`s, 0–10, on top of the volume |
| fullscreen | on or off (F11, or Alt+Enter, does the same) |
| window size | 1× to 8× (`--scale` on the command line wins for that run) |
| indent | how Enter indents in the code editor: **smart** (keep the indent, and add one after `if`, `for` …), **auto** (just keep the indent; Backspace straight after Enter takes the whole newline back), **none** |

Every change takes effect at once and is kept for next time in `settings.txt`, beside the save-data folder. That's `~/.local/share/gooberengine/settings.txt`, `%APPDATA%\GooberEngine\settings.txt` on Windows, the browser's storage on the web, or `$GOOBERENGINE_SETTINGS`. The file is `key = value` lines (`theme`, `volume`, `music_volume`, `sfx_volume`, `fullscreen`, `scale`), and other lines in it are left alone. Exported games don't read it: the settings belong to the console, not to the games it makes.

## The file tree

Every editor has the cart's files down its left side. The editor fills the rest of the screen.

```
files     <        < hides the tree (so does F6, or the button left of the tabs); right-click a file: rename, delete
v code    +        + makes a new file: code-1, tiles-1, map-1, sfx-1, music-1, font-1 ...
 main
 enemies
v tiles   +        click a file to open it in its editor; click a section's name to fold it
 tileset
 tiles-2*          * = changed, not saved
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

Dropping a `.gbcart` file or a cart folder on the window loads it, like `LOAD`.

## Code editor

`EDIT` opens the loaded cart's `main.gb`, with line numbers and syntax colours. Other `.gb` files of the cart (the ones `main.gb` includes, for example) open from the file tree. Each keeps its own cursor and undo history, and the bottom bar shows which file you're in. Each file keeps its top-level `local`s and its functions to itself: write `global function` (or `global` for a variable) to use it from another file. See GooberBASIC's README, under Files.
The colours mark keywords, console and core commands, type names, constants, numbers, strings and comments.

| Keys | |
|---|---|
| **Ctrl+R** | save and run. Esc in the cart comes back here. On an error, the editor jumps to the line, marks it red and shows the message |
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

`TILES` (or F2) edits a tileset: 400 × 240 pixels, 960 tiles. That's more than fits beside the file tree, so the editor works like a paint program:

- **The tileset** is where you paint. Middle-drag (or Space + drag) pans it, and the wheel zooms it (1× to 16×, starting at 6×) around the mouse.
- **The selection:** a yellow box marks the tile being edited, or a 2×2 or 4×4 block of tiles with **Z**, for sprites bigger than one tile. Only the selection can be painted: a stroke, line, rectangle or fill stops at its edge, so the tiles around it are safe. Click outside it to select another tile. The lines between tiles aren't drawn across it, so a block reads as one picture; at 6× and above a faint pixel grid shows inside it instead.
- **The preview window** shows the selection by itself, at the biggest size that fits. It's only for looking. Drag it by its title bar, and drag its bottom-right corner to make it bigger or smaller.
- **The tools window** holds the tools (P F L R I), the block actions (Z H V T X), the 64 colours, transparent, and colours A and B. Drag it by its title bar too. The bottom bar names what's under the mouse.
- **Transparent** pixels show as a pink checkerboard (colours 50 and 34).
- **Mouse:** in the selection the left button draws with colour **A** and the right button with colour **B**. B starts as transparent, so the right button erases. Clicking a colour sets A (left) or B (right). A middle click (without dragging) picks a colour up into A, from anywhere on the tileset, and so does the pick tool (I): left into A, right into B.
- **Running:** edits go straight into the open tileset, so Ctrl+R runs them without saving. A running cart's own `tile_pset` / `map_set` changes are undone when it stops, so every run starts from the edited files.

| Keys | |
|---|---|
| P / F / L / R / I | pencil, flood fill, line, rectangle (hold Shift for a filled one), pick a colour |
| Z | edit size: 1×1, 2×2 or 4×4 tiles |
| H / V / T | flip across, flip up-down, turn a quarter clockwise (the whole block) |
| Del | clear the block |
| arrows | move the selection to another tile (the view follows it) |
| Shift+arrows | nudge the pixels one step (they wrap around) |
| = / -, mouse wheel | zoom the tileset in / out |
| Home | zoom, view and windows back to where they started |
| X | swap colours A and B |
| G | grid on/off |
| Ctrl+C / X / V | copy, cut, paste the block |
| Ctrl+Z / Ctrl+Y | undo / redo (64 steps) |
| Ctrl+S, Ctrl+R, Esc | save `tiles/NAME.g64`, run, back to the shell |

Saving writes only as much of the sheet as is used, so a tileset with ten tiles in its top row is a small file.

## Map editor

`MAP` (or F3) edits a map. The view fills the editor.

- **Painting:** the left button paints with the brush and the right button picks up the tile under the mouse.
- **The brush:** one tile, or a block of tiles. **Tab** opens the tile picker, which shows the whole tileset (scroll it with the wheel, Shift+wheel, arrows or a middle drag). Click a tile, or drag across several for a block, such as a 2×2 tree. The select tool turns part of the map into a brush with Ctrl+C. **E** makes the brush the eraser, which leaves cells empty. Tile 0 is an ordinary tile.
- **Blocks:** a block brush is laid in step with where you started, so dragging tiles it neatly without overlapping. Rectangles and fills repeat it as a pattern.
- **Size:** a map is 1–10 screens wide and 1–10 high. Ctrl+→ / Ctrl+← add or remove a column of screens on the right, and Ctrl+↓ / Ctrl+↑ add or remove a row at the bottom. A new map is 4 × 4 screens.
- **Tiles:** the map is drawn with the tileset the tile editor has open. **T** switches to the next tileset.
- **Zoom:** 20, 10, 5, 2 or 1 pixels per cell. Below 5, each tile is drawn as its most common colour.
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

`SOUND` (or F4) edits a sound bank: 64 sound effects and the 16 waveforms they play.

- **Layout:**
  - On the left is the list of effects; white ones have notes.
  - Across the top are the speed, the length and the loop, plus a play button.
  - Then, for each of the 32 steps: its pitch (a bar), its volume, its wave (`0`–`f`, `n` = noise) and its effect.
  - At the bottom is the wave being drawn, all 16 waves, and the effect pens.
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

Beside the file tree there's no room for the list of effects. PgUp / PgDn switch effects, and the bottom bar shows which one is open. Hide the tree (F6) to get the list back.

## Music editor

`MUSIC` (or F5) is a tracker for a song. Beside the file tree, six of the eight tracks fit. The view scrolls to follow the cursor, and `<` / `>` show that more tracks are there.

- **The song:** an order list of up to 64 entries along the top. Each entry plays a pattern, jumps to another entry (`>N`, so a song can loop) or stops (`.`). `music(n)` starts playing at entry `n`, so one cart can hold several tunes separated by stops.
- **A pattern:** 32 rows × 8 tracks. A cell is a note, a wave, a volume and an effect, the same as a sound-effect step. `...` means nothing new: the last note carries on. `===` turns the note off.
- **Voices:** track *t* plays on voice *t*. A sound effect started on a voice the music is using takes over that voice, and the track picks up again at its next note after the effect ends. `sfx()` with no voice chooses a free voice first, then one of the music's.
- **Tempo:** set with the song speed, in ticks per row (120 ticks a second).
- **The cursor:** it sits on one field of a cell: note, wave, volume or effect. On the note field, the musical-keyboard keys enter notes, as in the sound editor. On the other fields, type the value. The cursor then moves down a row.
- **Playing:** Space plays the song from the selected entry, and the screen follows it as it plays. Shift+Space loops the pattern on screen.

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
| Ctrl+C / X / V | copy / cut / paste the pattern |
| Ctrl+Z / Ctrl+Y, Ctrl+S, Ctrl+R | undo / redo, save `music/NAME.g64`, run |

## Font editor

`FONT` (or F7) edits a font: the 96 glyphs, for characters 32–127, that a cart's `text()` draws with. A cart that has no font yet starts with a copy of the built-in one, so you change only the glyphs you want. Saving writes `fonts/NAME.g64`.

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

A cart can have any number of each. The files named `tileset`, `map`, `sfx`, `music` and `font` are the defaults: each is in slot 0 when the cart starts (see [Slots](#slots)). Every file is text, so carts can be diffed and edited outside the console.

**The .g64 files** are base-64 text. Each character of `0-9 a-z A-Z + *` stands for a number 0–63, in that order: `0` is 0, `a` is 10, `A` is 36, `*` is 63. A colour is its palette index, so `a` is colour 10 everywhere. Spaces and line breaks are ignored. GooberEngine writes one row per line.

**tiles/NAME.g64**: `XY` then the pixels, row by row. `X` and `Y` give the width and height as (digit + 1) × 10 pixels, so `00` is 10 × 10 and `jn` is 200 × 240. A tileset holds at most 400 × 240 (`dn`). A pixel is a colour character, or `.` for transparent. Tile *n* is at column *n* mod 40, row *n* / 40 of the sheet.

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

**sfx/NAME.g64**: `WS`, then the waves, then the sound effects. `W` and `S` give the number of waves and of sound effects as digit + 1. GooberEngine writes all 16 waves (`f`), and sound effects up to the last one that isn't empty. Waves and sound effects that aren't in the file are the defaults.

- **A wave** is 32 samples `0`–`f`, with 7–8 the middle. GooberEngine writes one wave per line.
- **A sound effect** is one line: speed − 1, length − 1, loop start, loop end, then all 32 steps. A step is four characters: the pitch (0–63: `0` is C-2, `c` C-3, `o` C-4, `A` C-5, `M` C-6), the wave (`0`–`f`, or `g` for noise), the volume 0–7, and the effect (`0` none, `1` slide, `2` vibrato, `3` drop, `4` fade in, `5` fade out, `6` arpeggio). A rest is volume 0, so an empty step reads `o000`.

```
f2                                     16 waves, 3 sound effects
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

These are added on top of the GooberBASIC core: `print`, maths, strings, lists, `rnd`, `millisecs` and the rest.
Arguments in `[brackets]` are optional.
A colour is a palette index 0–63. If you leave a colour out, the pen colour is used.
All drawing goes through `camera` and `clip`.

### Screen

| Command | |
|---|---|
| `cls([c])` | clear the screen, or the clip rectangle if one is set (default colour 0) |
| `pen(c)` | set the default colour |
| `pset(x, y [, c])` / `pget(x, y)` | write / read one pixel |
| `line(x1, y1, x2, y2 [, c])` | draw a line |
| `rect(x, y, w, h [, c])` / `rect_fill(...)` | outline / filled rectangle |
| `circle(x, y, r [, c])` / `circle_fill(...)` | outline / filled circle |
| `text(s, x, y [, c])` | draw text (`\n` breaks lines). Returns the x after the text |
| `text_width(s)` | width of a string in pixels, in the font in use (6 per character in the built-in one) |
| `font_w()` / `font_h()` | the size of a character of the font in use: `text` steps this far across, and down at a line break |
| `rgb(r, g, b)` | colour index from channels 0–3 |
| `camera([x, y])` | offset all drawing by (−x, −y). With no arguments, resets |
| `pal([a, b])` | from now on, drawing in colour `a` puts colour `b`: recolour sprites, flash a hit enemy white. `pal()` puts every colour back |
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
| `set_blend([b])` | how drawn colours mix with the screen, per red / green / blue channel (0–3): `BLEND_NORMAL` replaces, `BLEND_ADD` adds (up to 3), `BLEND_SUB` subtracts from the screen (down to 0), `BLEND_MAX` / `BLEND_MIN` keep the higher / lower, `BLEND_XOR` XORs. `set_blend()` is `BLEND_NORMAL`. `cls()` ignores it |

### The font

In the built-in font every character is 6 × 10 pixels: a letter 5 pixels wide, with a column of space to its right and a row above. Capitals are 7 pixels tall. Lowercase letters are 5, with ascenders (b d f h k l t) as tall as the capitals, and g j p q y hang two rows below the line, into the cell's bottom row.

This is the **built-in font**. The shell and the editors always use it. A cart's `text()` uses it too, until the cart has a font of its own: `fonts/font.g64`, made in the [font editor](#font-editor) (F7), or others loaded with `load_font`.

### Tiles and map

These use the tileset and map in use: the one swapped in last (slot 0 to start with).

| Command | |
|---|---|
| `sprite(n, x, y [, w, h, flags])` | draw tile `n` (0–959), or a w×h block of tiles from the sheet (40 per row). Flags: `FLIP_X`, `FLIP_Y` |
| `tile_pget(n, x, y)` / `tile_pset(n, x, y, c)` | read / write a tile pixel (−1 = transparent) |
| `map_draw([mx, my, sx, sy, w, h])` | draw map cells `mx,my` .. `+w,+h` at screen `sx,sy`. Empty cells are not drawn |
| `map_get(x, y)` / `map_set(x, y, t)` | read / change a map cell. An empty cell, or one outside the map, reads as −1; `map_set(x, y, -1)` empties a cell |
| `map_w()` / `map_h()` | the map's size in cells |

Changes a cart makes with `tile_pset` and `map_set` last until it stops. The next run starts from the cart's files again.

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

When a cart starts, slot 0 of each kind holds the file with the default name (`tileset`, `map`, `sfx`, `music`, `font`), or an empty one if the cart has none (for a font, the built-in one). Slot 0 is in use. A file open in an editor is loaded with its unsaved changes, so Ctrl+R runs what you see. Swapping to an empty slot is an error. Emptying the slot in use leaves nothing to draw: `map_get` reads −1, and `tile_pset` / `map_set` are errors.

```
load_map(1, "cave") : load_tileset(1, "cave-tiles")
swap_map(1) : swap_tileset(1)          ' into the cave
```

### The dot limit

GooberEngine draws in software, so each frame has a budget of pixels ("dots"). Every pixel a cart draws counts, including `cls`, shapes, text, sprites and maps. Pixels outside the clip rectangle and a tile's transparent pixels don't count. Past the limit, further pixels are dropped until the next `flip()`, so an over-budget frame shows up unfinished.

| Command | |
|---|---|
| `dot_limit([n])` | set the limit (0 to 1,000,000; 100,000 to start with). Returns the limit; `dot_limit()` alone reads it |
| `dots_used()` | pixels drawn since the last `flip()` |

A full-screen `cls()` costs 96,000 of the 100,000 dots. Draw only what changes: skip `cls()` when the map covers the screen, and use `clip()` to keep drawing to the part that needs it. [Seedling](carts/seedling/main.gb) does both.

### Sound

| Command | |
|---|---|
| `sfx(n [, voice])` | play sound effect `n` (0–63) on a voice (0–7), or on any free voice. Returns the voice it used |
| `note(voice, pitch, wave, vol [, ticks])` | play one note: pitch 0–63 (24 = C-4, 33 = A-4 at 440 Hz), wave 0–15 or `NOISE`, volume 0–7. It lasts `ticks` (120 a second), or until stopped |
| `voice_stop([voice])` | stop one voice, or all of them |
| `voice_sfx(voice)` | the effect a voice is playing: −1 for nothing, −2 for a `note` |

| `music([n])` | play the song from order entry `n` (default 0) |
| `music_stop()` | stop the music |
| `music_pos()` / `music_row()` | the order entry and row playing, −1 when stopped |

`voice_sfx` gives −3 for a voice the music is playing. All sound and music stop when the cart does. Constants: `VOICES` (8) and `NOISE` (the noise "wave").

### Input and system

| Command | |
|---|---|
| `btn(b)` / `btn_hit(b)` | pad button held / pressed this frame: `BTN_LEFT` `BTN_RIGHT` `BTN_UP` `BTN_DOWN` `BTN_A` `BTN_B` `BTN_C` `BTN_START` |
| `key_down(k)` / `key_hit(k)` | raw keyboard, same `KEY_` constants as GooberBASIC |
| `mouse_x()` `mouse_y()` `mouse_down(b)` `mouse_hit(b)` | mouse in screen pixels; `MOUSE_LEFT` / `MOUSE_RIGHT` / `MOUSE_MIDDLE` |
| `flip()` | show the frame and wait for the next one (60 fps). The screen is **not** cleared |
| `frame_count()` | frames since boot |
| `cart_title()` | the cart's title |

Constants: `SCREEN_W` `SCREEN_H` `TILE_SIZE` `SHEET_COLS` (40) `NTILES` (960) `SLOTS` (10) `MAP_SCREEN_W` (40) `MAP_SCREEN_H` (24) `FONT_W` (6) `FONT_H` (10). (`MAP_W` and `MAP_H` are gone: maps vary in size, so use `map_w()` and `map_h()`.) Named colours:

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

A cart that draws once and ends without calling `flip()` keeps its picture on screen until you press a key. `print` still writes to the terminal, which is handy for debugging.

## Example carts

The examples are in their own folder: `CD examples`, then `DIR`, `RUN depth` and so on (`CD ..` comes back).

| Cart | Shows |
|---|---|
| [carts/hello](carts/hello/main.gb) | the palette, shapes, text, pad input |
| [carts/examples/depth](carts/examples/depth/main.gb) | the depth buffer: circles and squares orbiting a planet, drawn in a shuffled order and still sorted by `set_depth`. A turns depth off to compare with plain draw order |
| [carts/examples/rotate](carts/examples/rotate/main.gb) | sprites of 1×1, 2×2 and 4×4 tiles turning around their centres with `tex_quad`, big and at their real size |
| [carts/examples/cube](carts/examples/cube/main.gb) | a dice cube (a 2×2-tile texture per face) spinning in perspective with `tex_quad_3d`: back-face culling, and a second cube cutting through the first |
| [carts/seedling](carts/seedling/main.gb) | a small game with a tileset, map, sound bank and song. Collisions with `map_get`, `map_set`, sound effects over a looping tune, sprite flipping, a following camera, a HUD, and staying inside the dot limit |

## Sharing carts

`EXPORT` turns a cart folder into one `.gbcart` text file, and `IMPORT` (or `LOAD`, or `gooberengine file.gbcart`) turns it back into a folder in the carts directory:

```
GOOBERCART 1
file cart.txt
|title = Seedling                 every line of a file sits behind a |
file main.gb
|' Seedling - gather every golden seed ...
end
```

- **What's bundled:** the cart's `.gb` and `.txt` files, and the `.g64` files in `tiles/`, `maps/`, `sfx/` and `music/` (named `file tiles/tileset.g64` and so on), plus any old `.txt` sound or music not yet converted. Not `*.unsaved` or `*.old` leftovers, or save data.
- **Names:** import refuses names that would leave the folder, and won't overwrite a cart that exists.
- **Bad bundles:** a cut-short bundle is rejected, with nothing left behind.

## GooberEngine in the browser

`make web` builds the whole engine (shell, editors and all) with Emscripten and serves it at http://localhost:8000. The files in `build/web` work from any static web server; copy `runtimes/` in beside them (`make web-build` does) and `EXPORT` works in the browser too.

- **Your carts stay in the browser.** They live in its IndexedDB storage, saved every couple of seconds and when you leave the page. The example carts are copied in on the first visit; a new example in a later build appears, and your own carts are never overwritten.
- **Moving carts in and out:** drop a `.gbcart` on the page, use the **open .gbcart** button, or type `UPLOAD`. `DOWNLOAD` (or `EXPORT`) saves the loaded cart to your computer. `EXPORT LINUX / WINDOWS / WEB` downloads a finished game; the web one comes as a `.zip`.
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

- The cart travels as its `.gbcart` text: its source and data files. The player compiles `main.gb` as it starts, which takes milliseconds. So the code of an exported game can be read by anyone who looks for it, just as with the `.gbcart`.
- **Linux and Windows:** the `.gbcart` is appended to the player executable, followed by an 80-byte trailer: the cart name (64 bytes), the `.gbcart` length (8 bytes, little-endian) and the magic `GOOBERPK`. A program ignores bytes after its own end, so the result is an ordinary executable. At start the player reads its own file, unpacks the cart into a temporary folder, and removes it on exit.
- **Web:** the `.gbcart` is embedded, base64, in a `<script>` tag in `index.html`, next to the unchanged `player.js` and `player.wasm`.
- **Save data** belongs to the player's machine: `~/.local/share/gooberengine/saves/` on Linux, `%APPDATA%\GooberEngine\saves\` on Windows, and the browser's storage on the web (per site).

**Things to know:**

- Linux browsers' downloads lose the "executable" mark: `chmod +x mygame-linux-x86_64` once.
- The Windows `.exe` isn't code-signed, so Windows SmartScreen asks "run anyway?" the first time.
- A player run with `--headless --frames N --shot file.png` draws N frames without a window, which is what the tests use. `--cart file.gbcart` plays a `.gbcart` with a bare runtime.

**Where the runtimes come from.** `make runtimes` builds them into `runtimes/`:

| Runtime | Built with |
|---|---|
| `player-linux-x86_64` | the system compiler, with SDL2 built from source as a static library whose back ends are loaded at run time. Only glibc is linked dynamically. Because it's built against this machine's glibc, it needs glibc 2.38 or newer; building it on an older distribution reaches older systems |
| `player-windows-x86_64.exe` | `x86_64-w64-mingw32-gcc`, with SDL2 cross-compiled and linked in statically |
| `web/player.js`, `player.wasm`, `index.html` | Emscripten's SDL2 port, with `-sASYNCIFY` so the frame loop can hand control back to the browser |

GooberEngine finds the runtimes in `$GOOBERENGINE_RUNTIMES`, then in `runtimes/` next to the executable, then in `./runtimes`. The web version fetches them from `runtimes/` on its own server.

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
   - A full-screen GooberBASIC editor inside the console: syntax colours from the compiler's own tables, scrolling, mouse and keyboard selection, and the system clipboard.
   - Undo/redo, find, go to line, comment toggling, and block indenting.
   - Ctrl+R to save and run, and jump-to-line on errors.
   - `NEW name` creates a cart and `SAVE` writes `main.gb`.
   - Scripted-input tests (`--type`).
   - Still to do: editing the `include`d files, find-and-replace, a scrollbar.
3. **Tile editor** ✅
   - A pixel editor for 1×1, 2×2 or 4×4 tile blocks, with a 64-colour palette (A/B colours) and the whole sheet beside it.
   - Pencil, fill, line, rectangle, colour pick, flip/turn, nudge, copy/paste of blocks, and 64-step undo.
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
   - 16 drawable waveforms plus pitched noise, with click-free volume changes.
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
   - One-file `.gbcart` `EXPORT` / `IMPORT`.
   - VS Code highlighting for the console API: `make vscode`, which uses a new `--with` option in GooberBASIC's grammar generator.

8. **Web and export** ✅
   - The whole engine in the browser (`make web`): Emscripten with ASYNCIFY, carts kept in IndexedDB, upload, download and drag-and-drop of `.gbcart` files, sound unlocked on the first gesture, and game controllers through SDL.
   - `EXPORT LINUX / WINDOWS / WEB / ALL` and `--export`: prebuilt player runtimes with the cart packed in (appended plus a trailer for executables, embedded in `index.html` for the web).
   - `make runtimes`: SDL2 built from source for a self-contained Linux binary and a static mingw-w64 `.exe`.
   - The engine writes its own indexed-colour PNGs, so SDL2_image is no longer used. Dropping a cart on the desktop window loads it.
   - Still to do: a Linux runtime built against an older glibc for older distributions; macOS; a game icon and version info in the `.exe`; touch controls for phones.
9. **Many files** ✅
   - Tilesets of 960 tiles (400 × 240) and maps of 1–10 screens each way, in compact base-64 `.g64` files under `tiles/` and `maps/`. Sound banks and songs live under `sfx/` and `music/`.
   - 10 run-time slots per kind: `load_tileset` / `swap_tileset`, `load_map` / `swap_map`, `load_sfx` / `swap_sfx`, `load_music` / `swap_music`.
   - The dot limit: `dot_limit()` and `dots_used()`, 100,000 pixels a frame by default.
   - A file tree beside every editor, several `.gb` files in the code editor, and a tile editor with a pannable, zoomable tileset and floating tile and tools windows.
   - Old carts' `tiles.txt` / `map.txt` / `sfx.txt` / `music.txt` are read and converted by the next `SAVE`.
   - Right-click in the tree to rename or delete a file.
   - A settings screen (F9, or the button at the bottom of the file tree): theme, overall / music / sound-effect volume, fullscreen, window size and the code editor's indent mode, kept in `settings.txt`.
   - Fonts are a resource like the rest (`fonts/NAME.g64`, `load_font` / `swap_font`), with a font editor on F7. A font's characters can be 4 × 4 to 20 × 20, spacing included; `font_w()` / `font_h()` tell a cart the size. The built-in font is every cart's default.
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
| `src/tiles_editor.c` | the tile editor |
| `src/map_editor.c` | the map editor |
| `src/synth.c` | the sound chip: voices, waves, effects, mixer |
| `src/sound_editor.c` | the sound editor |
| `src/music_editor.c` | the music tracker |
| `src/save.c` | each cart's save data |
| `src/bundle.c` | `.gbcart` export / import |
| `src/browser.c` | the `BROWSE` cart picker |
| `src/export.c` | game export (packing carts into runtimes, the .zip writer) and the player's start-up |
| `src/web.c` | the browser build's downloads, uploads and dropped files |
| `src/main.c` | command line |
| `web/engine.html`, `web/player.html` | the pages around the web engine and around an exported web game |
| `third_party/SDL2-2.32.10.zip` | SDL2's source, for building the runtimes |
