
//////////
// type //
//////////

type() {

  debugpanel_t * panel;
  colormap_t * colormap;

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

  int u = graph_set_intensity_max( gb_graph() );
  graph_draw_number( gb_graph(), 200, 200, rnd(-999,9999), debug.colormap,
    110, 0, 10, 20);
  graph_set_intensity( gb_graph(), 100 );
}

void debug_quit() {
}