
////////////
// struct //
////////////

typedef struct {

  int x;
  int y;

} point_t;

#define point_x(self) (self->x)
#define point_y(self) (self->y)

/////////
// new //
/////////

void point_init( point_t * self, int x, int y ) {
  point_x(self) = x;
  point_y(self) = y;
}

point_t * point(int x, int y) {
  point_t * r = alloc(point_t);
  point_init(r,x,y);
  return r;
}

point_t * clone_point( point_t * o ) {
  point_t * r = alloc(point_t);
  clone(r,o);
  return r;
}

void free_point( point_t * self ) {
  free(self);
}

///////////
// funcs //
///////////

void point_copy(point_t * self, point_t * other) {
  point_x(self) = point_x(other);
  point_y(self) = point_y(other);
}

void point_set(point_t * self, int x, int y) {
  point_x(self) = x;
  point_y(self) = y;
}

void point_add(point_t * self, int x, int y) {
  point_x(self) = point_x(self) + x;
  point_y(self) = point_y(self) + y;
}

void point_mul(point_t * self, int x, int y) {
  point_x(self) = point_x(self) * x;
  point_y(self) = point_y(self) * y;
}

void point_set_p( point_t * self, point_t * other ) {
  point_x(self) = point_x(other);
  point_y(self) = point_y(other);
}

void point_add_p( point_t * self, point_t * other ) {
  point_x(self) += point_x(other);
  point_y(self) += point_y(other);
}

void point_mul_p( point_t * self, point_t * other ) {
  point_x(self) *= point_x(other);
  point_y(self) *= point_y(other);
}

bool_t point_in_rect( point_t * self, int x, int y, int w, int h ) {
  return inrect(point_x(self), point_y(self), x, y, x+w, y+h);
}

void point_clamp( point_t * self, int x, int y, int w, int h ) {
  point_x(self) = clamp(point_x(self),x,x+w-1);
  point_y(self) = clamp(point_y(self),y,y+h-1);
}