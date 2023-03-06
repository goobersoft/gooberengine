
///////////////
// ui slider //
///////////////

type() {

  // contains min, max, and current value
  number_t *  values;
  // sets how much the slider moves when nudged.
  int         slideamt;
  // for gui applications, determine if this slider is meant to be
  // vertical or not (for up and down commands). this just redirects to
  // left and right respectively.
  bool_t      isvertical;

} slider_t;

#define slider_values(self)     (self->values)
#define slider_slideamt(self)   (self->slideamt)
#define slider_isvertical(self) (self->isvertical)

/////////
// new //
/////////

void init_slider( slider_t * self, int v, int l, int h ) {
  slider_values(self)     = number(v,l,h);
  slider_slideamt(self)   = 1;
  slider_isvertical(self) = false();
}

slider_t * slider( int v, int l, int h ) {
  slider_t * self = alloc(slider_t);
  init_slider(self,v,l,h);
  return self;
}

///////////////
// functions //
///////////////

void slider_set( slider_t * self, int n ) {
  number_set( slider_values(self), n );
}

void slider_set_bounds( slider_t * self, int l, int h ) {
  number_set_bounds( slider_values(self), l, h );
}

void slider_set_slide_amt( slider_t * self, int n ) {
  n = low(n,1);
  slider_slideamt(self) = n;
}

void slider_left( slider_t * self ) {
  number_add( slider_values(self), -slider_slideamt(self) );
}

void slider_right( slider_t * self ) {
  number_add( slider_values(self), slider_slideamt(self) );
}

void slider_up( slider_t * self ) {
  if (slider_isvertical(self)==false()) {
    slider_left(self);
  }
}

void slider_down( slider_t * self ) {
  if (slider_isvertical(self)==false()) {
    slider_right(self);
  }
}