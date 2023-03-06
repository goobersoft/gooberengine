
type() {

  local( entity_t * entity );

  local( point_t * velo );
  // size
  local( point_t * size );
  // sprite
  local( sprite_t * sprite );
  // entity
  

} entity_pball_t;

#define entity_pball_entity(self)  (self->entity)
#define entity_pball_pos(self)     entity_pos(entity_pball_entity(self))
#define entity_pball_pos_x(self)   point_x(entity_pball_pos(self))
#define entity_pball_pos_y(self)   point_y(entity_pball_pos(self))
#define entity_pball_size(self)    entity_size(entity_pball_entity(self))
#define entity_pball_size_x(self)  point_x(entity_pball_size(self))
#define entity_pball_size_y(self)  point_y(entity_pball_size(self))
#define entity_pball_velo(self)    (self->velo)
#define entity_pball_velo_x(self)  point_x(entity_pball_velo(self))
#define entity_pball_velo_y(self)  point_y(entity_pball_velo(self))
#define entity_pball_sprite(self)  (self->sprite)





/////////
// new //
/////////

void init_entity_pball( entity_pball_t * self ) {
  entity_pball_entity(self)  = entity("pball",self);

  entity_pball_velo(self)    = point(2,1);
  entity_pball_sprite(self)  = sprite(gb_get_colormap("bricker-0"),30,30,10,10);
  // the sprite will be offset by 1 pixel in x and y direction
  // to account for the smaller bounding box.
  sprite_set_offset   (entity_pball_sprite(self),1,1);
  
  entity_set_pos      (entity_pball_entity(self), 200, 200 );
  entity_set_size     (entity_pball_entity(self), 8, 8 );

}

entity_pball_t * entity_pball() {
  entity_pball_t * self = alloc(entity_pball_t);
  init_entity_pball(self);
  return self;
}

void free_entity_pball( entity_pball_t * self ) {
  free_point  ( entity_pball_velo(self) );
  free_sprite ( entity_pball_sprite(self) );
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

void entity_pball_update( entity_pball_t * self ) {

  if (entity_pball_pos_x(self)+entity_pball_size_x(self) >= 300) {
    entity_pball_pos_x(self) = 300 - entity_pball_size_x(self);
    entity_pball_velo_x(self) = -entity_pball_velo_x(self);
  }
  else if (entity_pball_pos_x(self) < 100) {
    entity_pball_pos_x(self) = 100;
    entity_pball_velo_x(self) = -entity_pball_velo_x(self);
  }

  if (entity_pball_pos_y(self)+entity_pball_size_y(self) >= 240) {
    entity_pball_pos_y(self) = 240 - entity_pball_size_y(self);
    entity_pball_velo_y(self) = -entity_pball_velo_y(self);
  }
  else if (entity_pball_pos_y(self) < 0) {
    entity_pball_pos_y(self) = 0;
    entity_pball_velo_y(self) = -entity_pball_velo_y(self);
  }
}

void entity_pball_draw( entity_pball_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), entity_pball_pos_x(self), entity_pball_pos_y(self), entity_pball_sprite(self) );
  graph_reset_intensity( gb_graph() );
}