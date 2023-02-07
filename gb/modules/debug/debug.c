
//////////
// type //
//////////

type() {

  debugpanel_t * panel;
  colormap_t * colormap;

  entity_t  * ent1;
  list_t    * entl;

  int t;

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
  debug.colormap = gb_get_colormap("bricker-0");

  settings_t * u = settings();
  string_t * s = settings_to_string(u);
  log("%s %d",string_data(s),string_length(s));
}

void debug_update_pre() {
  debugpanel_update(debug.panel);
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