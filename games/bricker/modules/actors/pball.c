
type() {

  local( actor_t * actor );
  local( point_t * velo );
  
  

} actor_pball_t;

#define actor_pball_actor(self)   (self->actor)
#define actor_pball_sprite(self)  actor_sprite(actor_pball_actor(self))
#define actor_pball_entity(self)  actor_entity(actor_pball_actor(self))

#define actor_pball_rect(self)      entity_rect(actor_pball_entity(self))
#define actor_pball_pos_x(self)     rect_x(actor_pball_rect(self))
#define actor_pball_pos_y(self)     rect_y(actor_pball_rect(self))
#define actor_pball_size_x(self)    rect_w(actor_pball_rect(self))
#define actor_pball_size_y(self)    rect_h(actor_pball_rect(self))
#define actor_pball_offset_x(self)  point_x(sprite_offset(actor_pball_sprite(self)))
#define actor_pball_offset_y(self)  point_y(sprite_offset(actor_pball_sprite(self)))  

#define actor_pball_velo(self)      (self->velo)
#define actor_pball_velo_x(self)    point_x(actor_pball_velo(self))
#define actor_pball_velo_y(self)    point_y(actor_pball_velo(self))

/////////
// new //
/////////

void init_actor_pball( actor_pball_t * self ) {
  actor_pball_actor(self)  = actor("pball",self);

  actor_pball_velo(self)    = point(2,1);
  actor_pball_sprite(self)  = sprite(gb_get_colormap("bricker-0"),30,30,10,10);
  // the sprite will be offset by 1 pixel in x and y direction
  // to account for the smaller bounding box.
  sprite_set_offset   (actor_pball_sprite(self),1,1);
  
  entity_set_pos      (actor_pball_entity(self), 200, 200 );
  entity_set_size     (actor_pball_entity(self), 8, 8 );

}

actor_pball_t * actor_pball() {
  actor_pball_t * self = alloc(actor_pball_t);
  init_actor_pball(self);
  return self;
}

void free_actor_pball( actor_pball_t * self ) {
  free_actor ( actor_pball_actor(self) );
  free_point ( actor_pball_velo(self) );
  free(self);
}

////////////
// macros //
////////////

////////////
// events //
////////////

void actor_pball_update( actor_pball_t * self ) {

  if (actor_pball_pos_x(self)+actor_pball_size_x(self) >= 300) {
    actor_pball_pos_x(self) = 300 - actor_pball_size_x(self);
    actor_pball_velo_x(self) = -actor_pball_velo_x(self);
  }
  else if (actor_pball_pos_x(self) < 100) {
    actor_pball_pos_x(self) = 100;
    actor_pball_velo_x(self) = -actor_pball_velo_x(self);
  }

  if (actor_pball_pos_y(self)+actor_pball_size_y(self) >= 240) {
    actor_pball_pos_y(self) = 240 - actor_pball_size_y(self);
    actor_pball_velo_y(self) = -actor_pball_velo_y(self);
  }
  else if (actor_pball_pos_y(self) < 0) {
    actor_pball_pos_y(self) = 0;
    actor_pball_velo_y(self) = -actor_pball_velo_y(self);
  }
}

void actor_pball_draw( actor_pball_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), actor_pball_pos_x(self), actor_pball_pos_y(self), actor_pball_sprite(self) );
  graph_reset_intensity( gb_graph() );
}