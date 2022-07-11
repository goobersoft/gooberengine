
//////////
// type //
//////////

type() {

  point_t  pos;
  point_t  size;

} collide_t;

#define collide_pos(self)     ref(self->pos)
#define collide_size(self)    ref(self->size)

#define collide_pos_x(self)   point_x(collide_pos(self))
#define collide_pos_y(self)   point_y(collide_pos(self))
#define collide_size_x(self)  point_x(collide_size(self))
#define collide_size_y(self)  point_y(collide_size(self))

/////////
// new //
/////////

collide_t * collide( int x, int y, int w, int h ) {
  collide_t * r = alloc(collide_t);
  point_set(collide_pos(r),x,y);
  point_set(collide_size(r),w,h);
  return r;
}

///////////
// funcs //
///////////

void collide_set_pos( collide_t * self, int x, int y ) {
  point_set(collide_pos(self),x,y);
}

void collide_set_size( collide_t * self, int w, int h ) {
  point_set(collide_size(self),w,h);
}

bool_t collide_check( collide_t * c1, collide_t * c2 ) {
  return rectinrect2(
    collide_pos_x(c1), collide_pos_y(c1), collide_size_x(c1), collide_size_y(c1),
    collide_pos_x(c2), collide_pos_y(c2), collide_size_x(c2), collide_size_y(c2)
  );
}