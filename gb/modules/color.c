

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

#define color_r(c)      c.r
#define color_g(c)      c.g
#define color_b(c)      c.b
#define color_a(c)      c.a

////////////
// Colors //
////////////
// GB64 uses 64 colors.
// RR GG BB <-- each color channel is two bits.
// transparent color is treated as -1

color_t color(int r, int g, int b) {  
  color_t rr;
  rr.r = clamp(r,0,3);
  rr.g = clamp(g,0,3);
  rr.b = clamp(b,0,3);
  rr.a = true();
  return rr;
}

#define color_random()  color(rnd(0,4),rnd(0,4),rnd(0,4))

color_t color__rgba(int r, int g, int b, int a) {
  color_t rr;
  rr.r = clamp(r,0,3);
  rr.g = clamp(g,0,3);
  rr.b = clamp(b,0,3);
  rr.a = clamp(a,0,1);
  return rr;
}

fnptr( 
  color_rgba, color__rgba,
  color_t, (int,int,int,int)
);

#define color_trans()   color__rgba(0,0,0,0)

color_t color__index( int id ) {
  if (id == 64) return color_trans();
  color_t rr;
  if (id >= 0) {
    rr.r = id % 4;
    rr.g = id / 4;
    rr.b = id / 16;
    rr.a = true();
  }
  return rr;
}

fnptr(
  color_index, color__index,
  color_t, (int)
);

int color_eq(color_t a, color_t b) {
  return (a.r==b.r) && (a.g==b.g) && (a.b==b.b);
}

color_t color_set( color_t c1, color_t c2 ) {
  if (c1.a||c2.a) {
    return color(c2.r, c2.g, c2.b);
  }
  return color_trans();
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

color_t color_high( color_t c1, color_t c2 ) {
  return color(
    high(color_r(c1),color_r(c2)),
    high(color_g(c1),color_g(c2)),
    high(color_b(c1),color_b(c2))
  );
}

color_t color_low( color_t c1, color_t c2 ) {
  return color(
    low(color_r(c1),color_r(c2)),
    low(color_g(c1),color_g(c2)),
    low(color_b(c1),color_b(c2))
  );
}

color_t color_avg( color_t c1, color_t c2 ) {
  return color(
    (color_r(c1)+color_r(c2))/2,
    (color_g(c1)+color_g(c2))/2,
    (color_b(c1)+color_b(c2))/2
  );
}