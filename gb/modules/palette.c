
////////////
// consts //
////////////

#define palette_size() (10)

////////////
// struct //
////////////

typedef struct {

  color_t * colors;

} palette_t;

#define palette_colors(p) (p->colors)

/////////
// new //
/////////

palette_t * palette() {
  palette_t * r     = alloc(palette_t);
  palette_colors(r) = array(palette_size(),0);
  return r;
}

/////////
// del //
/////////

void free_palette( palette_t * self ) {
  free(palette_colors(self));
  free(self);
}


