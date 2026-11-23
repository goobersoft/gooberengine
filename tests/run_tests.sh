#!/bin/sh
# GooberEngine tests: every cart compiles, boots headless, and errors reach the shell.
cd "$(dirname "$0")/.." || exit 1
GE=${GE:-./gooberengine}
mkdir -p build && export GOOBERENGINE_SETTINGS=$PWD/build/settings.txt && rm -f "$GOOBERENGINE_SETTINGS"   # never the user's own
export GOOBERENGINE_CACHE=$PWD/build/cartcache.txt && rm -f "$GOOBERENGINE_CACHE"          # the zipped carts' stub cache
export TMPDIR=$PWD/build/tmp && rm -rf "$TMPDIR" && mkdir -p "$TMPDIR"    # scratch folders: only this run's (the leftover checks look here)
pass=0; fail=0
ok()  { pass=$((pass + 1)); }
bad() { fail=$((fail + 1)); echo "FAIL: $1"; [ -n "$2" ] && printf '%s\n' "$2" | head -20; }

for c in carts/*/ carts/*/*/; do
    [ -f "$c/main.gb" ] || continue      # carts/examples/ is a folder of carts, carts/x/tiles/ is not a cart
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
expect "esc stops a loop with no flip" "--type '{esc}' tests/carts/spin" "spin stopped"
expect "esc leaves the shell running" "--type '{esc}help{enter}' tests/carts/spin" "RUN [name]"   # --frames quits instead
expect "--frames ends a loop with no flip" tests/carts/spin "spin stopped"
MOUSE="--type '{mouse:10,10}{mouse:15,7}{click:15,7}{rclick:15,7}{mclick:15,7}{wheel:-2}{wheel:3}' tests/carts/mouse"
expect "mouse_dx/dy"    "$MOUSE" "m 000 w 0 d 5,-3"
expect "mouse_btn left"   "$MOUSE" "m 100 "
expect "mouse_btn right"  "$MOUSE" "m 010 "
expect "mouse_btn middle" "$MOUSE" "m 001 "
expect "wheel down is +"  "$MOUSE" "w 2 d"
expect "wheel up is -"    "$MOUSE" "w -3 d"
# networking: a host cart and a client cart, in two engines on this machine, over TCP then UDP
$GE --headless --frames 30 tests/carts/net_host > build/net_host.out 2>&1 &
net_pid=$!
sleep 0.3
net_out=$($GE --headless --frames 30 tests/carts/net_join 2>&1)
wait $net_pid
net_out="$net_out
$(cat build/net_host.out)"
for want in "net host 1: slot1 777.0 'hi from 1' from 1" "net host 2: slot1 777.0 'hi from 1' from 1" \
            "net client 1: id 1 slot0 1234.5 players 2" "net client 2: id 1 slot0 1234.5 players 2"; do
    printf '%s' "$net_out" | grep -qF -- "$want" && ok || bad "network: wanted '$want'" "$net_out"
done
expect "missing cart"   "--exec \"load nope\"" "no cart 'nope'"
expect "dir"            "--exec dir" "seedling"
expect "dir folders"    "--exec dir" "examples/"
expect "cd"             "--exec \"cd examples\" --exec dir" "rotate"
expect "cd load"        "--exec \"cd examples\" --exec \"load cube\"" "loaded Spinning Cube"
expect "cd .."          "--exec \"cd examples\" --exec \"cd ..\"" "in carts/"
expect "cd top"         "--exec \"cd ..\"" "already at the top"
expect "cd into a cart" "--exec \"cd examples/depth\"" "'depth' is a cart"
expect "cd nowhere"     "--exec \"cd nope\"" "no folder 'nope'"

# ---- THEME: the editors' colours, kept in settings.txt ----
expect "theme list"     "--exec theme" "theme: blue  (blue green orange grey)"
expect "theme set"      "--exec \"theme green\"" "theme: green"
grep -qxF 'theme = green' "$GOOBERENGINE_SETTINGS" && ok || bad "theme kept in settings.txt" "$(cat "$GOOBERENGINE_SETTINGS" 2>&1)"
expect "theme remembered" "--exec theme" "theme: green  ("
expect "theme white = grey" "--exec \"theme white\"" "theme: grey"
expect "theme unknown"  "--exec \"theme pink\"" "no theme 'pink'"
rm -f "$GOOBERENGINE_SETTINGS"


# ---- slots, the dot limit, the old file formats ----
expect "slot 0"         tests/carts/slots "a 10 7 40x24"
expect "load a slot"    tests/carts/slots "b -1 0"
expect "swap a slot"    tests/carts/slots "c 5 -1 1"
expect "map slot size"  tests/carts/slots "d 937 -1 80x24"
expect "free a slot"    tests/carts/slots "e -1 0"
expect "tile 0 is a tile" tests/carts/slots "f 0 -1"
expect "empty slot"     tests/carts/slots "swap_map: slot 7 is empty"
expect "dot limit"      tests/carts/dots "limit 100000"
expect "cls is free"    tests/carts/dots "cls 0"
expect "over the limit" tests/carts/dots "full 100000 5 2"
expect "flip resets"    tests/carts/dots "flip 0"
expect "clip counts"    tests/carts/dots "clip 100"
expect "cls ignores limit" tests/carts/dots "cls free 0 4"
expect "dot_limit(n)"   tests/carts/dots "small 5 6 4"
expect "rect_fill_fast" tests/carts/dots "fast 0 4 7 7 4"
expect "dot_limit range" tests/carts/dots "dot_limit: -1 out of range 0-1000000"
expect "bad .g64"       tests/carts/badg64 "tileset.g64: a 10 x 10 tileset has 100 pixels, this one has 3"
LC=$PWD/build/lcarts; rm -rf "$LC" && mkdir -p "$LC" && cp -r tests/carts/legacy "$LC/"
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 legacy 2>&1); said() { printf '%s' "$out" | grep -qF -- "$2" && ok || bad "$1: wanted '$2'" "$out"; }
said "old formats read" 't 48 m -1 5 160x96'
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 --exec "load legacy" --exec save --exec run 2>&1)
said "old formats saved" 'saved tiles/tileset.g64, maps/map.g64, sfx/sfx.g64, music/music.g64'
said "converted cart runs the same" 't 48 m -1 5 160x96'
[ -f "$LC/legacy/tiles.txt.old" ] && [ ! -f "$LC/legacy/tiles.txt" ] && [ -f "$LC/legacy/maps/map.g64" ] && ok || bad "old files become *.old" "$(ls -R "$LC/legacy")"
[ "$(head -1 "$LC/legacy/maps/map.g64")" = 33 ] && ok || bad "an old map is 4 x 4 screens" "$(head -1 "$LC/legacy/maps/map.g64")"
# sound and music that are still text in their folders (sfx/sfx.txt ...): oldsound has seedling's, as they were
cp -r tests/carts/oldsound "$LC/"
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 oldsound 2>&1);  said "old sound text read" 'oldsound 1'
out=$(GOOBERENGINE_CARTS=$LC $GE --headless --frames 10 --exec "load oldsound" --exec save 2>&1)
said "old sound text saved" 'saved sfx/sfx.g64, music/music.g64'
# what seedling's sound and music were when oldsound was made (seedling itself keeps changing)
OE=tests/carts/oldsound-expected
cmp -s $OE/sfx.g64 "$LC/oldsound/sfx/sfx.g64" && ok || bad "old sfx.txt converts exactly" "$(diff $OE/sfx.g64 "$LC/oldsound/sfx/sfx.g64" | head)"
cmp -s $OE/music.g64 "$LC/oldsound/music/music.g64" && ok || bad "old music.txt converts exactly" "$(diff $OE/music.g64 "$LC/oldsound/music/music.g64" | head)"
[ -f "$LC/oldsound/sfx/sfx.txt.old" ] && [ ! -f "$LC/oldsound/sfx/sfx.txt" ] && ok || bad "old sfx.txt becomes sfx.txt.old" "$(ls -R "$LC/oldsound")"

