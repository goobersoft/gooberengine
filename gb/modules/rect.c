
//////////
// type //
//////////

type() {

  int x;
  int y;
  int w;
  int h;

} rect_t;

#define rect_x(self) (self->x)
#define rect_y(self) (self->y)
#define rect_w(self) (self->w)
#define rect_h(self) (self->h)

/////////
// new //
/////////

void rect_init( rect_t * self, int x, int y, int w, int h ) {
  rect_x(self) = x;
  rect_y(self) = y;
  rect_w(self) = w;
  rect_h(self) = h;
}

rect_t * rect( int x, int y, int w, int h ) {
  rect_t * r = alloc(rect_t);
  rect_init(r,x,y,w,h);
  return r;
}

rect_t make_rect( int x, int y, int w, int h ) {
  rect_t r;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return r;
}

rect_t * clone_rect( rect_t * o ) {
  rect_t * r = alloc(rect_t);
  clone(r,o);
  return r;
}

void free_rect( rect_t * self ) {
  free(self);
}

///////////
// funcs //
///////////

void rect_set( rect_t * self, int x, int y, int w, int h ) {
  rect_x(self) = x;
  rect_y(self) = y;
  rect_w(self) = w;
  rect_h(self) = h;
}

void rect_set_pos( rect_t * self, int x, int y ) {
  rect_x(self) = x;
  rect_y(self) = y;
}

void rect_set_size( rect_t * self, int w, int h ) {
  rect_w(self) = w;
  rect_h(self) = h;
}

void rect_add_pos( rect_t * self, int x, int y ) {
  rect_x(self) += x;
  rect_y(self) += y;
}

void rect_add_size( rect_t * self, int w, int h ) {
  rect_w(self) += w;
  rect_h(self) += h;
}