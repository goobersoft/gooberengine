
//////////
// type //
//////////

type() {

  playfield_t * playfield;

} bricker_debug_t;

/////////////
// globals //
/////////////

bricker_debug_t bricker_debug;

////////////
// events //
////////////

void bricker_debug_init() {
  
}

void bricker_debug_load() {
}

void bricker_debug_start() {
  bricker_debug.playfield = playfield();

  fixed_t * f = fixed(-1,+995);
  log("%s",fixed_to_string(f));
  

  loop(i,10) {
    fixed_add( f, -1, 0 );
    log("%s",fixed_to_string(f));
    
  }
}

void bricker_debug_update() {
  playfield_update( bricker_debug.playfield );
}

void bricker_debug_draw_pre() {
  playfield_draw( bricker_debug.playfield );
}

void bricker_debug_draw_post() {

}

void bricker_debug_quit() {
}