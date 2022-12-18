
//////////
// type //
//////////

type() {

  pball_t  * pball;
  paddle_t * paddle;

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
  /*
  bricker_debug.paddle  = paddle();
  bricker_debug.pball   = pball();
  */
}

void bricker_debug_update() {
  /*
  paddle_update(bricker_debug.paddle);
  pball_update(bricker_debug.pball);
  */
}

void bricker_debug_draw() {
  /*
  int u = graph_set_intensity(gb_graph(),1000);
  paddle_draw(bricker_debug.paddle);
  pball_draw(bricker_debug.pball);
  graph_set_intensity(gb_graph(),u);
  */
}

void bricker_debug_quit() {
}