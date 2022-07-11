

debugpanel_t  * debug_panel;
string_t      * debug_string;

////////////
// events //
////////////

void debug_init() {
  debug_panel = debugpanel();
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
  graph_set_intensity(gb_graph(),100);
  graph_set_mode(gb_graph(),graph_mode_normal());
  graph_set_color(gb_graph(),make_color(0,0,3));
  graph_draw_rect(gb_graph(),0,0,400,240);
}

void debug_draw_post() {
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}