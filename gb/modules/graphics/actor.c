
type() {

  local( entity_t * entity );
  local( sprite_t * sprite );
  foreign( object_t * spec );

} actor_t;

#define actor_entity(self) (self->entity)
#define actor_sprite(self) (self->sprite)
#define actor_spec(self)   (self->spec)

/////////
// new //
/////////

void init_actor( actor_t * self ) {
  actor_entity(self) = entity();
  actor_sprite(self) = sprite_as_blank();
  actor_spec(self)   = object_as_empty();
}

actor_t * actor() {
  actor_t * self = alloc(actor_t);
  init_actor(self);
  return self;
}