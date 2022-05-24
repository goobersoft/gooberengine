
/////////
// int //
/////////

type() {
  int value;
} integer_t;

#define integer_value(self) (self->value)
#define integer_get(self)   integer_value(self)

integer_t * integer( int x ) {
  integer_t * r = alloc(integer_t);
  integer_value(r) = x;
  return r;
}

void integer_set( integer_t * u, int x ) {
  integer_value(u) = x;
}

void integer_add( integer_t * u, int x ) {
  integer_value(u) += x;
}

void integer_mul( integer_t * u, int x ) {
  integer_value(u) *= x;
}

void integer_div( integer_t * u, int x ) {
  if (x==0) integer_value(u) = 0;
  integer_value(u) /= x;
}

//////////////////////////
// fixed floating point //
//////////////////////////

typedef int fixed_t;