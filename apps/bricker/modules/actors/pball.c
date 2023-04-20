
type() {

  local( actor_t * actor );
  local( point_t * velo );

} pball_t;

#define pball_actor(self)     (self->actor)
#define pball_sprite(self)    actor_sprite(pball_actor(self))
#define pball_entity(self)    actor_entity(pball_actor(self))

#define pball_rect(self)      entity_rect(pball_entity(self))
#define pball_pos_x(self)     rect_x(pball_rect(self))
#define pball_pos_y(self)     rect_y(pball_rect(self))
#define pball_size_x(self)    rect_w(pball_rect(self))
#define pball_size_y(self)    rect_h(pball_rect(self))
#define pball_offset_x(self)  point_x(sprite_offset(pball_sprite(self)))
#define pball_offset_y(self)  point_y(sprite_offset(pball_sprite(self)))  

#define pball_velo(self)      (self->velo)
#define pball_velo_x(self)    point_x(pball_velo(self))
#define pball_velo_y(self)    point_y(pball_velo(self))

/////////
// new //
/////////

void init_pball( pball_t * self ) {
  pball_actor(self)   = actor();

  pball_velo(self)    = point(200,100);
  pball_sprite(self)  = sprite(gb_get_colormap("bricker-0"),30,30,10,10);
  // the sprite will be offset by 1 pixel in x and y direction
  // to account for the smaller bounding box.
  sprite_set_offset   (pball_sprite(self),1,1);
  
  entity_set_pos      (pball_entity(self), 200000, 200000 );
  entity_set_size     (pball_entity(self), 8000, 8000 );

}

pball_t * pball(void) {
  pball_t * self = alloc(pball_t);
  init_pball(self);
  return self;
}

void free_pball( pball_t * self ) {
  free_actor ( pball_actor(self) );
  free_point ( pball_velo(self) );
  free(self);
}

////////////
// macros //
////////////

////////////
// events //
////////////

void pball_update( pball_t * self ) {

  if (pball_pos_x(self)+pball_size_x(self) >= 300000) {
    pball_pos_x(self) = 300000 - pball_size_x(self);
    pball_velo_x(self) = -pball_velo_x(self);
  }
  else if (pball_pos_x(self) < 100000) {
    pball_pos_x(self) = 100000;
    pball_velo_x(self) = -pball_velo_x(self);
  }

  if (pball_pos_y(self)+pball_size_y(self) >= 240000) {
    pball_pos_y(self) = 240000 - pball_size_y(self);
    pball_velo_y(self) = -pball_velo_y(self);
  }
  else if (pball_pos_y(self) < 0) {
    pball_pos_y(self) = 0;
    pball_velo_y(self) = -pball_velo_y(self);
  }
}

void pball_draw( pball_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), pball_pos_x(self)/1000, pball_pos_y(self)/1000, pball_sprite(self) );
  graph_reset_intensity( gb_graph() );
}