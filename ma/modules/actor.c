
#define actor_dir_up()     1
#define actor_dir_right()  2
#define actor_dir_down()   3
#define actor_dir_left()   4

#define actor_gravity_d()   125

type() {

  entity_t * entity;
  int        dir;
  int        gravity;

} actor_t;

#define actor_entity(self)  (self->entity)
#define actor_pos(self)     (self->entity->pos)
#define actor_size(self)    (self->entity->size)
#define actor_solid(self)   (self->entity->solid)
#define actor_dir(self)     (self->dir)
#define actor_gravity(self) (self->gravity)


/////////
// new //
/////////

void actor_init( actor_t * self ) {
  actor_entity(self)  = entity(0,0,10,10);
  actor_dir(self)     = 0;
  actor_gravity(self) = actor_gravity_d();
}

actor_t * actor() {
  actor_t * r = alloc(actor_t);
  actor_init(r);
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

////////////
// macros //
////////////

#define actor_set_pos(a,x,y)   entity_set_pos(actor_entity(a),x,y)
#define actor_set_size(a,w,h)  entity_set_size(actor_entity(a),w,h)
#define actor_set_solid(a,b)   entity_set_solid(actor_entity(a),b)