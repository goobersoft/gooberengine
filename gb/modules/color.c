

///////////
// types //
///////////

// it's just one byte!!
type() {
  byte_t r:2; // red component   [0-3]
  byte_t g:2; // green component [0-3]
  byte_t b:2; // blue component  [0-3]
  byte_t a:1; // alpha flag      [0-1]
  byte_t i:1; // intensity flag  [0-1]
} color_t;

#define color_r(c)      c->r
#define color_g(c)      c->g
#define color_b(c)      c->b
#define color_a(c)      c->a

#define get_color_r(c)  c.r
#define get_color_g(c)  c.g
#define get_color_b(c)  c.b
#define get_color_a(c)  c.a

////////////
// Colors //
////////////
// GB64 uses 64 colors.
// RR GG BB <-- each color channel is two bits.
// transparent color is treated as -1

void color_init( color_t * self, byte_t r, byte_t g, byte_t b, byte_t a ) {
  color_r(self) = r&3;
  color_g(self) = g&3;
  color_b(self) = b&3;
  color_a(self) = a&1;
}

color_t * color(byte_t r, byte_t g, byte_t b) {  
  color_t  * rr = alloc(color_t);
  color_init( rr, r, g, b, 1 );
  return rr;
}

color_t * color_from( color_t c ) {
  color_t  * rr = alloc(color_t);
  color_r(rr) = c.r;
  color_g(rr) = c.g;
  color_b(rr) = c.b;
  color_a(rr) = c.a;
  return rr;
}

// copy, not alloc.
color_t make_color( byte_t rr, byte_t gg, byte_t bb ) {
  color_t r;
  r.r = rr&3;
  r.g = gg&3;
  r.b = bb&3;
  r.a = 1;
  return r;
}

#define make_color_random() \
  make_color(rnd(0,4),rnd(0,4),rnd(0,4))

color_t make_color_rgba(byte_t r, byte_t g, byte_t b, byte_t a) {
  color_t rr;
  rr.r = r&3;
  rr.g = g&3;
  rr.b = b&3;
  rr.a = a&1;
  return rr;
}

#define make_color_trans() \
  make_color_rgba(0,0,0,0)

color_t make_color_index( int id ) {
  if (id == 64) return make_color_trans();
  color_t rr;
  if (id >= 0) {
    rr.r = (id >> 4) & 3;
    rr.g = (id >> 2) & 3;
    rr.b = id & 3;
    rr.a = true();
  }
  return rr;
}

///////////
// funcs //
///////////

int color_eq( color_t * a, color_t * b ) {
  return (color_r(a)==color_r(b)) && (color_g(a)==color_g(b)) && (color_b(a)==color_b(b));
}

int check_color_eq(color_t a, color_t b) {
  return (a.r==b.r) && (a.g==b.g) && (a.b==b.b);
}

int color_to_index( color_t c ) {
  if (c.a==0) return -1;
  return (c.r<<4) | (c.g<<2) | (c.b);
}

//////////////////
// global funcs //
//////////////////

/*
color_t make_color_set( color_t c1, color_t c2 ) {
  if (c2.a)     return make_color(c2.r, c2.g, c2.b);
  return          make_color_trans();
}
*/

color_t make_color_add( color_t c1, color_t c2 ) {
  return make_color(
    high(get_color_r(c1) + get_color_r(c2),3),
    high(get_color_g(c1) + get_color_g(c2),3),
    high(get_color_b(c1) + get_color_b(c2),3)
  );
}

color_t make_color_sub( color_t c1, color_t c2 ) {
  return make_color(
    low(get_color_r(c1) - get_color_r(c2),0),
    low(get_color_g(c1) - get_color_g(c2),0),
    low(get_color_b(c1) - get_color_b(c2),0)
  );
}

color_t make_color_high( color_t c1, color_t c2 ) {
  return make_color(
    high(get_color_r(c1),get_color_r(c2)),
    high(get_color_g(c1),get_color_g(c2)),
    high(get_color_b(c1),get_color_b(c2))
  );
}

color_t make_color_low( color_t c1, color_t c2 ) {
  return make_color(
    low(get_color_r(c1),get_color_r(c2)),
    low(get_color_g(c1),get_color_g(c2)),
    low(get_color_b(c1),get_color_b(c2))
  );
}

color_t make_color_avg( color_t c1, color_t c2 ) {
  return make_color(
    evenize(get_color_r(c1)+get_color_r(c2))/2,
    evenize(get_color_g(c1)+get_color_g(c2))/2,
    evenize(get_color_b(c1)+get_color_b(c2))/2
  );
}

void color_to_string( color_t c, string_t * s ) {
  int u = 0;
  u = string_copy_number_at(s,u,c.r);
  u = string_copy_number_at(s,u,c.g);
  u = string_copy_number_at(s,u,c.b);
  u = string_copy_number_at(s,u,c.a);
}