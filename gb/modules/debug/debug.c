
//////////
// type //
//////////

type() {

  debugpanel_t * panel;
  colormap_t   * colormap;
  list_t       * list;


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
  debug.list    = list();
}

void debug_load() {
}


void debug_start() {
  debug.colormap = gb_get_colormap("bricker-0");

  dict_t * d = dict();
  dict_set(d,"1ne","one");
  dict_set(d,"2wo","two");
  dict_set(d,"3hree","one");
  _dict_print(d);
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