
//////////
// type //
//////////

type() {

  debugpanel_t * panel;

} debug_t;

/////////////
// globals //
/////////////

debug_t debug;

////////////
// events //
////////////

void debug_init() {
  debug.panel   = debugpanel();
}

void debug_load() {
}


void debug_start() {
}

void debug_update_pre() {
}

void debug_update_post() {
}

void debug_draw_pre() {
}

void debug_draw_post() {
  debugpanel_draw(debug.panel);
}

void debug_quit() {
}