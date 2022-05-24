

///////////
// types //
///////////

// it's just an int!!
typedef int color_t;

////////////
// Colors //
////////////
// GB64 uses 64 colors.
// RR GG BB <-- each color channel is two bits.
// transparent color is treated as -1

color_t color(int r, int g, int b) {
  int rt = 0;
  // if ((r <= 0) || (g <= 0) || (b <= 0)) {
  if ( lt(r,0) or lt(g,0) or lt(b,0)  ) {
    return -1;
  }
  else {
    rt = ((r&3)<<4) | ((g&3)<<2) | (b&3);
  }
}

#define color_random() rnd(-1,64)


// returns the red color component
int color_r(color_t c) {
  if (c==-1) return -1;
  return (c&(3<<4))>>4;
}

// returns the green color component
int color_g(color_t c) {
  if (c==-1) return -1;
  return (c&(3<<2))>>2;
}

// returns the blue color component
int color_b(color_t c) {
  if (c==-1) return -1;
  return (c&3);
}