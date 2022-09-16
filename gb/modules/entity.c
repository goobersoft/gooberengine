
//////////
// type //
//////////

type() {

  point_t   pos;
  point_t   size;
  bool_t    solid;

} entity_t;

#define entity_pos(self)     ref(self->pos)
#define entity_size(self)    ref(self->size)
#define entity_solid(self)   (self->solid)

#define entity_pos_x(self)   point_x(entity_pos(self))
#define entity_pos_y(self)   point_y(entity_pos(self))
#define entity_size_x(self)  point_x(entity_size(self))
#define entity_size_y(self)  point_y(entity_size(self))

#define entity_make_rect(self) make_rect(entity_pos(self).x,entity_pos(self).y,entity_size(self).x,entity_size(self).y)

/////////
// new //
/////////

void entity_init( entity_t * self ) {
  point_set          (entity_pos(self),0,0);
  point_set          (entity_size(self),10,10);
  entity_solid(self) = true();
}

entity_t * entity() {
  entity_t * r = alloc(entity_t);
  entity_init(r);
  return r;
}

///////////
// funcs //
///////////

void entity_set_pos( entity_t * self, int x, int y ) {
  point_set(entity_pos(self),x,y);
}

void entity_set_size( entity_t * self, int w, int h ) {
  point_set(entity_size(self),w,h);
}

void entity_set_solid( entity_t * self, bool_t b ) {
  entity_solid(self) = bool(b);
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