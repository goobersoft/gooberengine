
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