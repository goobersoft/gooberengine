
#define ball_size()     20000
#define ball_gravity()    125

type() {

  local( point_t * pos );
  local( point_t * velo );
  local( color_t * color );

  // reference to graph object
  foreign( graph_t      * graph );
  foreign( controller_t * controller );

} ball_t;

#define ball_pos(self)        (self->pos)
#define ball_velo(self)       (self->velo)
#define ball_color(self)      (self->color)
#define ball_graph(self)      (self->graph)
#define ball_controller(self) (self->controller)

/////////
// new //
/////////

ball_t * ball( graph_t * g, controller_t * c ) {
  ball_t * r    = alloc(ball_t);
  ball_pos(r)   = point(rnd(0,380000),0);
  ball_velo(r)  = point(0,0);
  ball_color(r) = color(0,0,3);

  ball_graph(r)      = g;
  ball_controller(r) = c;
  
  return r;
}

////////////
// events //
////////////

void ball_update( ball_t * self ) {

  // controller inquiries
  if ball_controller(self) {
    if (controller_is_button_held( ball_controller(self), controller_button_left() )) {
      point_add( ball_velo(self), -100, 0 );
    }
    else if (controller_is_button_held( ball_controller(self), controller_button_right() )) {
      point_add( ball_velo(self), 100, 0 );
    }

    if (controller_is_button_held( ball_controller(self), controller_button_down() )) {
      point_add( ball_velo(self), 0, 100 );
    }
    else if (controller_is_button_pressed( ball_controller(self), controller_button_up() )) {
      point_add( ball_velo(self), 0, -3000 );
    }
  }

  // add velocity
  point_add( ball_velo(self), 0, ball_gravity() );
  // add position
  point_add( ball_pos(self), point_x(ball_velo(self)), point_y(ball_velo(self)) );
  // check positions
  if (point_x( ball_pos(self) ) < 10000) {
    point_x(ball_pos(self))   = 10000;
    point_x(ball_velo(self))  = frac(abs(point_x(ball_velo(self))),9,10);
  }
  else if (point_x(ball_pos(self)) >= 390000) {
    point_x(ball_pos(self))   = 389999;
    point_x(ball_velo(self))  = -frac(abs(point_x(ball_velo(self))),9,10);
  }

  if (point_y(ball_pos(self)) >= 230000) {
    point_y(ball_pos(self))  = 229999;
    point_y(ball_velo(self)) = -frac( abs(point_y(ball_velo(self))),9,10);
    point_x(ball_velo(self)) = frac(point_x(ball_velo(self)),9,10);
  }
}

void ball_draw( ball_t * self) {
  if ball_graph(self) {
    int ti = graph_set_intensity(ball_graph(self),1000);
    color_t tm = graph_set_color(ball_graph(self),val(ball_color(self)));
    graph_draw_circle_line (ball_graph(self), point_x(ball_pos(self))/1000, point_y(ball_pos(self))/1000, 10);
    graph_set_intensity    (ball_graph(self),ti);
    graph_set_color        (ball_graph(self),tm);
  }
}