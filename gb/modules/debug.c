

/////////////
// globals //
/////////////

colormap_t * debug_colormap;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  assets_load_image( gb_assets(), 4, "ma/media/images/ma-0.png" );
  debug_colormap = assets_get_colormap(gb_assets(),0);
  log("%p",debug_colormap);
}

void debug_update_pre() {
  
}

void debug_update_post() {
}

void debug_draw_pre() {
  
}

void debug_draw_post() {
  static int wp = 0;

  /*
  graph_set_mode           ( gb_graph(), graph_mode_sub() );
  graph_set_color          ( gb_graph(), color(1,1,1) );
  graph_draw_rect_spray    ( gb_graph(), 0, 0, 400, 240, 50 );
  */
  /*
  loop(i,0,5) {
    graph_set_mode           ( gb_graph(), graph_mode_normal() );
    graph_set_color          ( gb_graph(), color(0,2,3) );
    graph_draw_circle_dots   ( gb_graph(), rnd(0,400), rnd(0,240), rnd(10,200), 60, 0 );
  }
  */

  graph_set_mode( gb_graph(), graph_mode_sub() );
  graph_set_color( gb_graph(), color(3,3,2) );
  graph_draw_hl( gb_graph(), 0, wp, 400 );
  wp = (wp + 1) % 240;

  graph_set_mode           ( gb_graph(), graph_mode_normal() );
  //graph_draw_colormap      ( gb_graph(), 0, 0, debug_colormap );
  graph_draw_colormap_spray( gb_graph(), 0, 0, debug_colormap, 20 );

  if (prob(10)) log("%d",graph_frame_dots(gb_graph()));
}

void debug_quit() {
}