# ---- the code editor, driven by scripted keys in a scratch carts folder ----
TC=$PWD/build/tcarts
edit() {    # edit <name> <keys>: NEW cart "t", type the keys; leaves output in $out, code in $TC/t/main.gb
    rm -rf "$TC" && mkdir -p "$TC"
    out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 400 --exec "new t" --type "$2" 2>&1)
}
has()  { grep -qxF -- "$2" "$TC/t/main.gb" && ok || bad "$1: main.gb has no line '$2'" "$(cat "$TC/t/main.gb")"; }
hasnt() { [ -f "$TC/t/main.gb" ] || { bad "$1: no main.gb"; return; }; grep -qF -- "$2" "$TC/t/main.gb" && bad "$1: main.gb still has '$2'" "$(cat "$TC/t/main.gb")" || ok; }
said() { printf '%s' "$out" | grep -qF -- "$2" && ok || bad "$1: wanted '$2'" "$out"; }
# Switching editors: NEW lands in the code editor, LOAD in the shell. From the shell, the commands
# (tiles{enter} ...). In an editor, a click on its tab at y 5, 13 px in (still on it if two tabs
# before it have their unsaved *): code 23, tiles 53, map 89, sound 113, music 149, font 185,
# each 72 more while the file tree (F9) is shown.

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

# ---- the settings screen (F10, SETTINGS, or the tree's bottom-left button): lines are theme,
# volume, music, sound fx, fullscreen, window size; Left / Right change the one picked ----
ST=$GOOBERENGINE_SETTINGS
sethas() { grep -qxF -- "$2" "$ST" 2>/dev/null && ok || bad "$1: settings.txt has no line '$2'" "$(cat "$ST" 2>&1)"; }
out=$($GE --headless --frames 30 --exec settings --type '{down}{left}{left}' 2>&1); said "settings: volume" '[settings] volume: 8'
sethas "settings kept" 'volume = 8'; sethas "settings: the rest written too" 'music_volume = 10'
out=$($GE --headless --frames 30 --exec settings --type '{right}' 2>&1); sethas "settings: theme" 'theme = green'
sethas "settings: the volume stays" 'volume = 8'
out=$($GE --headless --frames 30 --exec settings --type '{down}{down}{down}{down}{down}{right}' 2>&1); sethas "settings: window size" 'scale = 4'
out=$($GE --headless --frames 30 --exec settings --type '{down}{down}{down}{down}{enter}' 2>&1); sethas "settings: fullscreen" 'fullscreen = yes'
# the volume bar: step i is at x 72 + 86 + 8i (beside the file tree), y 50
out=$($GE --headless --frames 30 --exec settings --type '{click:186,50}' 2>&1); sethas "settings: click the volume bar" 'volume = 4'
out=$($GE --headless --frames 30 --exec "theme orange" 2>&1); sethas "THEME keeps the other settings" 'volume = 4'
printf 'mine = kept\n' >> "$ST"; $GE --headless --frames 30 --exec settings --type '{right}' > /dev/null 2>&1
sethas "settings.txt keeps lines it doesn't know" 'mine = kept'
out=$($GE --headless --frames 60 --type '{f10}{esc}help{enter}' 2>&1); said "F10, Esc: back to the shell" 'RUN [name]'
rm -f "$ST"
out=$($GE --headless --frames 40 --exec "load seedling" --type 'edit{enter}{click:30,235}{down}{left}' 2>&1); said "the tree's settings button" '[settings] volume: 9'
edit "settings: Esc goes back" '{click:95,5}{f10}{esc}{ctrl+end}{enter}zzz{ctrl+s}'; has "settings: Esc goes back" 'zzz'
# ---- help (F1): every keyword, type, command and constant has a page; F1 opens the one under the cursor
out=$($GE --help-check 2>&1) && ok || bad "help: a page for everything" "$out"
edit "help: F1 on a command"  '{ctrl+end}{enter}rect_fill{f1}';      said "help: F1 on a command" '[help] rect, rect_fill'
edit "help: F1 inside a word" '{ctrl+end}{enter}eachin{left}{left}{f1}'; said "help: F1 inside a word" '[help] for ... eachin'
edit "help: F1 on a keyword"  '{ctrl+end}{enter}endif{f1}';          said "help: F1 on a keyword" '[help] if'
edit "help: F1 on a constant" '{ctrl+end}{enter}KEY_SPACE{f1}';      said "help: F1 on a constant" '[help] key_down, key_hit'
edit "help: F1 on a suffix"   '{ctrl+end}{enter}x = left$(a$, 2){left}{left}{left}{left}{left}{left}{left}{left}{f1}'
said "help: F1 on a suffix" '[help] String functions'
edit "help: F1 on a number"   '{ctrl+end}{enter}x = 42{f1}';         said "help: F1 on a number" '[help] Numbers'
edit "help: F1 on a comment"  "{ctrl+end}{enter}' hello{f1}";        said "help: F1 on a comment" '[help] Comments'
edit "help: F1 on own name"   '{ctrl+end}{enter}zzqqx{f1}';          said "help: F1 on own name" "no page for 'zzqqx'"
edit "help: Esc back to the code" '{ctrl+end}{enter}cls{f1}{esc}(){esc}edit{enter}{f1}{esc}x{ctrl+s}'; has "help: Esc back to the code" 'cls()x'
edit "help: F1 in the help is the index" '{ctrl+end}{enter}cls{f1}{f1}';  said "help: F1 in the help is the index" '[help] Index: '
edit "help: F1 twice, back to the page" '{ctrl+end}{enter}cls{f1}{f1}{f1}{esc}y{ctrl+s}'; has "help: F1 twice, back to the page" 'clsy'
edit "help: narrow the index" '{ctrl+end}{enter}cls{f1}{f1}sprite{enter}'; said "help: narrow the index" '[help] sprite'
out=$($GE --headless --frames 30 --exec "help index" 2>&1); said "help: HELP index" '[help] Index: '
# the top bar's buttons: back (x 1-43), forward (45-105), contents (268-322), index (324-360), close (362-398)
went() {    # went <name> <places, | after each>: where the help went, in order
    got=$(printf '%s\n' "$out" | grep '^\[help\]' | sed 's/^\[help\] //; s/: [0-9]* names.*//' | tr '\n' '|')
    [ "$got" = "$2" ] && ok || bad "$1: went $got, wanted $2"
}
out=$($GE --headless --frames 40 --exec "help cls" --type '{tab}{enter}{click:10,5}' 2>&1)
went "help: the back button" 'cls|clip|cls|'
out=$($GE --headless --frames 40 --exec "help cls" --type '{tab}{enter}{click:10,5}{click:60,5}' 2>&1)
went "help: the forward button" 'cls|clip|cls|clip|'
out=$($GE --headless --frames 50 --exec "help cls" --type '{click:340,5}{enter}{click:10,5}{click:10,5}{click:60,5}{click:60,5}' 2>&1)
went "help: the index is a place to go back to" 'cls|Index|abs, sgn, min, max|Index|cls|Index|abs, sgn, min, max|'
out=$($GE --headless --frames 40 --exec "help cls" --type '{click:300,5}{alt+left}{alt+right}' 2>&1)
went "help: contents button, Alt+arrows" 'cls|Contents|cls|Contents|'
out=$($GE --headless --frames 40 --exec "help cls" --type '{tab}{enter}x{esc}{click:10,5}' 2>&1)
went "help: typing goes to the index, back returns" 'cls|clip|clip|cls|'
edit "help: the close button" '{ctrl+end}{enter}cls{f1}{tab}{enter}{click:380,5}y{ctrl+s}'; has "help: the close button" 'clsy'
edit "help: back from the first place leaves" '{ctrl+end}{enter}cls{f1}{click:10,5}y{ctrl+s}'; has "help: back from the first place leaves" 'clsy'
edit "help: follow a link"    '{ctrl+end}{enter}cls{f1}{tab}{enter}'; said "help: follow a link" '[help] clip'
edit "help: back"             '{ctrl+end}{enter}cls{f1}{tab}{enter}{backspace}{backspace}y{ctrl+s}'; has "help: back" 'clsy'
edit "help: search"           '{ctrl+end}{enter}{f1}circle_f{enter}'; said "help: search" '[help] circle, circle_fill'
out=$($GE --headless --frames 30 --type '{f1}' 2>&1);     said "help: F1 in the shell is the index" '[help] Index: 343 names'
out=$($GE --headless --frames 30 --type 'load{f1}' 2>&1); said "help: F1 in the shell narrows to the command" 'first LOAD (shell)'
out=$($GE --headless --frames 30 --type 'load{f1}{enter}' 2>&1); said "help: F1 in the shell, Enter" '[help] LOAD'
out=$($GE --headless --frames 30 --type '{f1}{esc}cls{enter}help{enter}' 2>&1); said "help: Esc from the index, back to the prompt" 'RUN [name]'
out=$($GE --headless --frames 30 --type '{f1}{f1}help{enter}' 2>&1);    said "help: F1 twice, back to the prompt" 'RUN [name]'
out=$($GE --headless --frames 30 --type 'zzqqx{f1}' 2>&1); said "help: F1 in the shell, unknown" "no page names with 'zzqqx'"
out=$($GE --headless --frames 30 --exec "help music" 2>&1); said "help: HELP word" '[help] music'
out=$($GE --headless --frames 30 --exec "help zzqqx" 2>&1); said "help: HELP unknown" "no help for 'zzqqx'"
out=$($GE --headless --frames 40 --exec "load seedling" --type 'tiles{enter}{f1}' 2>&1); said "help: F1 in the tile editor" '[help] Tile editor'

