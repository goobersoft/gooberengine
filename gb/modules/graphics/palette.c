
////////////
// consts //
////////////

#define palette_max_colors()   65
#define palette_index_trans()  palette_max_colors()-1

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

palette_t * palette(void) {
  palette_t * r     = alloc(palette_t);
  int cr = 0;
  int cg = 0;
  int cb = 0;
  loop(i,palette_max_colors()) {
    palette_colors(r)[i] = make_color(cr,cg,cb);
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
  palette_colors(r)[palette_index_trans()] = make_color_trans();
  return r;
}

palette_t * palette__grayscale() {
  palette_t * r = alloc(palette_t);
  int cg = 0;
  loop(i,palette_max_colors()) {
    cg = i%4;
    palette_colors(r)[i] = make_color(cg,cg,cg);
  }
  palette_colors(r)[palette_index_trans()] = make_color_trans();
  return r;
}

palette_t * palette__gameboy() {
  static color_t _cl[4];
  static bool_t  _init;

  // first call, needs initializing
  if (_init==false()) {
    _cl[0] = make_color_index(4);
    _cl[1] = make_color_index(9);
    _cl[2] = make_color_index(14);
    _cl[3] = make_color_index(47);
    _init  = true();
  }

  palette_t * r = alloc(palette_t);

  color_t cg;
  loop(i,palette_max_colors()) {
    cg = _cl[i%4];
    palette_colors(r)[i] = cg;
  }
  palette_colors(r)[palette_index_trans()] = make_color_trans();
  return r;
}

palette_t * palette__random() {
  palette_t * r = alloc(palette_t);
  loop(i,palette_max_colors()) {
    palette_colors(r)[i] = make_color_random();
  }
  palette_colors(r)[palette_index_trans()] = make_color_trans();
  return r;
}

///////////
// funcs //
///////////

// sets the palette colors to another array of colors
// (copies)
void palette_set_colors( palette_t * self, color_t * cs ) {
  loop(i,palette_max_colors()) {
    palette_colors(self)[i] = cs[i];
  }
}

void palette_copy( palette_t * self, palette_t * ot ) {
  loop(i,palette_max_colors()) {
    palette_colors(self)[i] = palette_colors(ot)[i];
  }
}

void palette_set_color( palette_t * self, int n, color_t c ) {
  if inrange(n,0,palette_max_colors()) {
    palette_colors(self)[n] = c;
  }
}

color_t palette_get_color( palette_t * self, int n ) {
  n = wrap(n,0,palette_max_colors());
  return palette_colors(self)[n];
}