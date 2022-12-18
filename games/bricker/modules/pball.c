
type() {

  // position
  local( point_t * pos );
  // velocity
  local (point_t * velo );
  // size
  local( point_t * size );
  // sprite
  local( sprite_t * sprite );

} pball_t;

#define pball_tag(self)     (self->tag)
#define pball_id(self)      (self->tag->id)
#define pball_pos(self)     (self->pos)
#define pball_velo(self)    (self->velo)
#define pball_pos_x(self)   point_x(self->pos)
#define pball_pos_y(self)   point_y(self->pos)
#define pball_size(self)    (self->size)
#define pball_sprite(self)  (self->sprite)

/////////
// new //
/////////

void init_pball( pball_t * self ) {
  pball_pos(self)     = point(0,20);
  pball_size(self)    = point(8,8);
  pball_sprite(self)  = sprite(gb_get_colormap("bricker-0"),30,30,10,10);
  sprite_set_offset(pball_sprite(self),0,0);
}

pball_t * pball() {
  pball_t * r = alloc(pball_t);
  init_pball(r);
  return r;
}

////////////
// events //
////////////

void pball_update( pball_t * self ) {
}

void pball_draw( pball_t * self ) {
  graph_draw_sprite( gb_graph(), pball_pos_x(self), pball_pos_y(self), pball_sprite(self) );
}