
#define statpanel_pos_bottom() 0
#define statpanel_pos_top()    1

#define statpanel_max_text()       100
#define statpanel_text_pos_fps(n)  n+0
#define statpanel_text_pos_cpu(n)  n+10

type() {

  bool_t     visible;
  bool_t     pos;
  string_t * text;

  int value_timing_fps;
  int value_mouse_x;
  int value_mouse_y;
  int value_graph_dots;

} statpanel_t;

#define statpanel_visible(self)           (self->visible)
#define statpanel_pos(self)               (self->pos)
#define statpanel_text(self)              (self->text)
#define statpanel_value_timing_fps(self)  (self->value_timing_fps)
#define statpanel_value_mouse_x(self)     (self->value_mouse_x)
#define statpanel_value_mouse_y(self)     (self->value_mouse_y)
#define statpanel_value_graph_dots(self)  (self->value_graph_dots)


/////////
// new //
/////////

statpanel_t * statpanel() {
  statpanel_t * self                = alloc(statpanel_t);
  statpanel_visible(self)           = true();
  statpanel_pos(self)               = statpanel_pos_bottom();
  statpanel_text(self)              = string__length(100);
  return self;
}

////////////
// events //
////////////

void statpanel_update( statpanel_t * self ) {
}