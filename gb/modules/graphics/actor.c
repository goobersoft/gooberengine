
type() {

  local( entity_t * entity );
  local( sprite_t * sprite );
  foreign( object_t * spec );

} actor_t;

#define actor_entity(self) (self->entity)
#define actor_sprite(self) (self->sprite)
#define actor_spec(self)   (self->spec)

#define actor_rect(self)   entity_rect(actor_entity(self))
#define actor_pos_x(self)  rect_x(actor_rect(self))
#define actor_pos_y(self)  rect_y(actor_rect(self))
#define actor_size_x(self) rect_w(actor_rect(self))
#define actor_size_y(self) rect_h(actor_rect(self))
#define actor_offset(self) sprite_offset(actor_sprite(self))
#define actor_offset_x(self) point_x(actor_offset(self))d
#define actor_offset_y(self) point_y(actor_offset(self))

#define actor_class(self)        object_class(actor_spec(self))
#define actor_set_spec(self,a,b) object_set(actor_spec(self),a,b)
#define actor_get_spec(self)     object_data(actor_spec(self))

/////////
// new //
/////////

void init_actor( actor_t * self, char * cls, void * dt ) {
  actor_entity(self) = entity("actor",self);
  actor_sprite(self) = sprite_as_blank();
  actor_spec(self)   = object(cls,dt);
}

actor_t * actor( char * cls, void * dt ) {
  actor_t * self = alloc(actor_t);
  init_actor(self,cls,dt);
  return self;
}

actor_t * actor_as_empty() {
  return actor("",null());
}

void free_actor( actor_t * self ) {
  free_entity( actor_entity(self) );
  free_sprite( actor_sprite(self) );
  free_object( actor_spec(self) );
  free( self );
}

//
//
//