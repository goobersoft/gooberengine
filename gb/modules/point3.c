
////////////
// struct //
////////////

typedef struct {

  int x;
  int y;
  int z;

} point3_t;

#define point3_x(self) (self->x)
#define point3_y(self) (self->y)
#define point3_z(self) (self->z)

/////////
// new //
/////////

void point3_init( point3_t * self, int x, int y, int z ) {
  point3_x(self) = x;
  point3_y(self) = y;
  point3_z(self) = z;
}

point3_t * point3(int x, int y, int z) {
  point3_t * r = alloc(point3_t);
  point3_init(r,x,y,z);
  return r;
}

///////////
// funcs //
///////////

void point3_set(point3_t * self, int x, int y, int z) {
  point3_x(self) = x;
  point3_y(self) = y;
  point3_z(self) = z;
}

void point3_add(point3_t * self, int x, int y) {
  point3_x(self) = point3_x(self) + x;
  point3_y(self) = point3_y(self) + y;
  point3_z(self) = point3_z(self) + z;
}

void point3_mul(point3_t * self, int x, int y, int z) {
  point3_x(self) = point3_x(self) * x;
  point3_y(self) = point3_y(self) * y;
  point3_z(self) = point3_z(self) * z;
}