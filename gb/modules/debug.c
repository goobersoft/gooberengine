

debugpanel_t  * debug_panel;

////////////
// events //
////////////

void debug_init() {
  debug_panel  = debugpanel();
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
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}