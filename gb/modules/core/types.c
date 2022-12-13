
/////////////
// integer //
/////////////
// in the event that we want to haved an integer
// which can be allocated like other objects, this is the
// type we use.

// range constants
#define integer_max_value()  2147483647
#define integer_min_value() -2147483648

// type
type() {

  int value;

} integer_t;

// getters
#define integer_value(self) (self->value)



//-- new --//

// not really necessary since it has just one field,
// but kept for conventional purposes.
void integer_init( integer_t * self, int n ) {
  integer_value(self) = n;
}

integer_t * integer( int n ) {
  integer_t * r = alloc(integer_t);
  integer_init(r,n);
  return r;
}

void free_integer( integer_t * self ) {
  free(self);
}

//-- funcs --//

void integer_set( integer_t * self, int n ) {
  integer_value(self) = n;
}

void integer_add( integer_t * self, int n ) {
  integer_value(self) = clamp(integer_value(self)+n,integer_min_value(),integer_max_value());
}

void integer_mul( integer_t * self, int n ) {
  integer_value(self) = clamp(integer_value(self)*n,integer_min_value(),integer_max_value());
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

// this function utilizes the additional bit space of the
// frac function, which internally uses a long integer.
// frac(a,b,c) uses the expression ((a*b)/c).
void integer_frac( integer_t * self, int n, int d ) {
  integer_value(self) = frac( integer_value(self), n, d );
}

// using the str() function, convert the value to a string
// representation. Keep in mind that the value itself is stored
// internally and is not your own as subsequent calls to str() will
// overwrite the previous contents. If you'd like to keep the value,
// copy the contents to another string, or use string_from(), which makes
// a string object.
char * integer_to_string( integer_t * self ) {
  return str(integer_value(self));
}

/////////////
// boolean //
/////////////
// 

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

// unlike integer_t, boolean_t will give you a string
// constant of "true" or "false" depending on its value.
// No need to allocate additional memory for each value as
// the 'true' and 'false' values here always have the same
// memory address.
char * boolean_to_string( boolean_t * self ) {
  return (boolean_value(self)==true()) ? "true" : "false";
}