
///////////////////////
// fountain particle //
///////////////////////

#define fountpart_gravity() 125

type() {

  bool_t      alive;
  sprite_t  * sprite;
  point_t   * pos;
  point_t   * velo;

} fountpart_t;

#define fountpart_alive(self)   (self->alive)
#define fountpart_sprite(self)  (self->sprite)
#define fountpart_pos(self)     (self->pos)
#define fountpart_velo(self)    (self->velo)

fountpart_t * fountpart( int x, int y, sprite_t * s, int vx, int vy) {
  fountpart_t * r      = alloc(fountpart_t);
  fountpart_alive(r)   = true();
  fountpart_sprite(r)  = clone_sprite(s);
  fountpart_pos(r)     = point(x,y);
  fountpart_velo(r)    = point(vx,vy);
  return r;
}

void free_fountpart( fountpart_t * self ) {
  free_sprite( fountpart_sprite(self) );
  free_point( fountpart_pos(self) );
  free_point( fountpart_velo(self) );
  free(self);
}

void fountpart_update( fountpart_t * self ) {
  point_add( fountpart_velo(self), 0, fountpart_gravity() );
  point_add( fountpart_pos(self), point_x(fountpart_velo(self)), point_y(fountpart_velo(self)) );
  if (point_y( fountpart_pos(self)) > 240000 ) {
    fountpart_alive(self) = false();
  }
}


//////////////
// fountain //
//////////////

type() {

  // position on screen
  point_t *  pos;
  // angle to spray particles
  int        angle;
  // angle difference
  int        angle_width;
  // sprite to use for the fountain graphic
  sprite_t * sprite;

  // list of particles
  list_t   * particles;

  // references to graph/colormap
  graph_t    * graph;
  colormap_t * colormap;

} fountain_t;

#define fountain_pos(self)         (self->pos)
#define fountain_angle(self)       (self->angle)
#define fountain_angle_width(self) (self->angle_width)
#define fountain_sprite(self)      (self->sprite)
#define fountain_particles(self)   (self->particles)
#define fountain_graph(self)       (self->graph)
#define fountain_colormap(self)    (self->colormap)

/////////
// new //
/////////

fountain_t * fountain( int x, int y, graph_t * g, colormap_t * c ) {
  fountain_t * r          = alloc(fountain_t);
  fountain_pos(r)         = point(x,y);
  fountain_angle(r)       = 0;
  fountain_angle_width(r) = 250;

  fountain_graph(r)       = g;
  fountain_colormap(r)    = c;

  fountain_sprite(r)      = sprite( c, 290, 220, 10, 10 );
  fountain_particles(r)   = list();
  return r;
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void fountain_update( fountain_t * self ) {

  // create a particle every frame

  int an = rnd(0,fountain_angle_width(self)*2);
  int vx, vy;
  vx = frac(sine( an-fountain_angle_width(self) ),9,2);
  vy = frac(cosine( an-fountain_angle_width(self) ),9,2);

  fountpart_t * p = fountpart( point_x(fountain_pos(self))*1000, point_y(fountain_pos(self))*1000,
    fountain_sprite(self), vx, vy);
  if (prob(500)) rect_add_pos( sprite_rect(fountpart_sprite(p)), 0, 10 );

  list_add_last( fountain_particles(self), p );


  foreach(fountain_particles(self),dt) {
    fountpart_update(cast(dt,fountpart_t*));
  }


  // check the front of the list for dead particles, rotate once
  p = cast(list_get_first( fountain_particles(self) ),fountpart_t*);
  if (fountpart_alive(p)==false()) {
    list_remove_first( fountain_particles(self) );
    free_fountpart(p);
  }
  else {
    list_rotate_next( fountain_particles(self) );
  }
}

void fountain_draw( fountain_t * self ) {

  //int in = graph_set_intensity_max( fountain_graph(self) );

  // draw the particles
  fountpart_t * u;
  foreach( fountain_particles(self), dt ) {
    u = cast(dt,fountpart_t*);

    graph_draw_sprite( fountain_graph(self), fountpart_sprite(u), 
      point_x(fountpart_pos(u))/1000, point_y(fountpart_pos(u))/1000);

  }

  //graph_set_intensity( fountain_graph(self), in );

}