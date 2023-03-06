
#define sprayer_intensity_d() 200

type() {

  // position on screen
  point_t * pos;
  // size of rectangular region
  point_t * size;
  // color to spray
  color_t * color;
  // mode of spray
  int       mode;
  // intensity of spray (0-1000)
  int       intensity;

  // graph ref.
  // gb_graph() does not yet exist in this section of the code.
  graph_t * graph;

} sprayer_t;

#define sprayer_pos(self)       (self->pos)
#define sprayer_size(self)      (self->size)
#define sprayer_color(self)     (self->color)
#define sprayer_mode(self)      (self->mode)
#define sprayer_intensity(self) (self->intensity)
#define sprayer_graph(self)     (self->graph)

/////////
// new //
/////////

sprayer_t * sprayer( graph_t * g, int x, int y, int w, int h, color_t c ) {
  sprayer_t * r        = alloc(sprayer_t);

  sprayer_graph(r)    = g;

  sprayer_pos(r)       = point(x,y);
  sprayer_size(r)      = point(w,h);
  sprayer_color(r)     = color_from(c);
  sprayer_mode(r)      = graph_mode_normal();
  sprayer_intensity(r) = sprayer_intensity_d();
  return r;
}

////////////
// events //
////////////

void sprayer_draw( sprayer_t * self ) {
  if sprayer_graph(self) {
    int ti      = graph_set_intensity(sprayer_graph(self), sprayer_intensity(self));
    int tm      = graph_set_mode(sprayer_graph(self), sprayer_mode(self));
    color_t tc  = graph_set_color(sprayer_graph(self), val(sprayer_color(self)));

    graph_draw_rect(sprayer_graph(self), point_x(sprayer_pos(self)), point_y(sprayer_pos(self)),
      point_x(sprayer_size(self)), point_y(sprayer_size(self)));

    graph_set_intensity ( sprayer_graph(self), ti );
    graph_set_mode      ( sprayer_graph(self), tm );
    graph_set_color     ( sprayer_graph(self), tc );
  }
}