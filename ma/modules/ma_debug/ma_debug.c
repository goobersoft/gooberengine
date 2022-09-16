

/////////////
// globals //
/////////////

ball_t        * debug_ball;
sprayer_t     * debug_sprayer;
fountain_t    * debug_fountain;
makeii_t      * debug_makeii;


////////////
// events //
////////////

void ma_debug_init() {
}

void ma_debug_load() {
  
}

void ma_debug_start() {

  debug_ball    = ball( gb_graph(), gb_controller() );
  debug_fountain = fountain( 200, 120, gb_graph(), assets_get_colormap( gb_assets(), "gb-0" ));

  debug_sprayer = sprayer( gb_graph(), 0, 0, 400, 240, make_color(1,1,1) );
  sprayer_mode(debug_sprayer) = graph_mode_sub();
  

  debug_makeii = makeii();
}

void ma_debug_update() {
  makeii_update( debug_makeii );
  ball_update( debug_ball );
  fountain_update( debug_fountain );
}

void ma_debug_draw() {
  sprayer_draw( debug_sprayer );
  fountain_draw( debug_fountain );
  ball_draw( debug_ball );
  makeii_draw( debug_makeii );
}

void ma_debug_quit() {
}