# the indent line (the 7th, third from the bottom): smart, auto, none
out=$($GE --headless --frames 30 --exec settings --type '{up}{up}{up}{right}' 2>&1); said "settings: indent" '[settings] indent: auto'
sethas "settings: indent kept" 'indent = auto'
# the right click line (second from the bottom): the tile editor's right button, eyedrop at first
sethas "settings: right click is eyedrop at first" 'tile_right = eyedrop'
out=$($GE --headless --frames 30 --exec settings --type '{up}{up}{right}' 2>&1); said "settings: right click" '[settings] right click: colour B'
sethas "settings: right click kept" 'tile_right = colour_b'
edit "auto indent: no block indent" '{ctrl+end}{enter}  if 1{enter}x = 1{ctrl+s}'; has "auto indent: no block indent" '  x = 1'
edit "auto indent: backspace undoes Enter" '{ctrl+end}{enter}  abc{enter}{backspace}d{ctrl+s}'; has "auto indent: backspace undoes Enter" '  abcd'
edit "auto indent: then backspace is one space" '{ctrl+end}{enter}    abc{enter}x{backspace}{backspace}y{ctrl+s}'; has "auto indent: then backspace is one space" '   y'
printf 'indent = none\n' > "$ST"
edit "no indent" '{ctrl+end}{enter}  if 1{enter}x = 1{ctrl+s}'; has "no indent" 'x = 1'
rm -f "$ST"
# the effects page (the last line): sliders by key, kept in settings.txt, read back and clamped
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{right}{right}' 2>&1); said "effects: crt by steps" '[settings] crt bend: 10%'
sethas "effects: crt kept" 'fx_crt = 10'
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{down}{left}' 2>&1); said "effects: tint wraps" '[settings] tint: 345 deg'
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{down}{down}{shift+left}' 2>&1); said "effects: fine step" '[settings] saturation: 99%'
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{up}{up}{end}' 2>&1); said "effects: burn tops out" '[settings] screen burn: 80%'
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{up}{enter}' 2>&1); said "effects: reset" '[settings] effects: all off'
sethas "effects: reset kept" 'fx_saturation = 100'
printf 'fx_burn = 200\nfx_tint = 400\n' > "$ST"
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{up}{up}{left}' 2>&1); said "effects: burn clamped on load" '[settings] screen burn: 75%'
out=$($GE --headless --frames 30 --exec settings --type '{up}{right}{down}{shift+right}' 2>&1); said "effects: tint wrapped on load" '[settings] tint: 41 deg'
rm -f "$ST"
if command -v python3 > /dev/null; then      # the levels reach the sound
    printf 'volume = 0\n' > "$ST"; $GE --headless --frames 70 --wav build/quiet.wav tests/carts/tone > /dev/null 2>&1
    got=$(python3 -I tests/wav_check.py build/quiet.wav 0.1 0.45); case "$got" in "peak 0 "*) ok ;; *) bad "volume 0 is silent" "$got" ;; esac
    printf 'sfx_volume = 5\n' > "$ST"; $GE --headless --frames 70 --wav build/half.wav tests/carts/tone > /dev/null 2>&1
    rm -f "$ST"; $GE --headless --frames 70 --wav build/full.wav tests/carts/tone > /dev/null 2>&1
    half=$(python3 -I tests/wav_check.py build/half.wav 0.1 0.45 | cut -d' ' -f2); full=$(python3 -I tests/wav_check.py build/full.wav 0.1 0.45 | cut -d' ' -f2)
    [ $(( half * 2 )) -ge $(( full - 200 )) ] && [ $(( half * 2 )) -le $(( full + 200 )) ] && ok || bad "sfx_volume 5 is half as loud" "half $half full $full"
    printf 'music_volume = 0\n' > "$ST"; $GE --headless --frames 70 --wav build/nomusic.wav tests/carts/tone > /dev/null 2>&1
    got=$(python3 -I tests/wav_check.py build/nomusic.wav 0.1 0.45); case "$got" in *"freq 440"*) ok ;; *) bad "music_volume 0 leaves notes alone" "$got" ;; esac
    $GE --headless --frames 130 --wav build/nomusic2.wav tests/carts/music > /dev/null 2>&1
    got=$(python3 -I tests/wav_check.py build/nomusic2.wav 0.1 0.7); case "$got" in "peak 0 "*) ok ;; *) bad "music_volume 0 silences the music" "$got" ;; esac
    rm -f "$ST"
fi

