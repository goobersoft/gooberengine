

/////////////
// globals //
/////////////

debugpanel_t  * debug_panel;
ball_t        * debug_ball;
sprayer_t     * debug_sprayer;

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

}

void debug_update_pre() {
  ball_update( debug_ball );
}

void debug_update_post() {
}

void debug_draw_pre() {
  sprayer_draw( debug_sprayer );
  ball_draw( debug_ball );
}

void debug_draw_post() {
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}