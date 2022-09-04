
#define debugpanel_pos_bottom() 0
#define debugpanel_pos_top()    1

#define debugpanel_max_text()       80
#define debugpanel_text_pos_fps(n)  n+0
#define debugpanel_text_pos_cpu(n)  n+10

type() {

  bool_t     visible;
  bool_t     pos;
  string_t * string;



} debugpanel_t;

#define debugpanel_visible(self)   (self->visible)
#define debugpanel_pos(self)       (self->pos)
#define debugpanel_string(self)    (self->string)

/////////
// new //
/////////

debugpanel_t * debugpanel() {
  debugpanel_t * self        = alloc(debugpanel_t);
  debugpanel_visible(self)   = true();
  debugpanel_pos(self)       = debugpanel_pos_top();
  debugpanel_string(self)    = string(debugpanel_max_text());

  return self;
}

////////////
// events //
////////////

void debugpanel_update( debugpanel_t * self ) {
  
}

void debugpanel_draw( debugpanel_t * self ) {

  if (debugpanel_visible(self)) {

    //////////////
    // updating //
    //////////////

    string_clear(debugpanel_string(self));
    
    loop(i,0,controller_max_buttons()) {
      string_copy_number_at(debugpanel_string(self),i,controller_buttons(gb_controller())[i]);
    }

    string_copy_at(debugpanel_string(self),13,"FPS:");
    string_copy_number_at(debugpanel_string(self),17,timing_fps(gb_timing()));

    string_copy_at(debugpanel_string(self),20,"DOTS:");

    int u1 = graph_frame_dots(gb_graph())/1000;
    int uu = digits(u1);

    string_copy_number_at(debugpanel_string(self),25,u1);
    string_copy_at(debugpanel_string(self),25+uu,"\%");

    int u2 = 0;
    u2 = string_copy_number_at(debugpanel_string(self),
      30,point_x(mouse_pos(gb_mouse()))/visual_window_scale(gb_visual()));
    u2 = string_copy_at(debugpanel_string(self),u2,",");
    u2 = string_copy_number_at(debugpanel_string(self),
      u2,point_y(mouse_pos(gb_mouse()))/visual_window_scale(gb_visual()));
    
    string_copy_at(debugpanel_string(self),50,"P:");
    string_copy_number_at(debugpanel_string(self),52,gb_paused());

    /////////////
    // drawing //
    /////////////

    int u = graph_intensity(gb_graph());
    graph_set_intensity(gb_graph(),1000);

    graph_set_color(gb_graph(),make_color(0,0,0));
    graph_draw_rect(gb_graph(),0,0,400,10);
    graph_set_color(gb_graph(),make_color(3,3,3));
    graph_draw_text(gb_graph(),0,0,string_data(debugpanel_string(self)));

    graph_set_intensity(gb_graph(),u);
  }
}