# ---- the tile editor. {f9} hides the file tree, so the editor has the whole width.
# The tileset is the canvas, at 6x from (0,10), and tile 0 is selected: its pixel (x, y)
# is clicked at (3 + 6x, 13 + 6y). In the tools window, palette colour i is at
# (368 + (i mod 4) * 8, 72 + (i / 4) * 8); the preview window is at (276,12) ----
TT=$TC/t/tiles/tileset.g64
row() {     # row <name> <tile> <row> <expected 10 characters>: from the tileset's rows (one per line)
    got=$(awk -v l=$(( ($2 / 40) * 10 + $3 + 2 )) -v c=$(( ($2 % 40) * 10 + 1 )) 'NR == l { s = substr($0, c, 10); while (length(s) < 10) s = s "."; print s }' "$TT" 2>/dev/null)
    [ "$got" = "$4" ] && ok || bad "$1: tile $2 row $3 is '$got', wanted '$4'" "$(cat "$TT" 2>/dev/null)"
}
edit "draw"      '{click:125,5}{f9}{click:3,13}{ctrl+s}';                       row "draw: tile 0 is selected at first" 0 0 '*.........'
edit "flip"      '{click:125,5}{f9}{click:3,13}{h}{ctrl+s}';                    row "flip" 0 0 '.........*'
edit "turn"      '{click:125,5}{f9}{click:3,13}{t}{ctrl+s}';                    row "turn" 0 0 '.........*'
edit "fill"      '{click:125,5}{f9}{f}{click:3,13}{ctrl+s}';                    row "fill" 0 9 '**********'
row "fill stays in the tile" 1 0 '..........'
edit "line"      '{click:125,5}{f9}{l}{click:3,19}{drag:57,19}{mouse:57,19}{ctrl+s}'; row "line" 0 1 '**********'
row "line" 0 0 '..........'
edit "stroke stops at the edge" '{click:125,5}{f9}{click:3,19}{drag:117,19}{mouse:117,19}{ctrl+s}'
row "stroke stops at the edge" 0 1 '**********'; row "stroke stops at the edge" 1 1 '..........'
edit "tile undo" '{click:125,5}{f9}{click:3,13}{click:15,13}{ctrl+z}{ctrl+s}';   row "tile undo" 0 0 '*.........'
edit "paste"     '{click:125,5}{f9}{click:3,13}{ctrl+c}{right}{ctrl+v}{ctrl+s}'; row "paste" 1 0 '*.........'
edit "colour"    '{click:125,5}{f9}{click:368,104}{click:3,13}{ctrl+s}';         row "colour" 0 0 'g.........'
edit "right click picks" '{click:125,5}{f9}{click:368,104}{click:3,13}{click:368,72}{rclick:3,13}{mouse:3,13}{click:9,13}{ctrl+s}'
row "right click picks" 0 0 'gg........'
edit "middle click picks" '{click:125,5}{f9}{click:3,13}{click:368,104}{mclick:3,13}{mouse:3,13}{click:9,13}{ctrl+s}'
row "middle click picks" 0 0 '**........'
edit "far tile"  '{click:125,5}{f9}{down}{down}{down}{right}{right}{click:123,173}{ctrl+s}'; row "far tile" 122 0 '*.........'
[ "$(head -1 "$TT")" = 23 ] && ok || bad "a tileset is saved only as big as it's used" "$(head -1 "$TT")"
edit "select on sheet" '{click:125,5}{f9}{click:185,15}{click:183,13}{ctrl+s}';  row "select on sheet" 3 0 '*.........'
row "selecting doesn't paint" 3 1 '..........'
edit "preview is only shown" '{click:125,5}{f9}{click:3,13}{click:314,50}{ctrl+s}'; row "preview is only shown" 0 0 '*.........'
row "preview is only shown" 0 4 '..........'
edit "move window" '{click:125,5}{f9}{click:300,16}{drag:100,120}{mouse:100,120}{click:263,13}{click:261,13}{ctrl+s}'
row "move window: the sheet under it" 4 0 '...*......'
edit "tiles unsaved" '{click:125,5}{f9}{click:3,13}{esc}new t2{enter}';       said "tiles unsaved" 'unsaved changes in the tiles'

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
edit "map pen"    '{click:161,5}{f9}{click:5,15}{drag:25,15}{mouse:25,15}{ctrl+s}';           maprow "map pen: tile 0 at first" 0 "00 00 00"
edit "map rect"   '{click:161,5}{f9}{r}{click:15,25}{drag:35,35}{mouse:35,35}{ctrl+s}'
maprow "map rect" 1 ".. 00 00 00"; maprow "map rect" 2 ".. 00 00 00"
edit "map fill"   '{click:161,5}{f9}{f}{click:55,55}{ctrl+s}';                                maprow "map fill" 95 "$(printf '00 %.0s' $(seq 159))00"
edit "map erase"  '{click:161,5}{f9}{click:5,15}{e}{click:5,15}{click:15,15}{ctrl+s}';        maprow "map erase" 0 ""
edit "map picker" '{click:161,5}{f9}{tab}{click:47,25}{click:5,15}{ctrl+s}';                  maprow "map picker" 0 "04"
edit "map block"  '{click:161,5}{f9}{tab}{click:17,25}{drag:27,35}{mouse:27,35}{click:5,15}{ctrl+s}'
maprow "map block" 0 "01 02"; maprow "map block" 1 "0F 0G"
edit "map tile 0" '{click:161,5}{f9}{]}{[}{click:5,15}{ctrl+s}';                              maprow "map tile 0" 0 "00"
edit "map pan"    '{click:161,5}{f9}{right}{right}{click:5,15}{ctrl+s}';                      maprow "map pan" 0 ".. .. 00"
edit "map undo"   '{click:161,5}{f9}{click:5,15}{click:15,15}{ctrl+z}{ctrl+s}';               maprow "map undo" 0 "00"
edit "map resize" '{click:161,5}{f9}{ctrl+right}{ctrl+down}{ctrl+down}{ctrl+up}{ctrl+s}'
[ "$(head -1 "$MT")" = 44 ] && [ "$(sed -n 2p "$MT" | wc -c)" = 401 ] && ok || bad "map resize: wanted 5 x 5 screens" "$(head -c 300 "$MT")"
edit "map unsaved" '{click:161,5}{f9}{click:5,15}{esc}new t2{enter}';                         said "map unsaved" 'unsaved changes in the map'
# the tile editor's selection is the map's pen, and a tile picked in the map is selected there
edit "tiles pen"  '{click:125,5}{f9}{click:185,15}{click:89,5}{click:5,15}{ctrl+s}';                  maprow "tiles pen: tile 3 from the tile editor" 0 "03"
edit "tiles block pen" '{click:125,5}{f9}{right}{z}{click:89,5}{click:5,15}{ctrl+s}'
maprow "tiles block pen: a 2x2 stamp" 0 "01 02"; maprow "tiles block pen: a 2x2 stamp" 1 "0F 0G"
edit "tiles block drag" '{click:125,5}{f9}{right}{z}{click:89,5}{click:5,15}{drag:45,15}{mouse:45,15}{ctrl+s}'
maprow "tiles block drag: steps a block at a time" 0 "01 02 01 02 01 02"; maprow "tiles block drag" 1 "0F 0G 0F 0G 0F 0G"
edit "map picks for tiles" '{click:161,5}{f9}{tab}{click:15,35}{mouse:15,35}{click:53,5}{click:63,73}{ctrl+s}';    row "map picks for tiles: tile 41 is selected" 41 0 '*.........'

# with Seedling's map: an unchanged save is byte-for-byte the same; right-click picks;
# select + copy makes a brush; a run sees unsaved edits
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
echo 'print("c " + str(map_get(0, 0)))' > "$TC/s/main.gb"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 30 --exec "load s" --type 'map{enter}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/maps/map.g64 "$TC/s/maps/map.g64" && ok || bad "map round trip" "$(diff carts/seedling/maps/map.g64 "$TC/s/maps/map.g64" | head)"
MT=$TC/s/maps/map.g64
cell() { awk -v l=$(( $2 + 2 )) -v c=$(( $1 * 2 + 1 )) 'NR == l { print substr($0, c, 2) }' "$MT"; }
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load s" \
      --type 'map{enter}{f9}{rclick:5,15}{click:55,65}{s}{click:5,15}{drag:25,25}{mouse:25,25}{ctrl+c}{click:105,115}{ctrl+s}' 2>&1)
got=$(cell 5 5); [ "$got" = 03 ] && ok || bad "map right-click pick: cell 5,5 is '$got'"
got="$(cell 10 10) $(cell 11 10) $(cell 12 10)"; [ "$got" = "03 03 03" ] && ok || bad "map copy brush: row 10 cells 10-12 are '$got'"
got=$(cell 10 11); [ "$got" = 03 ] && ok || bad "map copy brush: cell 10,11 is '$got'"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load s" --type 'map{enter}{f9}{e}{click:5,15}{ctrl+r}' 2>&1)
said "map run unsaved" 'c -1'

