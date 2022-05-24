
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

void point_init(point_t * p, int x, int y) {
  point_x(p) = x;
  point_y(p) = y;
}

point_t * point(int x, int y) {
  point_t * r = alloc(point_t);
  point_init(r,x,y);
  return r;
}



//////////
// free //
//////////

#define free_point(self) free(self)

///////////
// funcs //
///////////

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