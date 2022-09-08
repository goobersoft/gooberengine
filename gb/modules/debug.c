

/////////////
// globals //
/////////////

debugpanel_t  * debug_panel;
ball_t        * debug_ball;
sprayer_t     * debug_sprayer;
colormap_t    * debug_colormap;

////////////
// events //
////////////

void debug_init() {
  debug_panel   = debugpanel();
  debug_ball    = ball( gb_graph(), gb_controller() );
  debug_sprayer = sprayer( gb_graph(), 0, 0, 400, 240, make_color(1,1,1) );
  sprayer_mode(debug_sprayer) = graph_mode_sub();
}

void debug_load() {
}


void debug_start() {
  //log("%s",string_data(u));
  string_t * r = colormap_to_base64( assets_get_colormap( gb_assets(), "gb-0" ) );

  debug_colormap = colormap_from_base64( 400, 240, string_data(r) );

  free_string(r);
}

void debug_update_pre() {
  ball_update( debug_ball );
}

void debug_update_post() {
}

void debug_draw_pre() {
  //sprayer_draw( debug_sprayer );
  graph_draw_colormap( gb_graph(), 0, 0, debug_colormap );
  ball_draw( debug_ball );
}

void debug_draw_post() {
  //debugpanel_draw(debug_panel);
}

void debug_quit() {
}