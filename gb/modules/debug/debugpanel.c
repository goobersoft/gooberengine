
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

    // clear the string for the panel
    string_clear(debugpanel_string(self),' ');


    // button states (0: off, 1: held, 2: pressed)
    loop(i,controller_max_buttons()) {
      string_copy_number_at(debugpanel_string(self),i,controller_buttons(gb_controller())[i]);
    }

    // copy the FPS string
    string_copy_at(debugpanel_string(self),13,"FPS:");
    string_copy_number_at(debugpanel_string(self),17,timing_fps(gb_timing()));

    // copy the dots string
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
    
    u2 = string_copy_at(debugpanel_string(self),40,"CPU:");
    // will draw a bar instead of text

    u2 = string_copy_at ( debugpanel_string(self), 60, rstr(timing_clock_hours(gb_timing()),2) );
    u2 = string_copy_at ( debugpanel_string(self), 62, ":" );
    u2 = string_copy_at ( debugpanel_string(self), 63, rstr(timing_clock_minutes(gb_timing()),2) );
    u2 = string_copy_at ( debugpanel_string(self), 65, ":" );
    u2 = string_copy_at ( debugpanel_string(self), 66, rstr(timing_clock_seconds(gb_timing()),2) );

    u2 = string_copy_at ( debugpanel_string(self), 70, "S:");
    if (gb_scene()) {
      u2 = string_copy_at ( debugpanel_string(self), 72, scene_id(gb_scene()));
    }

    /////////////
    // drawing //
    /////////////

    // set to layer 9 (or top layer)
    graph_set_layer(gb_graph(),-1);
    // clear layer
    graph_cls(gb_graph());
    // set graph mode to normal
    graph_set_mode( gb_graph(), graph_mode_normal() );

    // record prev intensity, and set the intensity to max
    int u = graph_set_intensity_max(gb_graph());

    // set color to black
    graph_set_color(gb_graph(),make_color(0,0,0));
    // draw the background strip at the top
    graph_draw_rect(gb_graph(),0,0,400,10);

    // set color to red
    graph_set_color(gb_graph(),make_color(3,0,0));
    // make the CPU bar
    graph_draw_rect(gb_graph(),220,2,timing_cpu_usage(gb_timing())/2,6);

    // set color to white
    graph_set_color(gb_graph(),make_color(3,3,3));
    // draw the debugging string
    graph_draw_text(gb_graph(),0,0,string_data(debugpanel_string(self)));

    // reset the intensity
    graph_set_intensity(gb_graph(),u);
    // set layer to 0
    graph_set_layer(gb_graph(),0);
    // merge the top layer.
    graph_merge_layer(gb_graph(),-1);
  }
}