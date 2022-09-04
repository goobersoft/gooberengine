
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

/////////
// new //
/////////

entity_t * entity( int x, int y, int w, int h ) {
  entity_t * r = alloc(entity_t);
  point_set       (entity_pos(r),x,y);
  point_set       (entity_size(r),w,h);
  entity_solid(r) = true();
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

bool_t entity_check( entity_t * c1, entity_t * c2 ) {
  return rectinrect2(
    entity_pos_x(c1), entity_pos_y(c1), entity_size_x(c1), entity_size_y(c1),
    entity_pos_x(c2), entity_pos_y(c2), entity_size_x(c2), entity_size_y(c2)
  );
}