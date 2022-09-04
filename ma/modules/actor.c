
type() {


  entity_t * entity;

  // the direction, if applicable
  // different contexts may utilize this in diff ways.
  // (i.e. left=1, right=2, or an angle 0-999)
  int dir;

} actor_t;

#define actor_entity(self) (self->entity)
#define actor_pos(self)    (self->entity->pos)
#define actor_size(self)   (self->entity->size)
#define actor_solid(self)  (self->entity->solid)
#define actor_dir(self)    (self->dir)


/////////
// new //
/////////

void actor_init( actor_t * self ) {
  actor_entity(self)  = entity(0,0,10,10);
  actor_dir(self)     = 0;
}

actor_t * actor() {
  actor_t * r = alloc(actor_t);
  actor_init(r);
  return r;
}

///////////
// funcs //
///////////

/*
void actor_set_pos( actor_t * self, int x, int y ) {
  point_set( actor_pos(self), x, y );
}

void actor_set_size( actor_t * self, int w, int h ) {
  point_set( actor_size(self), low(w,1), low(h,1) );
}

void actor_set_dir( actor_t * self, int d ) {
  actor_dir(self) = d;
}

void actor_set_solid( actor_t * self, int s ) {
  actor_solid(self) = bool(s);
}

bool_t actor_collide( actor_t * self, collide_t * other ) {
  
}

bool_t actor_collide_actor( actor_t * self, actor_t * other ) {
  return rectinrect2( 
    self->pos.x, self->pos.y, self->size.x, self->size.y,
    other->pos.x, other->pos.y, other->size.x, other->size.y );
}
*/