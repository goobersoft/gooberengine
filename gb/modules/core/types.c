
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

/////////////////////////////
// fixed-fractional number //
/////////////////////////////

#define fixed_min_whole() -1000000000
#define fixed_max_whole()  1000000000
#define fixed_max_part()         1000 // min is 0

type() {
  int whole;
  int part;
} fixed_t;

#define fixed_whole(self) (self->whole)
#define fixed_part(self)  (self->part)

void init_fixed( fixed_t * self, int w, int p ) {
  w = clamp(w,fixed_min_whole(),fixed_max_whole());
  if (w == fixed_max_whole()) {
    // if we reached the maximum, we cannot have a fractional amount
    // over this maximum.
    p = 0;
  }
  else {
    p = wrap(p,0,fixed_max_part());
  }

  fixed_whole(self) = w;
  fixed_part(self)  = p;
}

fixed_t * fixed(int w, int p) {
  fixed_t * self = alloc(fixed_t);
  init_fixed(self,w,p);
  return self;
}

void fixed_set( fixed_t * self, int w, int p ) {
  fixed_whole(self)  = clamp(w,fixed_min_whole(),fixed_max_whole());
  if (w == fixed_max_whole())  p = 0;
  else                        p = wrap(p,0,fixed_max_part());
  fixed_part(self)   = p;
}

void fixed_add( fixed_t * self, int w, int p ) {
  while (p < 0) {
    p += fixed_max_part();
    w  = low(w-1,fixed_min_whole());
  }
  while (p >= fixed_max_part()) {
    p -= fixed_max_part();
    w  = high(w+1,fixed_max_whole());
  }
  fixed_whole(self) = clamp( fixed_whole(self)+w,
    fixed_min_whole(),fixed_max_whole() );
  fixed_part(self) = p;
}

char * fixed_to_string( fixed_t * self ) {

}

/////////////
// array2d //
/////////////

type() {

  int     size;
  int     w, h;
  void ** data;

} array2d_t;

#define array2d_data(self)   (self->data)
#define array2d_width(self)  (self->w)
#define array2d_height(self) (self->h)
#define array2d_size(self)   (self->size)

array2d_t * array2d( int w, int h ) {
  array2d_t * self     = alloc(array2d_t);
  array2d_data(self)   = allocv(void*,w*h);
  array2d_width(self)  = w;
  array2d_height(self) = h;
  array2d_size(self)   = w*h;
  return self;
}

void array2d_set( array2d_t * self, int x, int y, void * d ) {
  int u = wrap( (y*array2d_width(self)) + x, 0, array2d_size(self) );
  array2d_data(self)[ u ] = d;
}

void * array2d_get( array2d_t * self, int x, int y ) {
  int u = wrap( (y*array2d_width(self)) + x, 0, array2d_size(self) );
  return array2d_data(self)[ u ];
}

// helper macros
#define a2set(a,x,y,d) array2d_set(a,x,y,d)
#define a2get(a,x,y)   array2d_get(a,x,y)