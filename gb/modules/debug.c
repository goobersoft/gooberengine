

/////////////
// globals //
/////////////

int        debug_wave_y;
string_t * debug_string;

////////////
// events //
////////////

void debug_init() {
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
  graph_t * g     = gb_graph();
  colormap_t * m  = assets_get_colormap(gb_assets(),0);
  graph_set_intensity (g,10);
  graph_draw_colormap (gb_graph(),0,0,m);
}

void debug_draw_post() {
}

void debug_quit() {
}