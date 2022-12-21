
//////////
// type //
//////////

type() {

  point_t pos;
  point_t size;

} rect_t;

#define rect_pos(self)  ref(self->pos)
#define rect_size(self) ref(self->size)

#define rect_x(self) (self->pos.x)
#define rect_y(self) (self->pos.y)
#define rect_w(self) (self->size.x)
#define rect_h(self) (self->size.y)

/////////
// new //
/////////

void rect_init( rect_t * self, int x, int y, int w, int h ) {
  init_point( rect_pos(self), x, y );
  init_point( rect_size(self), w, h );
}

rect_t * rect( int x, int y, int w, int h ) {
  rect_t * r = alloc(rect_t);
  rect_init(r,x,y,w,h);
  return r;
}

rect_t make_rect( int x, int y, int w, int h ) {
  rect_t r;
  r.pos.x = x;
  r.pos.y = y;
  r.size.x = w;
  r.size.y = h;
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
  point_set( rect_pos(self), x, y );
  point_set( rect_size(self), w, h );
}

void rect_set_pos( rect_t * self, int x, int y ) {
  point_set( rect_pos(self), x, y );
}

void rect_set_size( rect_t * self, int w, int h ) {
  point_set( rect_size(self), w, h );
}

void rect_add_pos( rect_t * self, int x, int y ) {
  point_add( rect_pos(self), x, y );
}

void rect_add_size( rect_t * self, int w, int h ) {
  point_add( rect_size(self), w, h );
}

bool_t rect_collide( rect_t * self, rect_t * other ) {
  return rectinrect2( rect_x(self), rect_y(self), rect_w(self), rect_h(self), 
    rect_x(other), rect_y(other), rect_w(other), rect_h(other) );
}