# ---- the file tree (shown by default, 72 px wide): rows from y 11, 10 px each, with a 1 px
# separator above every section but the first:
# code, main.gb, tiles, tileset, maps, map, sfx, sfx, music, music. + is at x 66 ----
rm -rf "$TC" && mkdir -p "$TC"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "new t" --type '{click:161,5}{click:66,57}{click:77,15}{ctrl+s}' 2>&1)
[ -f "$TC/t/maps/map-1.g64" ] && [ ! -f "$TC/t/maps/map.g64" ] && ok || bad "tree +: a new map" "$out$(ls -R "$TC/t")"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "load t" --type 'edit{enter}{click:66,15}x = 2{ctrl+s}{click:20,25}' 2>&1)
grep -qxF 'x = 2' "$TC/t/code-1.gb" 2>/dev/null && ok || bad "tree +: a new .gb file" "$out"
said "tree: switch file" '[editor] saved code-1 ('
# a new .gb file isn't on disk until saved: left blank, it keeps its row (code, main.gb, code-1)
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "new u" --type '{click:66,15}{click:20,25}{click:20,35}y = 3{ctrl+s}' 2>&1)
grep -qxF 'y = 3' "$TC/u/code-1.gb" 2>/dev/null && ok || bad "tree: a blank new .gb file stays listed" "$out$(ls "$TC/u")"
printf 'include "lib.gb"\nboom()\n' > "$TC/t/main.gb"; printf 'function boom()\n  local a:int[2]\n  a[5] = 1\nendfunction\n' > "$TC/t/lib.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load t" --type 'edit{enter}{ctrl+r}' 2>&1)
said "error in an include opens it" '[editor] line 3: Array index 5'
printf 'local function boom()\nendfunction\n' > "$TC/t/lib.gb"     # a local function belongs to its file
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load t" --type 'edit{enter}{ctrl+r}' 2>&1)
said "another file's function is private" "[editor] line 2: 'boom' is a local function of lib.gb, and only that file can call it"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 60 --exec "load t" --type 'tiles{enter}{click:30,87}{ctrl+s}' 2>&1)
said "tree: open a map from the tile editor" '[map] saved maps/map-1.g64'
# folding: a click on a section's name folds its files away (rows move up), again brings them back
edit "tree fold"     '{click:20,15}{click:30,36}{ctrl+s}';               said "tree fold: the rows move up" '[tiles] saved tiles/tileset.g64'
edit "tree unfold"   '{click:20,15}{click:20,15}{click:30,25}{ctrl+end}{enter}zzz{ctrl+s}'; has "tree unfold" 'zzz'
edit "tree + unfolds" '{click:161,5}{click:20,57}{click:66,57}{ctrl+s}'
[ -f "$TC/t/maps/map-1.g64" ] && ok || bad "tree + on a folded section: a new map" "$out$(ls -R "$TC/t")"
# right-click a file: rename (type, Enter) or delete (y). Rows here: code, main.gb, extra.gb,
# tiles, tileset (slot 0), tiles-1, maps, map (slot 0), map-1 (from y 11, 10 px each, 1 px more above a section); the menu's
# two lines follow the row
rm -rf "$TC" && mkdir -p "$TC" && cp -r tests/carts/slots "$TC/s" && printf 'x = 1\n' > "$TC/s/extra.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "load s" \
      --type 'tiles{enter}{rclick:30,56}{click:30,66}{backspace}{backspace}{backspace}cave{enter}{click:95,5}{rclick:30,35}{click:30,45}{backspace}{backspace}{backspace}{backspace}{backspace}lib{enter}{rclick:30,97}{click:30,117}y{rclick:30,25}' 2>&1)
said "tree rename" 'renamed tileset to tilecave'
[ -f "$TC/s/tiles/tilecave.g64" ] && [ ! -f "$TC/s/tiles/tileset.g64" ] && ok || bad "tree rename: the file moves" "$(ls -R "$TC/s")"
grep -qxF 'slot tiles 0 = tilecave' "$TC/s/cart.txt" 2>/dev/null && ok || bad "tree rename: the slot goes with the file" "$(cat "$TC/s/cart.txt" 2>&1)"
said "tree rename code" 'renamed extra to lib'
[ -f "$TC/s/lib.gb" ] && [ ! -f "$TC/s/extra.gb" ] && ok || bad "tree rename code: the file moves" "$(ls "$TC/s")"
said "tree delete" 'deleted map-1'
[ ! -f "$TC/s/maps/map-1.g64" ] && [ -f "$TC/s/maps/map.g64" ] && ok || bad "tree delete: only map-1 goes" "$(ls -R "$TC/s")"
said "main stays" 'main stays'
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 100 --exec "load s" --type 'map{enter}{rclick:30,87}{click:30,107}n' 2>&1)
[ -f "$TC/s/maps/map.g64" ] && ok || bad "tree delete: n keeps the file" "$out"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 100 --exec "load s" --type 'tiles{enter}{rclick:30,66}{click:30,76}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}{backspace}tilecave{enter}' 2>&1)
said "tree rename: a name in use" 'tiles/tilecave.g64 already exists'

# slots: the number left of a file (x < 8) is the slot it loads into when the game runs.
# A click puts it in the lowest free slot or takes it out; a right-click moves it to the next
# one, swapping with the file there. cart.txt keeps them (a slot changing says nothing: the column shows it).
slotted() { grep -qxF -- "$2" "$TC/s/cart.txt" 2>/dev/null && ok || bad "$1: cart.txt has no line '$2'" "$out$(cat "$TC/s/cart.txt" 2>&1)"; }
rm -rf "$TC" && mkdir -p "$TC" && cp -r tests/carts/slots "$TC/s" && printf 'x = 1\n' > "$TC/s/extra.gb"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 200 --exec "load s" \
      --type 'edit{enter}{click:3,35}{click:3,66}{rclick:3,66}{click:3,56}{click:3,97}{rclick:3,87}{click:3,25}' 2>&1)
