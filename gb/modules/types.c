
/////////////
// integer //
/////////////

#define integer_max_value()  2147483647
#define integer_min_value() -2147483648

type() {

  int value;

} integer_t;

#define integer_value(self) (self->value)

// -- new -- //

integer_t * integer( int n ) {
  integer_t * r = alloc(integer_t);
  integer_value(r) = n;
  return r;
}

// -- funcs -- //

void integer_set( integer_t * self, int n ) {
  integer_value(self) = n;
}

void integer_add( integer_t * self, int n ) {
  integer_value(self) += n;
}

void integer_mul( integer_t * self, int n ) {
  integer_value(self) *= n;
}

void integer_sub( integer_t * self, int n ) {
  integer_value(self) -= n;
}

void integer_div( integer_t * self, int n ) {
  if (n == 0) {
    if (integer_value(self) >= 0) {
      integer_value(self) = integer_max_value();
    }
    else {
      integer_value(self) = integer_min_value();
    }
  }
  else {
    integer_value(self) /= n;
  }
}

void integer_frac( integer_t * self, int n, int d ) {
  integer_mul(self,n);
  integer_div(self,d);
}

char * integer_to_string( integer_t * self ) {
  return str(integer_value(self));
}

/////////////
// boolean //
/////////////

type() {

  bool_t value;

} boolean_t;

#define boolean_value(self) (self->value)

// -- new -- //

boolean_t * boolean( bool_t b ) {
  boolean_t * r = alloc(boolean_t);
  boolean_value(r) = bool(b);
  return r;
}

// -- funcs -- //

void boolean_set( boolean_t * self, bool_t b ) {
  boolean_value(self) = bool(b);
}

void boolean_toggle( boolean_t * self ) {
  boolean_value(self) = not boolean_value(self);
}

char * boolean_to_string( boolean_t * self ) {
  return (boolean_value(self)==true()) ? "true" : "false";
}