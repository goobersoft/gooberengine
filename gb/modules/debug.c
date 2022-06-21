

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
  static int _init = 0;
  static int _star_x[100];
  static int _star_y[100];
  static int _star_s[100];

  if (_init == 0) {
    loop(i,0,100) {
      _star_x[i] = rnd(0,400000);
      _star_y[i] = rnd(0,240);
      _star_s[i] = rnd(1000,10000);
    }
    _init = 1;
  }

  graph_set_mode        ( gb_graph(), graph_mode_sub() );
  graph_set_color       ( gb_graph(), color(1,1,1) );
  graph_draw_rect_spray ( gb_graph(), 0, 0, 400, 240, 200 );

  graph_set_mode        ( gb_graph(), graph_mode_normal() );
  graph_set_color       ( gb_graph(), color(0,2,3) );
  loop(i,0,100) {
    _star_x[i] -= _star_s[i];
    if (_star_x[i] < 0) {
      _star_x[i] = 400000;
      _star_y[i] = rnd(0,240);
    }

    graph_draw_hl( gb_graph(), _star_x[i]/1000, _star_y[i], _star_s[i]/1000 );
  }

  graph_draw_jake( gb_graph(), debug_jake );
  graph_draw_text( gb_graph(), 100, 100, "stars");
}

void debug_draw_post() {

}

void debug_quit() {
}