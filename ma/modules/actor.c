
// actor direction constants
#define ma_actor_dir_north()   1
#define ma_actor_dir_east()    2
#define ma_actor_dir_south()   3
#define ma_actor_dir_west()    4

// default actor gravity value
#define ma_actor_gravity_d()   125

type() {

  // an integer direction value
  int        dir;
  // a gravity constant
  int        gravity;

  // component holding the bounding box
  local( entity_t * entity );
  // a velocity vector
  local( point_t  * velo );
  // a local sprite object
  local( sprite_t * sprite );
  // parent actor type
  // since we do not know what the parent's type is,
  // we leave this up to the parent to free itself.
  remote( void * parent );

} ma_actor_t;

#define ma_actor_entity(self)  (self->entity)
#define ma_actor_pos(self)     entity_pos(ma_actor_entity(self))
#define ma_actor_size(self)    entity_size(ma_actor_entity(self))
#define ma_actor_solid(self)   entity_solid(ma_actor_entity(self))
#define ma_actor_dir(self)     (self->dir)
#define ma_actor_gravity(self) (self->gravity)
#define ma_actor_sprite(self)  (self->sprite)
#define ma_actor_velo(self)    (self->velo)
#define ma_actor_parent(self)  (self->parent)

/////////
// new //
/////////

void ma_actor_init( ma_actor_t * self, colormap_t * c, void * p ) {
  
  ma_actor_entity(self)  = entity(self);
  ma_actor_dir(self)     = ma_actor_dir_south();
  ma_actor_gravity(self) = ma_actor_gravity_d();
  ma_actor_velo(self)    = point(0,0);
  ma_actor_sprite(self)  = sprite(c,390,0,10,10);

  ma_actor_parent(self)  = p;
}

ma_actor_t * ma_actor( colormap_t * c, void * p ) {
  ma_actor_t * r = alloc(ma_actor_t);
  ma_actor_init(r,c,p);
  return r;
}

void free_ma_actor( ma_actor_t * self ) {
  free_entity(ma_actor_entity(self));
  free_point(ma_actor_velo(self));
  free_sprite(ma_actor_sprite(self));
  // do not clear the parent.
  free(self);
}

///////////
// funcs //
///////////

void ma_actor_set_dir( ma_actor_t * self, int n ) {
  ma_actor_dir(self) = wrap(n,1,5);
}

void ma_actor_set_gravity( ma_actor_t * self, int g ) {
  ma_actor_gravity(self) = g;
}

////////////
// macros //
////////////

#define ma_actor_set_pos(a,x,y)               entity_set_pos(ma_actor_entity(a),x,y)
#define ma_actor_set_size(a,w,h)              entity_set_size(ma_actor_entity(a),w,h)
#define ma_actor_set_solid(a,b)               entity_set_solid(ma_actor_entity(a),b)
#define ma_actor_set_colormap(a,b)            sprite_set_colormap(ma_actor_sprite(a),b)
#define ma_actor_set_sprite_rect(a,x,y,w,h)   sprite_set_rect(ma_actor_sprite(a),x,y,w,h)
#define ma_actor_set_sprite_origin(a,x,y)     sprite_set_origin(ma_actor_sprite(a),x,y)