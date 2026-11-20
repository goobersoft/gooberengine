#!/bin/sh
# GooberEngine tests: every cart compiles, boots headless, and errors reach the shell.
cd "$(dirname "$0")/.." || exit 1
GE=${GE:-./gooberengine}
pass=0; fail=0
ok()  { pass=$((pass + 1)); }
bad() { fail=$((fail + 1)); echo "FAIL: $1"; [ -n "$2" ] && printf '%s\n' "$2" | head -20; }

for c in carts/*/; do
    out=$($GE --check "$c" 2>&1) && ok || bad "$c does not compile" "$out"
    out=$($GE --headless --frames 30 "$c" 2>&1) && ! printf '%s' "$out" | grep -q error && ok || bad "$c fails to run" "$out"
done

expect() {  # expect <name> <args> <text the shell must print>
    out=$(eval "$GE --headless --frames 30 $2" 2>&1)
    printf '%s' "$out" | grep -qF -- "$3" && ok || bad "$1: wanted '$3'" "$out"
}
expect "compile error"  tests/carts/compile_error "main.gb:3: error:"
expect "runtime error"  tests/carts/runtime_error "main.gb:4: runtime error:"
expect "call trace"     tests/carts/runtime_error "called from boom"
expect "shell survives" "--exec help tests/carts/compile_error" "RUN [name]"
expect "draw once"      tests/carts/draw_once "map 7 -1"
expect "tile pixels"    tests/carts/draw_once "tile 48 -1"
expect "pixel read"     tests/carts/draw_once "pixel 1"
expect "missing cart"   "--exec \"load nope\"" "no cart 'nope'"
expect "dir"            "--exec dir" "seedling"

# ---- slots, the dot limit, the old file formats ----
expect "slot 0"         tests/carts/slots "a 10 7 40x24"
expect "load a slot"    tests/carts/slots "b -1 0"
expect "swap a slot"    tests/carts/slots "c 5 -1 1"
expect "map slot size"  tests/carts/slots "d 937 -1 80x24"
expect "free a slot"    tests/carts/slots "e -1 0"
expect "tile 0 is a tile" tests/carts/slots "f 0 -1"
expect "empty slot"     tests/carts/slots "swap_map: slot 7 is empty"
expect "dot limit"      tests/carts/dots "limit 100000"
expect "cls counts"     tests/carts/dots "cls 96000"
expect "over the limit" tests/carts/dots "full 100000 2 1"
expect "flip resets"    tests/carts/dots "flip 0"
expect "clip counts"    tests/carts/dots "clip 100"
expect "dot_limit(n)"   tests/carts/dots "small 5 4 3"
expect "dot_limit range" tests/carts/dots "dot_limit: -1 out of range 0-1000000"
expect "bad .g64"       tests/carts/badg64 "tileset.g64: a 10 x 10 tileset has 100 pixels, this one has 3"
LC=$PWD/build/lcarts; rm -rf "$LC" && mkdir -p "$LC" && cp -r tests/carts/legacy "$LC/"
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 legacy 2>&1); said() { printf '%s' "$out" | grep -qF -- "$2" && ok || bad "$1: wanted '$2'" "$out"; }
said "old formats read" 't 48 m -1 5 160x96'
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 --exec "load legacy" --exec save --exec run 2>&1)
said "old formats saved" 'saved tiles/tileset.g64, maps/map.g64, sfx/sfx.txt, music/music.txt'
said "converted cart runs the same" 't 48 m -1 5 160x96'
[ -f "$LC/legacy/tiles.txt.old" ] && [ ! -f "$LC/legacy/tiles.txt" ] && [ -f "$LC/legacy/maps/map.g64" ] && ok || bad "old files become *.old" "$(ls -R "$LC/legacy")"
[ "$(head -1 "$LC/legacy/maps/map.g64")" = 33 ] && ok || bad "an old map is 4 x 4 screens" "$(head -1 "$LC/legacy/maps/map.g64")"

# ---- the code editor, driven by scripted keys in a scratch carts folder ----
TC=$PWD/build/tcarts
edit() {    # edit <name> <keys>: NEW cart "t", type the keys; leaves output in $out, code in $TC/t/main.gb
    rm -rf "$TC" && mkdir -p "$TC"
    out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 400 --exec "new t" --type "$2" 2>&1)
}
has()  { grep -qxF -- "$2" "$TC/t/main.gb" && ok || bad "$1: main.gb has no line '$2'" "$(cat "$TC/t/main.gb")"; }
hasnt() { [ -f "$TC/t/main.gb" ] || { bad "$1: no main.gb"; return; }; grep -qF -- "$2" "$TC/t/main.gb" && bad "$1: main.gb still has '$2'" "$(cat "$TC/t/main.gb")" || ok; }
said() { printf '%s' "$out" | grep -qF -- "$2" && ok || bad "$1: wanted '$2'" "$out"; }

edit "type, save"   '{ctrl+end}{enter}print("added"){ctrl+s}';          has "type, save" 'print("added")'
edit "auto indent"  '{ctrl+end}{enter}if 1{enter}x = 1{ctrl+s}';         has "auto indent" '  x = 1'
edit "undo"         '{ctrl+end}{enter}abc{ctrl+z}{ctrl+s}';             hasnt "undo" 'abc'
edit "redo"         '{ctrl+end}{enter}abc{ctrl+z}{ctrl+y}{ctrl+s}';     has "redo" 'abc'
edit "copy line"    '{ctrl+home}{ctrl+c}{ctrl+end}{enter}{ctrl+v}{ctrl+s}'
n=$(grep -c "^' t - a new" "$TC/t/main.gb"); [ "$n" = 2 ] && ok || bad "copy line: wanted 2 copies, got $n"
edit "find"         '{ctrl+f}COL_YELLOW{enter}COL_RED{ctrl+s}';         has "find" '  circle_fill(x, y, 8, COL_RED)'
edit "comment"      '{ctrl+g}2{enter}{ctrl+/}{ctrl+s}';                  has "comment" "' strict"
edit "select"       '{ctrl+home}{shift+down}{shift+down}{del}{ctrl+s}'; hasnt "select" 'strict'
edit "unindent"     '{ctrl+g}7{enter}{shift+tab}{ctrl+s}';              has "unindent" 'if btn(BTN_LEFT) then x :- 2'
edit "error line"   '{ctrl+g}3{enter}local a:int[2] : a[5] = 1{ctrl+r}'; said "error line" '[editor] line 3: Array index 5'
edit "run, esc"     '{ctrl+r}{esc}{ctrl+end}{enter}zzz{ctrl+s}';         has "run, esc" 'zzz'
edit "unsaved"      '{ctrl+end}{enter}q{esc}new t2{enter}';              said "unsaved" 'unsaved changes'
edit "discard"      '{ctrl+end}{enter}q{esc}new t2{enter}y';            said "discard" 'created'
edit "keep"         '{ctrl+end}{enter}q{esc}new t2{enter}n';            said "keep" 'kept your changes'
edit "keep (esc)"   '{ctrl+end}{enter}q{esc}new t2{enter}{esc}';        said "keep (esc)" 'kept your changes'

# ---- the tile editor. {f6} hides the file tree, so the editor has the whole width.
# The tile window's canvas is at (203,22), 12 px per pixel for one tile: pixel (x, y)
# is clicked at (209 + 12x, 28 + 12y). In the tools window, palette colour i is at
# (335 + (i mod 8) * 8, 53 + (i / 8) * 8) ----
TT=$TC/t/tiles/tileset.g64
row() {     # row <name> <tile> <row> <expected 10 characters>: from the tileset's rows (one per line)
    got=$(awk -v l=$(( ($2 / 40) * 10 + $3 + 2 )) -v c=$(( ($2 % 40) * 10 + 1 )) 'NR == l { s = substr($0, c, 10); while (length(s) < 10) s = s "."; print s }' "$TT" 2>/dev/null)
    [ "$got" = "$4" ] && ok || bad "$1: tile $2 row $3 is '$got', wanted '$4'" "$(cat "$TT" 2>/dev/null)"
}
edit "draw"      '{f2}{f6}{click:209,28}{ctrl+s}';                     row "draw" 1 0 '*.........'
edit "flip"      '{f2}{f6}{click:209,28}{h}{ctrl+s}';                  row "flip" 1 0 '.........*'
edit "turn"      '{f2}{f6}{click:209,28}{t}{ctrl+s}';                  row "turn" 1 0 '.........*'
edit "fill"      '{f2}{f6}{f}{click:209,28}{ctrl+s}';                  row "fill" 1 9 '**********'
edit "line"      '{f2}{f6}{l}{click:209,40}{drag:317,40}{mouse:317,40}{ctrl+s}'; row "line" 1 1 '**********'
row "line" 1 0 '..........'
edit "tile undo" '{f2}{f6}{click:209,28}{click:221,28}{ctrl+z}{ctrl+s}'; row "tile undo" 1 0 '*.........'
edit "paste"     '{f2}{f6}{click:209,28}{ctrl+c}{right}{ctrl+v}{ctrl+s}'; row "paste" 2 0 '*.........'
edit "colour"    '{f2}{f6}{click:335,69}{click:209,28}{ctrl+s}';        row "colour" 1 0 'g.........'
edit "far tile"  '{f2}{f6}{down}{down}{down}{right}{click:209,28}{ctrl+s}'; row "far tile" 122 0 '*.........'
[ "$(head -1 "$TT")" = 23 ] && ok || bad "a tileset is saved only as big as it's used" "$(head -1 "$TT")"
edit "pick on sheet" '{f2}{f6}{click:65,15}{click:209,28}{ctrl+s}';    row "pick on sheet" 3 0 '*.........'
edit "move window" '{f2}{f6}{click:220,16}{drag:120,16}{mouse:120,16}{click:109,28}{ctrl+s}'; row "move window" 1 0 '*.........'
edit "tiles unsaved" '{f2}{f6}{click:209,28}{esc}new t2{enter}';       said "tiles unsaved" 'unsaved changes in the tiles'

# a running cart's map_set / tile_pset last only for that run
rm -rf "$TC" && mkdir -p "$TC/r"
printf 'print("m " + str(map_get(1, 1)) + " " + str(tile_pget(3, 0, 0)))\nmap_set(1, 1, 9) : tile_pset(3, 0, 0, 5)\n' > "$TC/r/main.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 30 --exec "run r" --exec "run r" 2>&1)
n=$(printf '%s\n' "$out" | grep -c '^m -1 -1$'); [ "$n" = 2 ] && ok || bad "runs start fresh" "$out"

# ---- the map editor (tree hidden): at the starting zoom, cell (x, y) is drawn at (x*10, 10 + y*10);
# in the picker, tile t is at (2 + (t mod 40) * 10, 20 + (t / 40) * 10) ----
MT=$TC/t/maps/map.g64
maprow() {  # maprow <name> <y> <expected cells, space separated: two characters each, .. = empty>
    got=$(awk -v l=$(( $2 + 2 )) 'NR == l { s = $0; sub(/(\.\.)+$/, "", s); out = ""; for (i = 1; i <= length(s); i += 2) out = out (i > 1 ? " " : "") substr(s, i, 2); print out }' "$MT" 2>/dev/null)
    [ "$got" = "$3" ] && ok || bad "$1: map row $2 is '$got', wanted '$3'" "$(head -4 "$MT" 2>/dev/null)"
}
edit "map pen"    '{f3}{f6}{click:5,15}{drag:25,15}{mouse:25,15}{ctrl+s}';           maprow "map pen" 0 "01 01 01"
edit "map rect"   '{f3}{f6}{r}{click:15,25}{drag:35,35}{mouse:35,35}{ctrl+s}'
maprow "map rect" 1 ".. 01 01 01"; maprow "map rect" 2 ".. 01 01 01"
edit "map fill"   '{f3}{f6}{f}{click:55,55}{ctrl+s}';                                maprow "map fill" 95 "$(printf '01 %.0s' $(seq 159))01"
edit "map erase"  '{f3}{f6}{click:5,15}{e}{click:5,15}{click:15,15}{ctrl+s}';        maprow "map erase" 0 ""
edit "map picker" '{f3}{f6}{tab}{click:47,25}{click:5,15}{ctrl+s}';                  maprow "map picker" 0 "04"
edit "map block"  '{f3}{f6}{tab}{click:17,25}{drag:27,35}{mouse:27,35}{click:5,15}{ctrl+s}'
maprow "map block" 0 "01 02"; maprow "map block" 1 "0F 0G"
edit "map tile 0" '{f3}{f6}{[}{click:5,15}{ctrl+s}';                                 maprow "map tile 0" 0 "00"
edit "map pan"    '{f3}{f6}{right}{right}{click:5,15}{ctrl+s}';                      maprow "map pan" 0 ".. .. 01"
edit "map undo"   '{f3}{f6}{click:5,15}{click:15,15}{ctrl+z}{ctrl+s}';               maprow "map undo" 0 "01"
edit "map resize" '{f3}{f6}{ctrl+right}{ctrl+down}{ctrl+down}{ctrl+up}{ctrl+s}'
[ "$(head -1 "$MT")" = 44 ] && [ "$(sed -n 2p "$MT" | wc -c)" = 401 ] && ok || bad "map resize: wanted 5 x 5 screens" "$(head -c 300 "$MT")"
edit "map unsaved" '{f3}{f6}{click:5,15}{esc}new t2{enter}';                         said "map unsaved" 'unsaved changes in the map'

# with Seedling's map: an unchanged save is byte-for-byte the same; right-click picks;
# select + copy makes a brush; a run sees unsaved edits
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
echo 'print("c " + str(map_get(0, 0)))' > "$TC/s/main.gb"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 30 --exec "load s" --type '{f3}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/maps/map.g64 "$TC/s/maps/map.g64" && ok || bad "map round trip" "$(diff carts/seedling/maps/map.g64 "$TC/s/maps/map.g64" | head)"
MT=$TC/s/maps/map.g64
cell() { awk -v l=$(( $2 + 2 )) -v c=$(( $1 * 2 + 1 )) 'NR == l { print substr($0, c, 2) }' "$MT"; }
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load s" \
      --type '{f3}{f6}{rclick:5,15}{click:55,65}{s}{click:5,15}{drag:25,25}{mouse:25,25}{ctrl+c}{click:105,115}{ctrl+s}' 2>&1)
got=$(cell 5 5); [ "$got" = 03 ] && ok || bad "map right-click pick: cell 5,5 is '$got'"
got="$(cell 10 10) $(cell 11 10) $(cell 12 10)"; [ "$got" = "03 03 03" ] && ok || bad "map copy brush: row 10 cells 10-12 are '$got'"
got=$(cell 10 11); [ "$got" = 03 ] && ok || bad "map copy brush: cell 10,11 is '$got'"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load s" --type '{f3}{f6}{e}{click:5,15}{ctrl+r}' 2>&1)
said "map run unsaved" 'c -1'

# ---- the file tree (shown by default, 72 px wide): rows are 10 px from y 11:
# code, main.gb, tiles, tileset, maps, map, sfx, sfx, music, music. + is at x 66 ----
rm -rf "$TC" && mkdir -p "$TC"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "new t" --type '{f3}{click:66,55}{click:77,15}{ctrl+s}' 2>&1)
[ -f "$TC/t/maps/map-1.g64" ] && [ ! -f "$TC/t/maps/map.g64" ] && ok || bad "tree +: a new map" "$out$(ls -R "$TC/t")"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "load t" --type '{f1}{click:66,15}x = 2{ctrl+s}{click:20,25}' 2>&1)
grep -qxF 'x = 2' "$TC/t/code-1.gb" 2>/dev/null && ok || bad "tree +: a new .gb file" "$out"
said "tree: switch file" '[editor] saved code-1.gb'
printf 'include "lib.gb"\nboom()\n' > "$TC/t/main.gb"; printf 'function boom()\n  local a:int[2]\n  a[5] = 1\nendfunction\n' > "$TC/t/lib.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load t" --type '{f1}{ctrl+r}' 2>&1)
said "error in an include opens it" '[editor] line 3: Array index 5'
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load t" --type '{f2}{click:30,85}{ctrl+s}' 2>&1)
said "tree: open a map from the tile editor" '[map] saved maps/map-1.g64'
# right-click a file: rename (type, Enter) or delete (y). Rows here: code, extra.gb, main.gb,
# tiles, tiles-1, tileset, maps, map, map-1 (from y 11, 10 px each); the menu's two lines follow the row
rm -rf "$TC" && mkdir -p "$TC" && cp -r tests/carts/slots "$TC/s" && printf 'x = 1\n' > "$TC/s/extra.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "load s" \
      --type '{f2}{rclick:30,65}{click:30,75}{backspace}{backspace}{backspace}cave{enter}{f1}{rclick:30,25}{click:30,35}{backspace}{backspace}{backspace}{backspace}{backspace}lib{enter}{rclick:30,95}{click:30,115}y{rclick:30,35}' 2>&1)
said "tree rename" 'renamed tileset to tilecave'
[ -f "$TC/s/tiles/tilecave.g64" ] && [ ! -f "$TC/s/tiles/tileset.g64" ] && ok || bad "tree rename: the file moves" "$(ls -R "$TC/s")"
said "tree rename code" 'renamed extra.gb to lib.gb'
[ -f "$TC/s/lib.gb" ] && [ ! -f "$TC/s/extra.gb" ] && ok || bad "tree rename code: the file moves" "$(ls "$TC/s")"
said "tree delete" 'deleted map-1'
[ ! -f "$TC/s/maps/map-1.g64" ] && [ -f "$TC/s/maps/map.g64" ] && ok || bad "tree delete: only map-1 goes" "$(ls -R "$TC/s")"
said "main.gb stays" 'main.gb stays'
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 100 --exec "load s" --type '{f3}{rclick:30,85}{click:30,105}n' 2>&1)
[ -f "$TC/s/maps/map.g64" ] && ok || bad "tree delete: n keeps the file" "$out"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 100 --exec "load s" --type '{f2}{rclick:30,65}{click:30,75}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}tilecave{enter}' 2>&1)
said "tree rename: a name in use" 'tiles/tilecave.g64 already exists'

# ---- sound: the synth's pitch and timing, measured from a recording ----
if command -v python3 > /dev/null; then
    $GE --headless --frames 70 --wav build/tone.wav tests/carts/tone > /dev/null 2>&1
    got=$(python3 -I tests/wav_check.py build/tone.wav 0.1 0.45)
    case "$got" in *"freq 440"*) ok ;; *) bad "note A-4 is 440 Hz" "$got" ;; esac
    got=$(python3 -I tests/wav_check.py build/tone.wav 0.6 1.0)
    case "$got" in "peak 0 "*) ok ;; *) bad "voice_stop silences" "$got" ;; esac
    out=$($GE --headless --frames 70 --wav build/sfx.wav tests/carts/sfx 2>&1)
    got=$(python3 -I tests/wav_check.py build/sfx.wav 0.75 1.1)
    case "$got" in "peak 0 "*) ok ;; *) bad "an sfx ends" "$got" ;; esac
else
    echo "(no python3: skipping the recorded-sound checks)"
fi
out=$($GE --headless --frames 70 tests/carts/sfx 2>&1)
said "voice_sfx" 'voice 3 plays 2'; said "sfx ends" 'after -1'
printf 'sfx(64)\n' > build/bad_sfx.gb && mkdir -p build/bad_sfx && mv build/bad_sfx.gb build/bad_sfx/main.gb
out=$($GE --headless --frames 10 build/bad_sfx 2>&1); said "sfx range" 'sfx: 64 out of range'

# ---- the sound editor: Z row = notes, 1-7 = volume, Tab = effect ----
ST=$TC/t/sfx/sfx.txt
sfxhas() { grep -qF -- "$2" "$ST" 2>/dev/null && ok || bad "$1: sfx/sfx.txt has no '$2'" "$(cat "$ST" 2>/dev/null)"; }
edit "notes"       '{f4}{f6}zx{ctrl+s}';                                 sfxhas "notes" 'C-4 0 5 .  D-4 0 5 .  --- 0 0 .'
edit "volume"      '{f4}{f6}z{left}7{ctrl+s}';                           sfxhas "volume" 'C-4 0 7 .'
edit "effect"      '{f4}{f6}z{left}{tab}{ctrl+s}';                       sfxhas "effect" 'C-4 0 5 s'
edit "octave"      '{f4}{f6}{]}z{ctrl+s}';                               sfxhas "octave" 'C-5 0 5 .'
edit "wave pen"    '{f4}{f6}.z{ctrl+s}';                                 sfxhas "wave pen" 'C-4 1 5 .'
edit "length"      '{f4}{f6}zxc{left}{ctrl+l}{ctrl+s}';                  sfxhas "length" 'sfx 0 speed 8 length 3 loop 0 0'
edit "loop"        '{f4}{f6}z{ctrl+b}{right}{right}{ctrl+e}{ctrl+s}';    sfxhas "loop" 'length 32 loop 1 4'
edit "speed"       '{f4}{f6}z{-}{-}{ctrl+s}';                            sfxhas "speed" 'sfx 0 speed 6'
edit "next sfx"    '{f4}{f6}{pgdn}z{ctrl+s}';                            sfxhas "next sfx" 'sfx 1 speed 8'
edit "draw pitch"  '{f4}{f6}{click:71,57}{ctrl+s}';                      sfxhas "draw pitch" 'F#4 0 5 .'
edit "draw wave"   '{f4}{f6}{click:67,223}{ctrl+s}';                     sfxhas "draw wave" 'wave 0 0fffffffffffffff0000000000000000'
edit "sound undo"  '{f4}{f6}z{ctrl+z}{ctrl+s}'
grep -q '^sfx' "$ST" && bad "sound undo: sfx.txt still has an sfx" "$(cat "$ST")" || ok
edit "sound unsaved" '{f4}{f6}z{esc}new t2{enter}';                      said "sound unsaved" 'unsaved changes in the sound'
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 20 --exec "load s" --type '{f4}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/sfx/sfx.txt "$TC/s/sfx/sfx.txt" && ok || bad "sfx round trip" "$(diff carts/seedling/sfx/sfx.txt "$TC/s/sfx/sfx.txt" | head)"

# ---- music: timing, pitch and the end of the song ----
out=$($GE --headless --frames 130 --wav build/music.wav tests/carts/music 2>&1)
said "music row" 'row 10 voice -3'; said "music ends" 'end -1 -1'
if command -v python3 > /dev/null; then
    got=$(python3 -I tests/wav_check.py build/music.wav 0.1 0.7)
    case "$got" in *"freq 440"*) ok ;; *) bad "music A-4 is 440 Hz" "$got" ;; esac
    got=$(python3 -I tests/wav_check.py build/music.wav 0.9 1.5)
    case "$got" in "peak 0 "*) ok ;; *) bad "note off silences" "$got" ;; esac
fi

# ---- the music editor: the cursor starts on track 1's note field ----
MU=$TC/t/music/music.txt
mushas() { grep -qxF -- "$2" "$MU" 2>/dev/null && ok || bad "$1: music.txt has no line '$2'" "$(head -12 "$MU" 2>/dev/null)"; }
edit "mus note"    '{f5}{f6}z{ctrl+s}';                               mushas "mus note" '  C-4 0 5 .'
mushas "mus tracks" 'pattern 0 tracks 1'
edit "mus wave"    '{f5}{f6}z{up}{right}n{ctrl+s}';                   mushas "mus wave" '  C-4 n 5 .'
edit "mus vol"     '{f5}{f6}z{up}{right}{right}7{ctrl+s}';            mushas "mus vol" '  C-4 0 7 .'
edit "mus fx"      '{f5}{f6}z{up}{right}{right}{right}v{ctrl+s}';     mushas "mus fx" '  C-4 0 5 v'
edit "mus off"     '{f5}{f6}1{ctrl+s}';                               mushas "mus off" '  === . . .'
edit "mus track"   '{f5}{f6}{tab}q{ctrl+s}';                          mushas "mus track" '  ... . . .  C-5 0 5 .'
edit "mus order"   '{f5}{f6}{ctrl+right}{ctrl+up}{ctrl+up}z{ctrl+s}'; mushas "mus order" 'order 0 1'
mushas "mus order pattern" 'pattern 1 tracks 1'
edit "mus jump"    '{f5}{f6}{ctrl+right}{ctrl+j}{ctrl+s}';            mushas "mus jump" 'order 0 >0'
edit "mus speed"   '{f5}{f6}-{ctrl+s}';                               mushas "mus speed" 'song speed 7'
edit "mus undo"    '{f5}{f6}z{ctrl+z}{ctrl+s}'
grep -q '^pattern' "$MU" && bad "mus undo: music.txt still has a pattern" "$(cat "$MU")" || ok
edit "mus unsaved" '{f5}{f6}z{esc}new t2{enter}';                     said "mus unsaved" 'unsaved changes in the music'
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 20 --exec "load s" --type '{f5}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/music/music.txt "$TC/s/music/music.txt" && ok || bad "music round trip" "$(diff carts/seedling/music/music.txt "$TC/s/music/music.txt" | head)"

# ---- polish: pal, save data, labels, BROWSE, .gbcart ----
out=$($GE --headless --frames 5 tests/carts/pal 2>&1);           said "pal" 'pal 3 3 48'
SV=$PWD/build/saves; rm -rf "$SV"
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 tests/carts/savedata 2>&1)
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 tests/carts/savedata 2>&1)
said "save data" 'runs 2 note two|lines ~ tilde missing []'
printf 'save_data("bad key", "x")\n' > build/bad_sfx/main.gb
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 build/bad_sfx 2>&1); said "save key" "save_data: key 'bad key' must be"

rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/hello "$TC/" && rm -f "$TC/hello/label.txt"
WAIT=$(printf '{mouse:0,0}%.0s' 1 2 3 4 5)
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 30 --type "${WAIT}{f8}" hello 2>&1)
said "label" "saved the cart's label"
[ "$(grep -vc '^#' "$TC/hello/label.txt" 2>/dev/null)" = 60 ] && ok || bad "label.txt has 60 rows"
cp -r carts/seedling "$TC/"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 40 --exec browse --type '{right}{enter}{esc}' 2>&1)
said "browse runs" 'Seedling stopped'

# export, then import under another name: every file comes back the same
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "load seedling" --exec export 2>&1)
said "export" "wrote $TC/seedling.gbcart"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/seedling.gbcart copy" 2>&1)
said "import" "loaded Seedling"
diff -r -x '.*' carts/seedling "$TC/copy" > /dev/null && ok || bad "import gives back every file unchanged" "$(diff -r -x '.*' carts/seedling "$TC/copy" | head)"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/seedling.gbcart copy" 2>&1)
said "import refuses a taken name" "cart 'copy' already exists"
cp "$TC/seedling.gbcart" "$TC/rocket.gbcart"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "load $TC/rocket.gbcart" 2>&1)
said "load a .gbcart" 'loaded Seedling'; [ -f "$TC/rocket/main.gb" ] && ok || bad "load a .gbcart unpacks it"
head -20 "$TC/seedling.gbcart" > "$TC/cut.gbcart"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/cut.gbcart" 2>&1)
said "cut-short bundle" 'is cut short'; [ ! -e "$TC/cut" ] && ok || bad "a bad bundle leaves no folder behind"

# ---- the pad: d-pad + A B C + Start; keys Z (or Space) X C Enter ----
out=$($GE --headless --frames 30 --type 'zxc{enter}v ' tests/carts/pad 2>&1)
said "pad size" 'buttons 8 c 6'
got=$(printf '%s\n' "$out" | grep '^pad [ABCS]*$' | tr '\n' ' ')
[ "$got" = "pad A pad B pad C pad S pad A " ] && ok || bad "pad keys: got '$got'" "$out"
mkdir -p build/old_btn && printf 'strict\nprint(btn(BTN_Y))\n' > build/old_btn/main.gb
out=$($GE --check build/old_btn 2>&1); said "BTN_Y is gone" 'BTN_Y'

# ---- export: standalone games. Most checks need the player runtimes (make runtimes) ----
EX=$PWD/build/texport
rm -rf "$EX"
out=$(GOOBERENGINE_RUNTIMES=/nonexistent $GE --export linux seedling "$EX" 2>&1); said "export without runtimes" 'make runtimes'
out=$($GE --export nowhere,else seedling "$EX" 2>&1); said "export: bad target" 'EXPORT takes'
if [ -x runtimes/player-linux-x86_64 ] && [ -f runtimes/player-windows-x86_64.exe ] && [ -f runtimes/web/player.wasm ]; then
    out=$($GE --export all seedling "$EX" 2>&1); said "export all" 'seedling-web.zip'
    [ "$(tail -c 8 "$EX/seedling-linux-x86_64")" = GOOBERPK ] && ok || bad "linux game ends in the packed-cart trailer"
    [ "$(tail -c 8 "$EX/seedling-windows.exe")" = GOOBERPK ] && ok || bad "windows game ends in the packed-cart trailer"
    out=$("$EX/seedling-linux-x86_64" --headless --frames 60 --shot "$EX/game.png" 2>&1) && [ -s "$EX/game.png" ] && ok || bad "exported game runs" "$out"
    $GE --headless --frames 60 --shot "$EX/engine.png" seedling >/dev/null 2>&1
    cmp -s "$EX/game.png" "$EX/engine.png" && ok || bad "exported game draws what the engine draws"
    grep -q 'data-name="seedling"' "$EX/seedling-web/index.html" && ! grep -q '__GE_' "$EX/seedling-web/index.html" && ok || bad "web index.html has the cart in it"
    [ -s "$EX/seedling-web/player.wasm" ] && [ -s "$EX/seedling-web.zip" ] && ok || bad "web export has player.wasm and a .zip"
    out=$(runtimes/player-linux-x86_64 --headless 2>&1); said "a bare runtime says what it is" 'no game packed'
    out=$($GE --export linux tests/carts/runtime_error "$EX" 2>&1)
    out=$("$EX/runtime_error-linux-x86_64" --headless --frames 20 2>&1); said "game error reaches the player" 'runtime error:'
    out=$($GE --headless --frames 5 --exec "load hello" --exec "export $EX/hello.gbcart" 2>&1)
    out=$(runtimes/player-linux-x86_64 --headless --frames 20 --cart "$EX/hello.gbcart" 2>&1) && ok || bad "player --cart" "$out"
    out=$($GE --headless --frames 5 --exec "load hello" --exec "export web $EX/sh" 2>&1); said "shell EXPORT WEB" 'hello-web.zip'
    ls "${TMPDIR:-/tmp}"/gooberengine-* >/dev/null 2>&1 && bad "the player leaves its unpack folder behind" || ok
    if [ -n "$GE_TEST_WINE" ] && command -v wine >/dev/null; then   # slow: GE_TEST_WINE=1 make test
        WINEDEBUG=-all wine "$EX/seedling-windows.exe" --headless --frames 60 --shot "$EX/wine.png" >/dev/null 2>&1
        cmp -s "$EX/wine.png" "$EX/engine.png" && ok || bad "the windows game (under wine) draws what the engine draws"
    fi
else
    echo "(skipping most export tests: no runtimes/ - run make runtimes)"
fi

# ---- the untitled cart GooberEngine boots into ----
UT=$PWD/build/ucarts; rm -rf "$UT" && mkdir -p "$UT"
ut() { out=$(GOOBERENGINE_CARTS=$UT $GE --headless --frames 300 --type "$1" 2>&1); }
ut 'info{enter}';                               said "boots untitled" 'untitled: not saved'
ut '{f1}{ctrl+end}{enter}x = 1{ctrl+s}';        said "untitled ctrl+s" 'type SAVE name in the shell'
[ -z "$(ls "$UT")" ] && ok || bad "the untitled cart wrote to the carts folder" "$(ls "$UT")"
ut '{f1}{ctrl+end}{enter}print(1){esc}save keepme{enter}'; said "SAVE name" 'saved as'
grep -qxF 'print(1)' "$UT/keepme/main.gb" && ok || bad "SAVE name: main.gb lacks the edit" "$(cat "$UT/keepme/main.gb" 2>&1)"
grep -qxF 'title = keepme' "$UT/keepme/cart.txt" && ok || bad "SAVE name: cart.txt title" "$(cat "$UT/keepme/cart.txt" 2>&1)"
ut 'save keepme{enter}';                        said "SAVE name taken" "overwrite it? (Y/N)"
ut '{f1}{ctrl+end}{enter}print(2){esc}save keepme{enter}n'; said "SAVE taken, N" "not saved"
grep -qxF 'print(2)' "$UT/keepme/main.gb" && bad "SAVE taken, N: overwrote anyway" || ok
echo stale > "$UT/keepme/stale.txt"
ut '{f1}{ctrl+end}{enter}print(3){esc}save keepme{enter}y'; said "SAVE taken, Y" 'saved as'
grep -qxF 'print(3)' "$UT/keepme/main.gb" && ok || bad "SAVE taken, Y: main.gb lacks the edit" "$(cat "$UT/keepme/main.gb" 2>&1)"
[ ! -e "$UT/keepme/stale.txt" ] && ok || bad "SAVE taken, Y: the old cart's files linger"
ut 'save{enter}';                               said "SAVE alone" 'saved as ucarts/untitled'
ut 'save{enter}y';                              said "SAVE alone, taken" 'overwrite it? (Y/N)'; said "SAVE alone, Y" 'saved as ucarts/untitled'
ut '{f1}x{esc}load keepme{enter}';              said "untitled edits ask" 'discard them? (Y/N)'
ls "${TMPDIR:-/tmp}"/gooberengine-untitled-* >/dev/null 2>&1 && bad "the untitled cart's scratch folder is left behind" || ok

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
