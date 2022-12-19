
//////////
// type //
//////////

type() {
  
  local( rect_t * rect );

  // a boolean for solidity.
  // two entities may collide only if both of them are solid.
  bool_t solid;

  // tag
  local( tag_t * tag );
  // a pointer to the parent object
  foreign( void * parent );

} entity_t;

#define entity_rect(self)    (self->rect)
#define entity_solid(self)   (self->solid)
#define entity_tag(self)     (self->tag)
#define entity_parent(self)  (self->parent)

#define entity_id(self)      tag_id(self->tag)
#define entity_pos_x(self)   rect_x(entity_rect(self))
#define entity_pos_y(self)   rect_y(entity_rect(self))
#define entity_size_x(self)  rect_w(entity_rect(self))
#define entity_size_y(self)  rect_h(entity_rect(self))

//#define entity_set_parent(self,p)  set(self,parent,p)
#define entity_set_pos(self,x,y)   rect_set_pos(entity_rect(self),x,y)
#define entity_set_size(self,w,h)  rect_set_size(entity_rect(self),w,h)
#define entity_set_solid(self,b)   set(self,solid,bool(b))

/////////
// new //
/////////

void entity_init( entity_t * self, void * p ) {
  entity_rect(self)        = rect(0,0,10,10);
  entity_solid(self)       = true();
  entity_tag(self)         = tag(self,"entity");
  entity_parent(self)      = p;
}

entity_t * entity( void * p ) {
  entity_t * r = alloc(entity_t);
  entity_init(r,p);
  return r;
}

void free_entity( entity_t * self ) {
  free_rect(entity_rect(self));
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
 
 // check if two entities are overlapping
bool_t entity_collide( entity_t * self, entity_t * other ) {
  if (entity_solid(self) and entity_solid(other)) {
    return rect_collide( entity_rect(self), entity_rect(other) );
    /*
    return rectinrect2(
      entity_pos_x(c1), entity_pos_y(c1), entity_size_x(c1), entity_size_y(c1),
      entity_pos_x(c2), entity_pos_y(c2), entity_size_x(c2), entity_size_y(c2)
    );
    */
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