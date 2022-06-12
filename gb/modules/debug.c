

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
jake_t     * debug_jake;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  //assets_load_image( gb_assets(), 4, "ma/media/images/ma-0.png" );
  debug_colormap  = assets_get_colormap(gb_assets(),0);
  debug_jake      = jake(debug_colormap);
}

void debug_update_pre() {
  jake_update(debug_jake);
}

void debug_update_post() {
}

void debug_draw_pre() {
  
}

void debug_draw_post() {
  static int _init = 0;
  static int _tmr = 0;
  static int _tdots = 0;
  static char _str[20];
  if (_init == 0) {
    graph_set_cls_color( gb_graph(), color(0,0,3) );
    _init = 1;
  }

  //graph_cls(gb_graph());

  
  graph_set_mode( gb_graph(), graph_mode_high() );
  graph_set_color( gb_graph(), color(0,0,3) );
  graph_draw_rect_spray( gb_graph(), 0, 0, 400, 240, 100 );
  
  graph_set_mode( gb_graph(), graph_mode_normal() );
  graph_set_color( gb_graph(), color_random() );
  graph_draw_triangle_line( gb_graph(), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240) );

  graph_set_mode( gb_graph(), graph_mode_normal() );
  graph_draw_text( gb_graph(), 10, 10, "Hello world. Welcome to GooberEngine!" );
  
  sprintf(_str,"FPS: %d",timing_fps(gb_timing()));
  graph_draw_text( gb_graph(), 0, 230, _str );

  _tmr += 1;
  if (_tmr == 60) {
    _tmr = 0;
    _tdots = graph_frame_dots(gb_graph());
  }

  sprintf(_str,"DOTS: %d",_tdots);
  graph_draw_text( gb_graph(), 0, 220, _str );

  //graph_set_flip( gb_graph(), true(), false() );
  graph_set_mode( gb_graph(), graph_mode_normal() );
  graph_draw_jake( gb_graph(), debug_jake );
  
  graph_set_mode( gb_graph(), graph_mode_normal() );
}

void debug_quit() {
}