
typedef struct {

  int    value;
  int    low;
  int    high;
  bool_t f_wrap;

} number_t;

#define number_value(self)      (self->value)
#define number_low(self)        (self->low)
#define number_high(self)       (self->high)
#define number_f_wrap(self)     (self->f_wrap)

/////////
// new //
/////////

void number_init( number_t * self, int n, int l, int h, bool_t w ) {
  if (l > h) swap(ref(l),ref(h));
  number_low(self)       = l;
  number_high(self)      = h;
  number_value(self)     = clamp(n,l,h);
  number_f_wrap(self)    = bool(w);
}

// clamping number
number_t * number( int n, int l, int h ) {
  number_t * r = alloc(number_t);
  number_init(r,n,l,h,false());
  return r;
}

// wrapping number
number_t * number_w( int n, int l, int h ) {
  number_t * r = alloc(number_t);
  number_init(r,n,l,h,true());
  return r;
}

///////////
// funcs //
///////////

void number_set( number_t * self, int v ) {
  if (number_f_wrap(self) == true())  number_value(self) = wrap(v,number_low(self),number_high(self));
  else                                number_value(self) = clamp(v,number_low(self),number_high(self));
}

void number_add( number_t * self, int v ) {
  if (number_f_wrap(self) == true()) number_value(self) = wrap(number_value(self)+v,number_low(self),number_high(self));
  else                               number_value(self) = clamp(number_value(self)+v,number_low(self),number_high(self));
}

void number_mul( number_t * self, int v ) {
  if (number_f_wrap(self) == true()) number_value(self) = wrap(number_value(self)*v,number_low(self),number_high(self));
  else                               number_value(self) = clamp(number_value(self)*v,number_low(self),number_high(self));
}

void number_to_string( number_t * self, char * s ) {
  sprintf(s,"(%d|%d|%d)",number_low(self),number_value(self),number_high(self));
}

void number_set_bounds( number_t * self, int l, int h ) {
  number_low(self) = l;
  number_high(self) = h;
  if (number_f_wrap(self))  number_value(self) = wrap(number_value(self),number_low(self),number_high(self));
  else                      number_value(self) = clamp(number_value(self),number_low(self),number_high(self));
}

void number_set_wrappable( number_t * self, bool_t w ) {
  number_f_wrap(self) = bool(w);
}