slotted "slot: code"                  'slot code 0 = extra'
slotted "slot: a right-click moves on" 'slot tiles 2 = tiles-1'
grep -qF 'tileset' "$TC/s/cart.txt" && bad "slot: tileset is out" "$(cat "$TC/s/cart.txt")" || ok
slotted "slot: a right-click swaps"   'slot maps 1 = map'
slotted "slot: ...with the one there" 'slot maps 0 = map-1'
printf '%s' "$out" | grep -q '\[tree\].*main' && bad "slot: a click on main's dash says nothing" "$out" || ok
grep -qxF 'title = s' "$TC/s/cart.txt" && ok || bad "slot: a cart.txt is made with a title" "$(cat "$TC/s/cart.txt")"
printf 'title = keep me\n# a comment\nslot tiles 4 = tiles-1\n' > "$TC/s/cart.txt"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 100 --exec "load s" --type 'tiles{enter}{click:3,56}' 2>&1)
grep -qF 'tiles-1' "$TC/s/cart.txt" && bad "slot: slotted files come first (the click took tiles-1 out)" "$(cat "$TC/s/cart.txt")" || ok
slotted "slot: every slot empty"          'slots = none'
slotted "slot: cart.txt keeps its lines"  '# a comment'
slotted "slot: ...and its title"          'title = keep me'
# running: slotted files are loaded before the first line; slotted code is compiled in ahead of main.gb
expect "autoload"          tests/carts/autoload "auto -1 937 80x24 5 42 lib,main"
expect "autoload: bad slot" tests/carts/autoload "map slot 5: no maps/gone.g64"

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
# sfx/sfx.g64 is "f" + (sound effects - 1), 16 wave lines, then one line per sfx:
# speed-1 length-1 loop loop, then 32 steps of pitch wave volume effect (C-4 = o, so a rest is o000)
ST=$TC/t/sfx/sfx.g64
sfxhas() { grep -q -- "$2" "$ST" 2>/dev/null && ok || bad "$1: sfx/sfx.g64 has no '$2'" "$(cat "$ST" 2>/dev/null)"; }
edit "notes"       '{click:185,5}{f9}zx{ctrl+s}';                                 sfxhas "notes" '^7v00o050q050o000'
edit "volume"      '{click:185,5}{f9}z{left}7{ctrl+s}';                           sfxhas "volume" '^7v00o070'
edit "effect"      '{click:185,5}{f9}z{left}{tab}{ctrl+s}';                       sfxhas "effect" '^7v00o051'
edit "octave"      '{click:185,5}{f9}{]}z{ctrl+s}';                               sfxhas "octave" '^7v00A050'
edit "wave pen"    '{click:185,5}{f9}.z{ctrl+s}';                                 sfxhas "wave pen" '^7v00o150'
edit "length"      '{click:185,5}{f9}zxc{left}{ctrl+l}{ctrl+s}';                  sfxhas "length" '^7200o050q050s050'
edit "loop"        '{click:185,5}{f9}z{ctrl+b}{right}{right}{ctrl+e}{ctrl+s}';    sfxhas "loop" '^7v14o050'
edit "speed"       '{click:185,5}{f9}z{-}{-}{ctrl+s}';                            sfxhas "speed" '^5v00o050'
edit "next sfx"    '{click:185,5}{f9}{pgdn}z{ctrl+s}';                            sfxhas "next sfx" '^f1$'; sfxhas "next sfx 2" '^7v00o050'
edit "draw pitch"  '{click:185,5}{f9}{click:71,57}{ctrl+s}';                      sfxhas "draw pitch" '^7v00u050'
edit "draw wave"   '{click:185,5}{f9}{click:67,223}{ctrl+s}';                     sfxhas "draw wave" '^0fffffffffffffff0000000000000000$'
edit "sound undo"  '{click:185,5}{f9}z{ctrl+z}{ctrl+s}'
[ "$(sed -n 1p "$ST") $(sed -n 18p "$ST")" = "f0 7v00$(printf 'o000%.0s' $(seq 32))" ] && ok || bad "sound undo: sfx.g64 still has an sfx" "$(cat "$ST")"
edit "sound unsaved" '{click:185,5}{f9}z{esc}new t2{enter}';                      said "sound unsaved" 'unsaved changes in the sound'
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 20 --exec "load s" --type 'sound{enter}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/sfx/sfx.g64 "$TC/s/sfx/sfx.g64" && ok || bad "sfx round trip" "$(diff carts/seedling/sfx/sfx.g64 "$TC/s/sfx/sfx.g64" | head)"

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
# music/music.g64 is speed-1, order entries-1, patterns-1, tracks-1, then the order (00 = pattern 0,
# 10 = jump to entry 0, .. = stop), then 32 rows a pattern: pitch wave volume effect, .... empty, ==== off
MU=$TC/t/music/music.g64
mushas() { grep -qxF -- "$2" "$MU" 2>/dev/null && ok || bad "$1: music.g64 has no line '$2'" "$(head -12 "$MU" 2>/dev/null)"; }
muhead() { [ "$(head -2 "$MU" 2>/dev/null | tr '\n' ' ')" = "$2" ] && ok || bad "$1: music.g64 starts '$(head -2 "$MU" 2>/dev/null | tr '\n' ' ')', wanted '$2'"; }
edit "mus note"    '{click:221,5}{f9}z{ctrl+s}';                               mushas "mus note" 'o050'
muhead "mus tracks" '7000 00 '
edit "mus wave"    '{click:221,5}{f9}z{up}{right}n{ctrl+s}';                   mushas "mus wave" 'og50'
edit "mus vol"     '{click:221,5}{f9}z{up}{right}{right}7{ctrl+s}';            mushas "mus vol" 'o070'
edit "mus fx"      '{click:221,5}{f9}z{up}{right}{right}{right}v{ctrl+s}';     mushas "mus fx" 'o052'
edit "mus off"     '{click:221,5}{f9}1{ctrl+s}';                               mushas "mus off" '===='
edit "mus track"   '{click:221,5}{f9}{tab}q{ctrl+s}';                          mushas "mus track" '....A050'
edit "mus order"   '{click:221,5}{f9}{ctrl+right}{ctrl+up}{ctrl+up}z{ctrl+s}'; muhead "mus order" '7110 0001 '
[ "$(sed -n 37p "$MU")" = o050 ] && ok || bad "mus order pattern: pattern 1 row 0 is '$(sed -n 37p "$MU")'"
edit "mus jump"    '{click:221,5}{f9}{ctrl+right}{ctrl+j}{ctrl+s}';            muhead "mus jump" '7100 0010 '
edit "mus speed"   '{click:221,5}{f9}-{ctrl+s}';                               muhead "mus speed" '6000 00 '
edit "mus undo"    '{click:221,5}{f9}z{ctrl+z}{ctrl+s}'
[ "$(sed 1,2d "$MU" | grep -vc '^\.*$')" = 0 ] && ok || bad "mus undo: music.g64 still has notes" "$(cat "$MU")"
# the piano roll (Ctrl+P), tree hidden: columns 10 px from x 66 (after the pattern list), a pitch every 6 px up from y 224;
# with no notes it opens on pitches 14-45, so C-4 (24, 'o') is the lane around y 161
edit "roll draw"   '{click:221,5}{f9}{ctrl+p}{click:71,161}{drag:101,161}{ctrl+s}'
[ "$(sed -n 4p "$MU") $(sed -n 8p "$MU")" = "o050 ====" ] && ok || bad "roll draw: rows 0 and 4 are '$(sed -n 4p "$MU") $(sed -n 8p "$MU")'" "$(head -10 "$MU")"
[ "$(sed -n 5,7p "$MU" | grep -vc '^\.*$')" = 0 ] && ok || bad "roll draw: rows 1-3 carry the note on" "$(head -10 "$MU")"
edit "roll pitch"  '{click:221,5}{f9}{ctrl+p}{click:71,155}{ctrl+s}';          mushas "roll pitch" 'p050'
edit "roll erase"  '{click:221,5}{f9}{ctrl+p}{click:71,161}{drag:101,161}{rclick:91,100}{ctrl+s}'
[ "$(sed 1,2d "$MU" | grep -vc '^\.*$')" = 0 ] && ok || bad "roll erase: music.g64 still has notes" "$(head -10 "$MU")"
edit "roll undo"   '{click:221,5}{f9}{ctrl+p}{click:71,161}{drag:101,161}{ctrl+z}{ctrl+s}'
[ "$(sed 1,2d "$MU" | grep -vc '^\.*$')" = 0 ] && ok || bad "roll undo: one stroke, one undo" "$(head -10 "$MU")"
edit "roll keys"   '{click:221,5}{f9}{ctrl+p}{right}{right}c{ctrl+s}';         [ "$(sed -n 6p "$MU")" = s050 ] && ok || bad "roll keys: row 2 is '$(sed -n 6p "$MU")'"
edit "mus unsaved" '{click:221,5}{f9}z{esc}new t2{enter}';                     said "mus unsaved" 'unsaved changes in the music'
rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/seedling "$TC/s"
GOOBERENGINE_CARTS=$TC $GE --headless --frames 20 --exec "load s" --type 'music{enter}{ctrl+s}' > /dev/null 2>&1
cmp -s carts/seedling/music/music.g64 "$TC/s/music/music.g64" && ok || bad "music round trip" "$(diff carts/seedling/music/music.g64 "$TC/s/music/music.g64" | head)"

# ---- fonts: a cart's text() uses its font slot; fonts/font.g64 is slot 0 ----
out=$($GE --headless --frames 8 tests/carts/font 2>&1)
said "own font" 'own 63 63 0 5x10'; said "swap_font: a 6 x 12 font" 'alt 63 0 63 1 6x12 18'
said "empty font slot: the built-in font" 'empty 0 63 6'; said "swap_font: empty slot" 'swap_font: slot 2 is empty'
rm -rf build/badfont && cp -r tests/carts/font build/badfont && sed -i 1s/49/kk/ build/badfont/fonts/font.g64
out=$($GE --headless --frames 5 build/badfont 2>&1); said "font too big" 'font.g64: is 21 x 21; a font is 4 x 4 to 20 x 20'
rm -f build/fontcart.zip; $GE --headless --frames 5 --exec "load tests/carts/font" --exec "export $PWD/build/fontcart.zip" > /dev/null 2>&1
unzip -l build/fontcart.zip 2>/dev/null | grep -q ' font/fonts/alt.g64$' && ok || bad "an exported .zip carries the cart's fonts" "$(unzip -l build/fontcart.zip 2>&1)"

