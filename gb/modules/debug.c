

debugpanel_t  * debug_panel;
tilemap_t     * debug_tilemap;
int             debug_x;

////////////
// events //
////////////

void debug_init() {
  debug_panel = debugpanel();
  
}

void debug_load() {
}

void debug_start() {
  debug_tilemap = tilemap(120,24,assets_get_colormap(gb_assets(),0));
  loop(i,0,120) {
    loop(j,0,24) {
      tilemap_plot_data(debug_tilemap,i,j,rnd(0,40),rnd(0,24));
    }
  }
}

void debug_update_pre() {
  debug_x += 1;
}

void debug_update_post() {
}

void debug_draw_pre() {
  graph_draw_rect(gb_graph(),0,0,400,240);
  graph_set_layer(gb_graph(),1);
  graph_draw_tilemap(gb_graph(),-(debug_x/10),0,debug_tilemap);
  graph_set_layer(gb_graph(),0);
  graph_merge_layer(gb_graph(),1);
}

void debug_draw_post() {
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}