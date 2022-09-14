
#define actor_dir_north()   1
#define actor_dir_east()    2
#define actor_dir_south()   3
#define actor_dir_west()    4

#define actor_gravity_d()   125

type() {

  entity_t * entity;
  int        dir;
  point_t  * velo;
  int        gravity;
  sprite_t * sprite;

} actor_t;

#define actor_entity(self)  (self->entity)
#define actor_pos(self)     entity_pos(actor_entity(self))
#define actor_size(self)    entity_size(actor_entity(self))
#define actor_solid(self)   entity_solid(actor_entity(self))
#define actor_dir(self)     (self->dir)
#define actor_gravity(self) (self->gravity)
#define actor_sprite(self)  (self->sprite)
#define actor_velo(self)    (self->velo)

/////////
// new //
/////////

void actor_init( actor_t * self, colormap_t * c ) {
  actor_entity(self)  = entity(0,0,10,10);
  actor_dir(self)     = actor_dir_south();
  actor_gravity(self) = actor_gravity_d();
  actor_velo(self)    = point(0,0);
  actor_sprite(self)  = sprite(c,390,0,10,10);
}

actor_t * actor( colormap_t * c ) {
  actor_t * r = alloc(actor_t);
  actor_init(r,c);
  return r;
}

///////////
// funcs //
///////////

void actor_set_dir( actor_t * self, int n ) {
  actor_dir(self) = wrap(n,1,5);
}

void actor_set_gravity( actor_t * self, int g ) {
  actor_gravity(self) = g;
}

////////////
// events //
////////////

void actor_update( actor_t * self ) {
}

void actor_draw( actor_t * self ) {
}

////////////
// macros //
////////////

#define actor_set_pos(a,x,y)               entity_set_pos(actor_entity(a),x,y)
#define actor_set_size(a,w,h)              entity_set_size(actor_entity(a),w,h)
#define actor_set_solid(a,b)               entity_set_solid(actor_entity(a),b)
#define actor_set_colormap(a,b)            sprite_set_colormap(actor_sprite(a),b)
#define actor_set_sprite_rect(a,x,y,w,h)   sprite_set_rect(actor_sprite(a),x,y,w,h)
#define actor_set_sprite_origin(a,x,y)     sprite_set_origin(actor_sprite(a),x,y)