# ---- the font editor. With the tree hidden, a 6 x 10 font's canvas is 12 px a pixel from (325,13):
# pixel (x, y) of the cell is clicked at (331 + 12x, 19 + 12y). Character c is line c - 30 of the file,
# one character a row: its top bit (32, "w") is the leftmost pixel ----
FT=$TC/t/fonts/font.g64
glyph() { got=$(sed -n "$(( $2 - 30 ))p" "$FT" 2>/dev/null); [ "$got" = "$3" ] && ok || bad "$1: character $2 is '$got', wanted '$3'" "$(head -3 "$FT" 2>/dev/null)"; }
edit "font default"   '{click:257,5}{ctrl+s}';                                    glyph "font default" 97 000s2uyu00
[ "$(head -1 "$FT")" = 59 ] && [ "$(wc -l < "$FT")" = 97 ] && ok || bad "the built-in font is 59 then 96 lines" "$(head -2 "$FT")"
edit "font draw"      '{click:257,5}{f9}a{del}{click:331,19}{ctrl+s}';            glyph "font draw" 97 w000000000
edit "font erase"     '{click:257,5}{f9}a{rclick:343,55}{ctrl+s}';                glyph "font erase" 97 000c2uyu00
edit "font padding is drawable" '{click:257,5}{f9}a{del}{click:391,127}{ctrl+s}'; glyph "font padding is drawable" 97 0000000001
edit "font stroke"    '{click:257,5}{f9}z{del}{click:331,19}{drag:331,127}{mouse:331,127}{ctrl+s}'; glyph "font stroke" 122 wwwwwwwwww
edit "font type picks" '{click:257,5}{f9}!{del}{ctrl+s}';                         glyph "font type picks" 33 0000000000
edit "font grid"      '{click:257,5}{f9}{click:19,23}{del}{ctrl+s}';               glyph "font grid" 33 0000000000
edit "font built-in"  '{click:257,5}{f9}a{del}{ctrl+d}{ctrl+s}';                  glyph "font built-in" 97 000s2uyu00
edit "font copy"      '{click:257,5}{f9}b{ctrl+c}a{ctrl+v}{ctrl+s}';               glyph "font copy" 97 0wwYyyyY00
edit "font nudge"     '{click:257,5}{f9}a{shift+right}{ctrl+s}';                  glyph "font nudge" 97 000e1fhf00
edit "font undo"      '{click:257,5}{f9}a{del}{ctrl+z}{ctrl+s}';                  glyph "font undo" 97 000s2uyu00
edit "font size"      '{click:257,5}{f9}{ctrl+right}{ctrl+right}{ctrl+down}{ctrl+s}'
[ "$(head -1 "$FT")" = 7a ] && ok || bad "font size: 8 x 11 is '7a'" "$(head -1 "$FT")"
glyph "font size keeps the glyphs (8 wide: two characters a row)" 97 000000s020u0y0u0000000
edit "font smaller"   '{click:257,5}{f9}{ctrl+left}{ctrl+left}{ctrl+left}{ctrl+up}{ctrl+up}{ctrl+up}{ctrl+up}{ctrl+up}{ctrl+up}{ctrl+up}{ctrl+s}'
[ "$(head -1 "$FT")" = 33 ] && ok || bad "font size stops at 4 x 4" "$(head -1 "$FT")"
said "font size limit" 'a font is 4 x 4 to 20 x 20'
glyph "font smaller cuts the glyphs" 97 000s
edit "font size undo" '{click:257,5}{f9}{ctrl+right}{ctrl+z}{ctrl+s}'
[ "$(head -1 "$FT")" = 59 ] && ok || bad "font size undo" "$(head -1 "$FT")"
edit "font unsaved"   '{click:257,5}{f9}a{del}{esc}new t2{enter}';                said "font unsaved" 'unsaved changes in the font'
edit "font run"       '{click:257,5}{f9}a{del}{ctrl+r}';                          said "font run" 't stopped'

# ---- polish: pal, save data, BROWSE, EXPORT / IMPORT ----
out=$($GE --headless --frames 5 tests/carts/pal 2>&1);           said "pal" 'pal 3 3 48'
# ---- depth, blending, spray, textured quads ----
out=$($GE --headless --frames 5 tests/carts/depth 2>&1)
said "blend modes"      'blend 56 32 45 37 26 5'
said "depth buffer"     'depth 1 1 1 1 1 1'
said "spray"            'spray 1 0 1'
said "tex_quad"         'quad 256 9 0'
said "tex_quad_3d"      'quad3d 5 5 9 5'
SV=$PWD/build/saves; rm -rf "$SV"
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 tests/carts/savedata 2>&1)
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 tests/carts/savedata 2>&1)
said "save data" 'runs 2 note two|lines ~ tilde missing []'
printf 'save_data("bad key", "x")\n' > build/bad_sfx/main.gb
out=$(GOOBERENGINE_SAVES=$SV $GE --headless --frames 5 build/bad_sfx 2>&1); said "save key" "save_data: key 'bad key' must be"

rm -rf "$TC" && mkdir -p "$TC" && cp -r carts/hello "$TC/"
WAIT=$(printf '{mouse:0,0}%.0s' 1 2 3 4 5)
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 30 --type "${WAIT}{f8}" hello 2>&1)
[ ! -e "$TC/hello/label.txt" ] && ok || bad "F8 makes no label (there are no labels)"
cp -r carts/seedling "$TC/"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 40 --exec browse --type '{down}{enter}{esc}' 2>&1)
said "browse runs" 'Seedling stopped'

# export, then import under another name: every file comes back the same
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "load seedling" --exec export 2>&1)
said "export" "wrote $TC/seedling.zip"
unzip -l "$TC/seedling.zip" | grep -q ' seedling/main.gb$' && ok || bad "EXPORT zips the cart folder (seedling/main.gb ...)" "$(unzip -l "$TC/seedling.zip" 2>&1)"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/seedling.zip copy" 2>&1)
said "import" "loaded Seedling"
diff -r -x '.*' -x '*.unsaved' carts/seedling "$TC/copy" > /dev/null && ok || bad "import gives back every file unchanged" "$(diff -r -x '.*' -x '*.unsaved' carts/seedling "$TC/copy" | head)"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/seedling.zip copy" 2>&1)
said "import refuses a taken name" "cart 'copy' already exists"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "load seedling" --exec "export $TC/named" --exec "export $TC/x.gbcart" 2>&1)
said "export adds .zip" "wrote $TC/named.zip"; said "no more .gbcart" "carts are shared as .zip now"
[ ! -e "$TC/x.gbcart" ] && [ ! -e "$TC/x.gbcart.zip" ] && ok || bad "EXPORT x.gbcart wrote a file"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/x.gbcart" 2>&1); said "import refuses .gbcart" "IMPORT takes a .zip"
head -c 2000 "$TC/seedling.zip" > "$TC/cut.zip"
out=$(GOOBERENGINE_CARTS=$TC $GE --headless --frames 5 --exec "import $TC/cut.zip" 2>&1)
said "cut-short zip" 'not a zip file'; [ ! -e "$TC/cut" ] && ok || bad "a bad zip leaves no folder behind"

