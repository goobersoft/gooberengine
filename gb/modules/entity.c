
//////////
// type //
//////////

type() {

  local( point_t * pos );
  local( point_t * size );
  bool_t solid;

  // a pointer to the parent object
  remote( void * parent );

} entity_t;

#define entity_pos(self)     (self->pos)
#define entity_size(self)    (self->size)
#define entity_solid(self)   (self->solid)
#define entity_parent(self)  (self->parent)

#define entity_pos_x(self)   point_x(entity_pos(self))
#define entity_pos_y(self)   point_y(entity_pos(self))
#define entity_size_x(self)  point_x(entity_size(self))
#define entity_size_y(self)  point_y(entity_size(self))

#define entity_make_rect(self) make_rect(entity_pos(self).x,entity_pos(self).y,entity_size(self).x,entity_size(self).y)

//#define entity_set_parent(self,p)  set(self,parent,p)
#define entity_set_pos(self,x,y)   point_set(entity_pos(self),x,y)
#define entity_set_size(self,w,h)  point_set(entity_size(self),w,h)
#define entity_set_solid(self,b)   set(self,solid,bool(b))

/////////
// new //
/////////

void entity_init( entity_t * self, void * p ) {
  entity_pos(self)         = point(0,0);
  entity_size(self)        = point(10,10);
  entity_solid(self)       = true();
  entity_parent(self)      = p;
}

entity_t * entity( void * p ) {
  entity_t * r = alloc(entity_t);
  entity_init(r,p);
  return r;
}

void free_entity( entity_t * self ) {
  free_point(entity_pos(self));
  free_point(entity_size(self));
  free(self);
}

///////////
// funcs //
///////////

// setting a parent for this entity object would be useful for
// knowing the owner actor of this entity. The reason why the
// pointer is void is because different types may use the
// entity type for collisions.
void entity_set_parent( entity_t * self, void * p ) {
  entity_parent(self) = p;
}

bool_t entity_check_collide( entity_t * c1, entity_t * c2 ) {
  if (entity_solid(c1) and entity_solid(c2)) {
    return rectinrect2(
      entity_pos_x(c1), entity_pos_y(c1), entity_size_x(c1), entity_size_y(c1),
      entity_pos_x(c2), entity_pos_y(c2), entity_size_x(c2), entity_size_y(c2)
    );
  }
}

// attempts to glide horizontally while checking against another entity.
int entity_glide_h( entity_t * self, int n, entity_t * other ) {
}

// attempts to glide vertically while checking against another entity.
int entity_glide_v( entity_t * self, int n, entity_t * other ) {
}

int entity_glide_h_list( entity_t * self, int n, list_t * l ) {
}

int entity_glide_v_hist( entity_t * self, int n, list_t * l ) {
}

int entity_glide_hv( entity_t * self, int dx, int dy, entity_t * other ) {
}

int entity_glide_vh( entity_t * self, int dx, int dy, entity_t * other ) {
}

int entity_glide_hv_list( entity_t * self, int dx, int dy, list_t * l ) {
}

int entity_glide_vh_list( entity_t * self, int dx, int dy, list_t * l ) {
}