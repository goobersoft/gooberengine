

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
image_t *    debug_image;
sound_t *    debug_sound;
list_t *     debug_list;
int          debug_data;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  debug_image     = assets_get_image(gb_assets(),0);
  debug_colormap  = colormap_from_image( debug_image );
  graph_cls(gb_graph());
}

void debug_update() {
  int rn;
  if (prob(1)) {
    rn = rnd(0,4);
    audio_play(gb_audio(),assets_get_sound(gb_assets(),rn),rn);
  }
}

void debug_draw_pre() {
}

void debug_draw_post() {
  static int _f = 0;

  graph_set_mode           ( gb_graph(), graph_mode_normal() );
  graph_set_color          ( gb_graph(), color_random() );
  graph_set_depth          ( gb_graph(), 100 );
  graph_draw_triangle_line ( gb_graph(), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240));
  
  graph_enable_depth ( gb_graph(), false() );
  graph_set_depth    ( gb_graph(), 0 );
  
  graph_set_mode( gb_graph(), graph_mode_low() );
  graph_set_flip( gb_graph(), true(), true() );
  graph_draw_colormap( gb_graph(), 0, 0, debug_colormap );
  
  graph_set_mode     ( gb_graph(), graph_mode_sub() );
  graph_set_color    ( gb_graph(), color(1,1,1) );
  if (_f==0) {
    graph_draw_rect_dither( gb_graph(), 0, 0, 400, 240, false() );
    _f += 1;
  }
  else if (_f==1) { 
    graph_draw_rect_dither( gb_graph(), 0, 0, 400, 240, true() );
    _f = 0;
  }
  graph_enable_depth( gb_graph(), true() );
  
  
}

void debug_quit() {
}