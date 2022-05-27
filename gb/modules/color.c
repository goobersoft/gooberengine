

///////////
// types //
///////////

// it's just an int!!
type() {
  byte_t r:2;
  byte_t g:2;
  byte_t b:2;
  byte_t a:1;
} color_t;

////////////
// Colors //
////////////
// GB64 uses 64 colors.
// RR GG BB <-- each color channel is two bits.
// transparent color is treated as -1

color_t color(int r, int g, int b) {
  r = clamp(r,0,3);
  g = clamp(g,0,3);
  b = clamp(b,0,3);
  
  color_t rr;

  rr.r = r;
  rr.g = g;
  rr.b = b;
  rr.a = true();

  return rr;
}

color_t color__rgba(int r, int g, int b, int a) {
  r = clamp(r,0,3);
  g = clamp(g,0,3);
  b = clamp(b,0,3);
  a = clamp(a,0,1);
  
  color_t rr;

  rr.r = r;
  rr.g = g;
  rr.b = b;
  rr.a = a;

  return rr;
}

color_t color__trans() {
  return color__rgba(0,0,0,0);
}

#define color_random()  color(rnd(0,4),rnd(0,4),rnd(0,4))
#define color_r(c)      c.r
#define color_g(c)      c.g
#define color_b(c)      c.b
#define color_a(c)      c.a
#define color_trans(c)  color_a(c)

int color_eq(color_t a, color_t b) {
  return (a.r==b.r) && (a.g==b.g) && (a.b==b.b);
}

color_t color_add( color_t c1, color_t c2 ) {
  return color(
    color_r(c1) + color_r(c2),
    color_g(c1) + color_g(c2),
    color_b(c1) + color_b(c2)
  );
}

color_t color_sub( color_t c1, color_t c2 ) {
  return color(
    color_r(c1) - color_r(c2),
    color_g(c1) - color_g(c2),
    color_b(c1) - color_b(c2)
  );
}

color_t color_mask( color_t c1, color_t c2 ) {
  return color(
    high(color_r(c2),color_r(c1)),
    high(color_g(c2),color_g(c1)),
    high(color_b(c2),color_b(c1))
  );
}