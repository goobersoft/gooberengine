
////////////
// consts //
////////////

#define palette_max_colors() 64

////////////
// struct //
////////////

typedef struct {

  color_t colors[palette_max_colors()];

} palette_t;

#define palette_colors(p) (p->colors)

/////////
// new //
/////////

palette_t * palette() {
  palette_t * r     = alloc(palette_t);
  int cr = 0;
  int cg = 0;
  int cb = 0;
  loop(i,0,palette_max_colors()) {
    palette_colors(r)[i] = color(cr,cg,cb);
    cr += 1;
    if (cr==4) {
      cr = 0;
      cg += 1;
    }
    if (cg==4) {
      cg = 0;
      cb += 1;
    }
  }
  return r;
}

palette_t * palette__grayscale() {
  palette_t * r = alloc(palette_t);
  int cg = 0;
  loop(i,0,palette_max_colors()) {
    cg = i%4;
    palette_colors(r)[i] = color(cg,cg,cg);
  }
  return r;
}

palette_t * palette__gameboy() {
  static color_t _cl[4];
  static bool_t  _init;

  // first call, needs initializing
  if (not _init) {
    _cl[0] = color__index(4);
    _cl[1] = color__index(9);
    _cl[2] = color__index(14);
    _cl[3] = color__index(47);
    _init  = true();
  }

  palette_t * r = alloc(palette_t);

  color_t cg;
  loop(i,0,palette_max_colors()) {
    cg = _cl[i%4];
    palette_colors(r)[i] = cg;
  }

  return r;
}

palette_t * palette__random() {
  palette_t * r = alloc(palette_t);
  loop(i,0,palette_max_colors()) {
    palette_colors(r)[i] = color_random();
  }
  return r;
}

/////////
// del //
/////////

void free_palette( palette_t * self ) {
  free(palette_colors(self));
  free(self);
}

///////////
// funcs //
///////////

// sets the palette colors to another array of colors
// (copies)
void palette_set( palette_t * self, color_t * cs ) {
  loop(i,0,palette_max_colors()) {
    palette_colors(self)[i] = cs[i];
  }
}

void palette_copy( palette_t * self, palette_t * ot ) {
  loop(i,0,palette_max_colors()) {
    palette_colors(self)[i] = palette_colors(ot)[i];
  }
}

void palette_set_color( palette_t * self, int n, color_t c ) {
  if (inrange(n,0,palette_max_colors())) {
    palette_colors(self)[n] = c;
  }
}