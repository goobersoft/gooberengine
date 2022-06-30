
typedef struct {

  int value;
  int low;
  int high;
  int wrappable;

} number_t;

#define number_value(self)      (self->value)
#define number_low(self)        (self->low)
#define number_high(self)       (self->high)
#define number_wrappable(self)  (self->wrappable)

/////////
// new //
/////////

void number_init( number_t * self, int n, int l, int h, bool_t w ) {
  if (l > h) swap(ref(l),ref(h));
  number_low(self)       = l;
  number_high(self)      = h;
  number_value(self)     = clamp(n,l,h);
  number_wrappable(self) = bool(w);
}

number_t * number( int n, int l, int h ) {
  number_t * r = alloc(number_t);
  number_init(r,n,l,h,false());
  return r;
}

number_t * number_w( int n, int l, int h ) {
  number_t * r = alloc(number_t);
  number_init(r,n,l,h,true());
  return r;
}

///////////
// funcs //
///////////



void number_set( number_t * self, int v ) {
  if (number_wrappable(self) == true()) number_value(self) = wrap(v,number_low(self),number_high(self));
  else                                number_value(self) = clamp(v,number_low(self),number_high(self));
}

void number_add( number_t * n, int v ) {
  if (n->wrappable == true()) n->value = wrap(n->value+v,n->low,n->high);
  else                      n->value = clamp(n->value+v,n->low,n->high);
}

void number_mul( number_t * n, int v ) {
  if (n->wrappable == true()) n->value = wrap(n->value*v,n->low,n->high);
  else                      n->value = clamp(n->value*v,n->low,n->high);
}

void number_to_string( number_t * n, char * s ) {
  sprintf(s,"(%d|%d|%d)",number_low(n),number_value(n),number_high(n));
}