# ---- zipped carts: .zip / .tar.gz / .tar, unpacked into a folder on LOAD ----
ZC=$PWD/build/zcarts; rm -rf "$ZC" && mkdir -p "$ZC" && cp -r carts/seedling "$ZC/"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "load seedling" --exec export --exec "export $ZC/seed2.tar.gz" 2>&1)
said "export .zip by default" "wrote $ZC/seedling.zip"
said "export .tar.gz" "wrote $ZC/seed2.tar.gz"
unzip -tq "$ZC/seedling.zip" > /dev/null 2>&1 && ok || bad "unzip reads the exported .zip" "$(unzip -t "$ZC/seedling.zip" 2>&1 | tail -3)"
[ "$(wc -c < "$ZC/seedling.zip")" -lt "$(cat carts/seedling/*.gb carts/seedling/*/*.g64 | wc -c)" ] && ok || bad "the .zip is compressed"
rm -rf "$ZC/x" && mkdir "$ZC/x" && tar xzf "$ZC/seed2.tar.gz" -C "$ZC/x" && diff -r -x '.*' -x '*.unsaved' carts/seedling "$ZC/x/seedling" > /dev/null && ok || bad "tar reads the exported .tar.gz"
rm -rf "$ZC/x" "$ZC/seedling"
(cd carts && zip -qr9 "$ZC/rocket.zip" seedling && tar cf "$ZC/plain.tar" seedling)   # made by other tools, cart in a folder
python3 -I -c '
import zipfile, sys
with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("main.gb", "print(\"evil ran\")\n"); z.writestr("../escape.gb", "x"); z.writestr("tiles/../../up.gb", "x")
' "$ZC/evil.zip"
printf 'PK\003\004 junk' > "$ZC/junk.zip"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec dir 2>&1)
said "dir lists a zip by its title" "rocket.zip  Seedling 1 by GooberEngine examples"
said "dir: a bad zip" "junk.zip  (not a cart)"
grep -q "rocket.zip	.*	1	Seedling	" "$GOOBERENGINE_CACHE" && ok || bad "the stub is cached" "$(cat "$GOOBERENGINE_CACHE")"
sed -i 's/	Seedling	/	Cached	/' "$GOOBERENGINE_CACHE"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec dir 2>&1); said "dir reads the cache" "rocket.zip  Cached 1"
touch -d 2020-01-01 "$ZC/rocket.zip"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec dir 2>&1); said "a changed zip is read again" "rocket.zip  Seedling 1"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "load rocket" 2>&1)
said "load a name finds its .zip" "unpacked $ZC/rocket"; said "load a .zip" "loaded Seedling"
diff -r -x '.*' -x '*.unsaved' carts/seedling "$ZC/rocket" > /dev/null && ok || bad "a .zip unpacks every file unchanged" "$(diff -r -x '.*' -x '*.unsaved' carts/seedling "$ZC/rocket" | head)"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "load rocket.zip" 2>&1); said "load an unpacked .zip" "rocket.zip is unpacked already: loading rocket/"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec dir 2>&1)
printf '%s' "$out" | grep -q "rocket.zip" && bad "dir hides an unpacked .zip" "$out" || ok
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "load plain.tar" 2>&1); said "load a .tar" "loaded Seedling"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "import $ZC/seed2.tar.gz other" 2>&1); said "import a .tar.gz under a name" "unpacked $ZC/other"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "run evil" 2>&1); said "a zip's cart runs" "evil ran"
[ ! -e "$PWD/build/escape.gb" ] && [ ! -e "$ZC/escape.gb" ] && [ ! -e "$ZC/up.gb" ] && [ "$(ls "$ZC/evil")" = "main.gb" ] && ok || bad "a zip can't write outside its cart folder" "$(ls -R "$ZC/evil")"
out=$(GOOBERENGINE_CARTS=$ZC $GE --headless --frames 5 --exec "load junk.zip" 2>&1)
said "a bad zip" "not a zip file"; [ ! -e "$ZC/junk" ] && ok || bad "a bad zip leaves no folder behind"

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
    [ "$(tail -c 8 "$EX/seedling-linux-x86_64")" = GOOBERZP ] && ok || bad "linux game ends in the packed-cart trailer"
    [ "$(tail -c 8 "$EX/seedling-windows.exe")" = GOOBERZP ] && ok || bad "windows game ends in the packed-cart trailer"
    out=$("$EX/seedling-linux-x86_64" --headless --frames 60 --shot "$EX/game.png" 2>&1) && [ -s "$EX/game.png" ] && ok || bad "exported game runs" "$out"
    $GE --headless --frames 60 --shot "$EX/engine.png" seedling >/dev/null 2>&1
    cmp -s "$EX/game.png" "$EX/engine.png" && ok || bad "exported game draws what the engine draws"
    grep -q 'data-name="seedling"' "$EX/seedling-web/index.html" && ! grep -q '__GE_' "$EX/seedling-web/index.html" && ok || bad "web index.html has the cart in it"
    [ -s "$EX/seedling-web/player.wasm" ] && [ -s "$EX/seedling-web.zip" ] && ok || bad "web export has player.wasm and a .zip"
    out=$(runtimes/player-linux-x86_64 --headless 2>&1); said "a bare runtime says what it is" 'no game packed'
    out=$($GE --export linux tests/carts/runtime_error "$EX" 2>&1)
    out=$("$EX/runtime_error-linux-x86_64" --headless --frames 20 2>&1); said "game error reaches the player" 'runtime error:'
    out=$($GE --headless --frames 5 --exec "load hello" --exec "export $EX/hello.zip" 2>&1)
    out=$(runtimes/player-linux-x86_64 --headless --frames 20 --cart "$EX/hello.zip" 2>&1) && ok || bad "player --cart" "$out"
    grep -q 'cartName + ".zip"' "$EX/seedling-web/index.html" && ok || bad "the web game unpacks a .zip"
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
ut 'edit{enter}{ctrl+end}{enter}x = 1{ctrl+s}';        said "untitled ctrl+s" 'type SAVE name in the shell'
[ -z "$(ls "$UT")" ] && ok || bad "the untitled cart wrote to the carts folder" "$(ls "$UT")"
ut 'edit{enter}{ctrl+end}{enter}print(1){esc}save keepme{enter}'; said "SAVE name" 'saved as'
grep -qxF 'print(1)' "$UT/keepme/main.gb" && ok || bad "SAVE name: main.gb lacks the edit" "$(cat "$UT/keepme/main.gb" 2>&1)"
grep -qxF 'title = keepme' "$UT/keepme/cart.txt" && ok || bad "SAVE name: cart.txt title" "$(cat "$UT/keepme/cart.txt" 2>&1)"
ut 'save keepme{enter}';                        said "SAVE name taken" "overwrite it? (Y/N)"
ut 'edit{enter}{ctrl+end}{enter}print(2){esc}save keepme{enter}n'; said "SAVE taken, N" "not saved"
grep -qxF 'print(2)' "$UT/keepme/main.gb" && bad "SAVE taken, N: overwrote anyway" || ok
echo stale > "$UT/keepme/stale.txt"
ut 'edit{enter}{ctrl+end}{enter}print(3){esc}save keepme{enter}y'; said "SAVE taken, Y" 'saved as'
grep -qxF 'print(3)' "$UT/keepme/main.gb" && ok || bad "SAVE taken, Y: main.gb lacks the edit" "$(cat "$UT/keepme/main.gb" 2>&1)"
[ ! -e "$UT/keepme/stale.txt" ] && ok || bad "SAVE taken, Y: the old cart's files linger"
ut 'save{enter}';                               said "SAVE alone" 'saved as ucarts/untitled'
ut 'save{enter}y';                              said "SAVE alone, taken" 'overwrite it? (Y/N)'; said "SAVE alone, Y" 'saved as ucarts/untitled'
ut 'edit{enter}x{esc}load keepme{enter}';              said "untitled edits ask" 'discard them? (Y/N)'
ut 'load keepme{enter}edit{enter}{ctrl+end}{enter}print(4){esc}quit{enter}';  said "QUIT with edits asks" 'save changes? (Y/N)'
ut 'load keepme{enter}edit{enter}{ctrl+end}{enter}print(4){esc}quit{enter}n'
grep -qxF 'print(4)' "$UT/keepme/main.gb" && bad "QUIT, N: saved anyway" || ok
ut 'load keepme{enter}edit{enter}{ctrl+end}{enter}print(5){esc}quit{enter}{esc}info{enter}'; said "QUIT, Esc stays" 'still here'
grep -qxF 'print(5)' "$UT/keepme/main.gb" && bad "QUIT, Esc: saved anyway" || ok
ut 'load keepme{enter}edit{enter}{ctrl+end}{enter}print(6){esc}quit{enter}y'
grep -qxF 'print(6)' "$UT/keepme/main.gb" && ok || bad "QUIT, Y: main.gb lacks the edit" "$out$(cat "$UT/keepme/main.gb" 2>&1)"
ut 'edit{enter}x{esc}quit{enter}y';                    said "untitled QUIT, Y" 'not saved: still here'
ls "${TMPDIR:-/tmp}"/gooberengine-untitled-* >/dev/null 2>&1 && bad "the untitled cart's scratch folder is left behind" || ok

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
