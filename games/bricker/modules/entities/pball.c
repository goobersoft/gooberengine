
type() {

  local( point_t * velo );
  // size
  local( point_t * size );
  // sprite
  local( sprite_t * sprite );
  // entity
  local( entity_t * entity );

} pball_t;

#define pball_tag(self)     (self->tag)
#define pball_id(self)      (self->tag->id)
#define pball_velo(self)    (self->velo)
#define pball_velo_x(self)  point_x(pball_velo(self))
#define pball_velo_y(self)  point_y(pball_velo(self))
#define pball_sprite(self)  (self->sprite)
#define pball_entity(self)  (self->entity)
#define pball_pos(self)     entity_pos(pball_entity(self))
#define pball_pos_x(self)   point_x(pball_pos(self))
#define pball_pos_y(self)   point_y(pball_pos(self))
#define pball_size(self)    entity_size(pball_entity(self))
#define pball_size_x(self)  point_x(pball_size(self))
#define pball_size_y(self)  point_y(pball_size(self))




/////////
// new //
/////////

void init_pball( pball_t * self ) {
  pball_velo(self)    = point(2,1);
  pball_sprite(self)  = sprite(gb_get_colormap("bricker-0"),30,30,10,10);
  // the sprite will be offset by 1 pixel in x and y direction
  // to account for the smaller bounding box.
  sprite_set_offset(pball_sprite(self),1,1);
  pball_entity(self)  = entity(self);
  entity_set_pos      (pball_entity(self), 200, 200 );
  entity_set_size     (pball_entity(self), 8, 8 );

}

pball_t * pball() {
  pball_t * self = alloc(pball_t);
  init_pball(self);
  return self;
}

void free_pball( pball_t * self ) {
  free_point  ( pball_velo(self) );
  free_sprite ( pball_sprite(self) );
  free(self);
}

////////////
// macros //
////////////

#define pball_collide(self,other)     entity_collide(pball_entity(self),other)
#define pball_collide_list(self,ls)   entity_collide_list(pball_entity(self),ls)

////////////
// events //
////////////

void pball_update( pball_t * self ) {

  if (pball_pos_x(self)+pball_size_x(self) >= 300) {
    pball_pos_x(self) = 300 - pball_size_x(self);
    pball_velo_x(self) = -pball_velo_x(self);
  }
  else if (pball_pos_x(self) < 100) {
    pball_pos_x(self) = 100;
    pball_velo_x(self) = -pball_velo_x(self);
  }

  if (pball_pos_y(self)+pball_size_y(self) >= 240) {
    pball_pos_y(self) = 240 - pball_size_y(self);
    pball_velo_y(self) = -pball_velo_y(self);
  }
  else if (pball_pos_y(self) < 0) {
    pball_pos_y(self) = 0;
    pball_velo_y(self) = -pball_velo_y(self);
  }

}

void pball_draw( pball_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_set_color(gb_graph(), make_color(0,0,3));
  graph_draw_sprite( gb_graph(), pball_pos_x(self), pball_pos_y(self), pball_sprite(self) );
  graph_reset_intensity( gb_graph() );
}