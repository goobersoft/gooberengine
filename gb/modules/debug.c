

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
image_t *    debug_image;
sound_t *    debug_sound;
list_t *     debug_list;
int          debug_data;
palette_t *  debug_palette;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  debug_image     = assets_get_image(gb_assets(),0);
  debug_colormap  = colormap_from_image( debug_image );
  debug_palette   = palette__gameboy();
  graph_cls(gb_graph());
}

void debug_update() {
}

void debug_draw_pre() {
  static int _f = 0;
  static int _c = 0;

  graph_t * g = gb_graph();
  
  graph_set_palette( g, debug_palette );
  graph_set_mode           ( gb_graph(), graph_mode_normal() );
  graph_set_color          ( gb_graph(), color_random() );
  graph_draw_triangle_line ( gb_graph(), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240));
  
  loop(i,0,10) {
    
    graph_set_mode( g, graph_mode_normal() );
    graph_set_flip( g, rnd(0,2), rnd(0,2) );
    graph_draw_colormap_sub( g, rnd(0,400), rnd(0,240), debug_colormap, rnd(0,400), rnd(0,240), 10, 10 );
  }

  graph_set_palette(g, null());
  graph_set_mode     ( g, graph_mode_sub() );
  graph_set_color    ( g, color(1,1,1) );

  graph_draw_rect_dither( g, 0, _f, 400, 5, _c );
  _f = _f + 5;
  if (_f == 240) {
    _f = 0;
    _c = (_c + 1) % 2;
  }
}

void debug_draw_post() {
}

void debug_quit() {
}