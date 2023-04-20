
type() {

  local( entity_t * entity );
  local( sprite_t * sprite );

} actor_t;

#define actor_entity(self) (self->entity)
#define actor_sprite(self) (self->sprite)

#define actor_rect(self)     entity_rect(actor_entity(self))
#define actor_pos_x(self)    rect_x(actor_rect(self))
#define actor_pos_y(self)    rect_y(actor_rect(self))
#define actor_size_x(self)   rect_w(actor_rect(self))
#define actor_size_y(self)   rect_h(actor_rect(self))
#define actor_offset(self)   sprite_offset(actor_sprite(self))
#define actor_offset_x(self) point_x(actor_offset(self))d
#define actor_offset_y(self) point_y(actor_offset(self))

/////////
// new //
/////////

void init_actor( actor_t * self ) {
  actor_entity(self) = entity();
  actor_sprite(self) = sprite_as_blank();
}

actor_t * actor(void) {
  actor_t * self = alloc(actor_t);
  init_actor(self);
  return self;
}

void free_actor( actor_t * self ) {
  free_entity( actor_entity(self) );
  free_sprite( actor_sprite(self) );
  free( self );
}

////////////
// events //
////////////

void actor_update( actor_t * self ) {
  sprite_update( actor_sprite(self) );
}