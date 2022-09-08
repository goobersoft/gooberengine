

/////////////
// globals //
/////////////

debugpanel_t  * debug_panel;
ball_t        * debug_ball;
sprayer_t     * debug_sprayer;
fountain_t    * debug_fountain;

////////////
// events //
////////////

void debug_init() {
  debug_panel   = debugpanel();
  debug_ball    = ball( gb_graph(), gb_controller() );
  debug_sprayer = sprayer( gb_graph(), 0, 0, 400, 240, make_color(3,2,1) );
  sprayer_mode(debug_sprayer) = graph_mode_sub();

  
}

void debug_load() {
}


void debug_start() {
  debug_fountain = fountain( 200, 120, gb_graph(), assets_get_colormap( gb_assets(), "gb-0" ));
}

void debug_update_pre() {
  ball_update( debug_ball );
  fountain_update( debug_fountain );
}

void debug_update_post() {
}

void debug_draw_pre() {
  sprayer_draw( debug_sprayer );
  fountain_draw( debug_fountain );
  ball_draw( debug_ball );
}

void debug_draw_post() {
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}