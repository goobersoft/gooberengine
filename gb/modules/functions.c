
/////////////
// generic //
/////////////

// empty function
void emp() {
}

char _bits[36];
char * bits(uint_t n) {
  loop(i,0,32) {
    _bits[31-i] = (n&1) ? '1' : '0';
    n = n>>1;
  }
}





void swap(int * a, int * b) {
  a[0] ^= b[0];
  b[0] ^= a[0];
  a[0] ^= b[0];
}


// aka max()
int low(int a, int b) {
  return (a < b) ? b : a;
}

// aka min()
int high(int a, int b) {
  return (a > b) ? b : a;
}

// clamps int n between l and h inclusively.
int clamp(int n, int l, int h) {
  return high(low(n,l),h);
}

int wrap(int n, int l, int h) {
  if (l>h)           swap(ref(l),ref(h));
  if (l==h)          return l;
  while (n<l)    n += (h-l);
  while (n>=h)   n -= (h-l);
  return n;
}

// resizes an int array, coping all values.
// if new array is shorter, data will be lost.
int * resize(int * a, int os, int ns) {
  int * r = allocv(int,ns);
  for (int i = 0; i < high(os,ns); i++) {
    r[i] = a[i];
  }
  // free original array
  free(a);
  return r;
}

// fraction a / b
// p is the precision (1000 is the norm)
// the equation is a * p / b
// uses a long for temporary storage.
int frac(int a, int b, int p) {
  long r = (a * b);
  r = r / p;
  return (int)r;
}


int power( int a, int b ) {
  b = low(b,0);
  if (b == 0) return 1;
  b -= 1;
  int r = a;
  while(b>0) {
    r *= a;
    b -= 1;
  }
  return r;
}

int digits( int n ) {
  n     = abs(n);
  if (n<10) return 1;
  int u = 1;
  while ( n >= power(10,u) ) {
    u += 1;
  }
  return u;
}


/*
char _str[20];
char * str( int n ) {
  int c = n;
  int q = (n<0);

}
*/

// a: array to search
// l: length of array
// c: starting position
// v: value to check for
int find( int * a, int l, int c, int v ) {
  for (int i = 0; i < l; i++) {
    if (a[c] == v) {
      return c;
    }
    c = (c + 1) % l;
  }
  return undefined();
}


Uint32 abgr( byte_t a, byte_t b, byte_t g, byte_t r ) {
  return (((Uint32)a)<<24) | (((Uint32)b)<<16) | (((Uint32)g)<<8) | ((Uint32)r);
}


char * sdl_pixelformat_str( uint_t f ) {
  switch(f) {
    case SDL_PIXELFORMAT_UNKNOWN:     return "SDL_PIXELFORMAT_UNKNOWN";
    case SDL_PIXELFORMAT_INDEX1LSB:   return "SDL_PIXELFORMAT_INDEX1LSB";
    case SDL_PIXELFORMAT_INDEX1MSB:   return "SDL_PIXELFORMAT_INDEX1MSB";
    case SDL_PIXELFORMAT_INDEX4LSB:   return "SDL_PIXELFORMAT_INDEX4LSB";
    case SDL_PIXELFORMAT_INDEX4MSB:   return "SDL_PIXELFORMAT_INDEX4MSB";
    case SDL_PIXELFORMAT_INDEX8:      return "SDL_PIXELFORMAT_INDEX8";
    case SDL_PIXELFORMAT_RGB332:      return "SDL_PIXELFORMAT_RGB332";
    case SDL_PIXELFORMAT_RGB444:      return "SDL_PIXELFORMAT_RGB444";
    case SDL_PIXELFORMAT_RGB555:      return "SDL_PIXELFORMAT_RGB555";
    case SDL_PIXELFORMAT_BGR555:      return "SDL_PIXELFORMAT_BGR555";
    case SDL_PIXELFORMAT_ARGB4444:    return "SDL_PIXELFORMAT_ARGB4444";
    case SDL_PIXELFORMAT_RGBA4444:    return "SDL_PIXELFORMAT_RGBA4444";
    case SDL_PIXELFORMAT_ABGR4444:    return "SDL_PIXELFORMAT_ABGR4444";
    case SDL_PIXELFORMAT_BGRA4444:    return "SDL_PIXELFORMAT_BGRA4444";
    case SDL_PIXELFORMAT_ARGB1555:    return "SDL_PIXELFORMAT_ARGB1555";
    case SDL_PIXELFORMAT_RGBA5551:    return "SDL_PIXELFORMAT_RGBA5551";
    case SDL_PIXELFORMAT_ABGR1555:    return "SDL_PIXELFORMAT_ABGR1555";
    case SDL_PIXELFORMAT_BGRA5551:    return "SDL_PIXELFORMAT_BGRA5551";
    case SDL_PIXELFORMAT_RGB565:      return "SDL_PIXELFORMAT_RGB565";
    case SDL_PIXELFORMAT_BGR565:      return "SDL_PIXELFORMAT_BGR565";
    case SDL_PIXELFORMAT_RGB24:       return "SDL_PIXELFORMAT_RGB24";
    case SDL_PIXELFORMAT_BGR24:       return "SDL_PIXELFORMAT_BGR24";
    case SDL_PIXELFORMAT_RGB888:      return "SDL_PIXELFORMAT_RGB888";
    case SDL_PIXELFORMAT_RGBX8888:    return "SDL_PIXELFORMAT_RGBX8888";
    case SDL_PIXELFORMAT_BGR888:      return "SDL_PIXELFORMAT_BGR888";
    case SDL_PIXELFORMAT_BGRX8888:    return "SDL_PIXELFORMAT_BGRX8888";
    case SDL_PIXELFORMAT_ARGB8888:    return "SDL_PIXELFORMAT_ARGB8888";
    case SDL_PIXELFORMAT_RGBA8888:    return "SDL_PIXELFORMAT_RGBA8888";
    case SDL_PIXELFORMAT_ABGR8888:    return "SDL_PIXELFORMAT_ABGR8888";
    case SDL_PIXELFORMAT_BGRA8888:    return "SDL_PIXELFORMAT_BGRA8888";
    case SDL_PIXELFORMAT_ARGB2101010: return "SDL_PIXELFORMAT_ARGB2101010";
    //case SDL_PIXELFORMAT_RGBA32:      return "SDL_PIXELFORMAT_RGBA32";
    //case SDL_PIXELFORMAT_ARGB32:      return "SDL_PIXELFORMAT_ARGB32";
    //case SDL_PIXELFORMAT_BGRA32:      return "SDL_PIXELFORMAT_BRGA32";
    //case SDL_PIXELFORMAT_ABGR32:      return "SDL_PIXELFORMAT_ABGR32";
    case SDL_PIXELFORMAT_YV12:        return "SDL_PIXELFORMAT_YV12";
    case SDL_PIXELFORMAT_IYUV:        return "SDL_PIXELFORMAT_IYUV";
    case SDL_PIXELFORMAT_YUY2:        return "SDL_PIXELFORMAT_YUY2";
    case SDL_PIXELFORMAT_UYVY:        return "SDL_PIXELFORMAT_UYVY";
    case SDL_PIXELFORMAT_YVYU:        return "SDL_PIXELFORMAT_YVYU";
    case SDL_PIXELFORMAT_NV12:        return "SDL_PIXELFORMAT_NV12";
    case SDL_PIXELFORMAT_NV21:        return "SDL_PIXELFORMAT_NV21";
    default:                          return "UNDEFINED";
  }
}

/////////
// rng //
/////////

#define rnd(a,b)     (a+(rand()%(b-a)))
#define chance(x,y)  (rnd(0,y)<x)
#define prob(x)      chance(x,1000)
#define sqr(a)       ((a)*(a))
#define bool(x)      (x&1)

#define dist(x1,y1,x2,y2) \
  sqroot(sqr(x2-x1)+sqr(y2-y1))


int ceiled( int a, int d ) {
  int u = abs(a)%d;
  if (u>0) {
    if (a < 0) {
      a -= d-u;
    }
    else {
      a += d-u;
    }
  }
  return a;
}

int floored( int a, int d ) {
  int u = abs(a)%d;
  if (u>0) {
    if (a < 0) {
      a += u;
    }
    else {
      a -= u;
    }
  }
  return a;
}

int rounded( int a, int d ) {
  if ((abs(a)%d) >= d/2) {
    return ceiled(a,d);
  }
  else {
    return floored(a,d);
  }
}

int evenize( int a ) {
  return (a&1)?a+1:a;
}



////////////////
// collisions //
////////////////

#define inrange(n,l,h) \
  ((n>=l)&&(n<h)?true():false())

#define inrect(x1,y1,x2,y2,w2,h2) \
  (inrange(x1,x2,x2+w2)&&inrange(y1,y2,y2+h2))

#define incube(x1,y1,z1,x2,y2,z2,w2,h2,d2) \
  (inrange(x1,x2,x2+w2)&&inrange(y1,y2,y2+h2)&&inrange(z1,z2,z2+d2))

#define rectinrect(x1,y1,w1,h1,x2,y2,w2,h2) \
  (inrect(x1,y1,x2,y2,w2,h2)&&inrect(x1+w1-1,y1,x2,y2,w2,h2)&& \
   inrect(x1,y1+h1-1,x2,y2,w2,h2)&&inrect(x1+w1-1,y1+h1-1,x2,y2,w2,h2))

#define rectinrect2(x1,y1,w1,h1,x2,y2,w2,h2) \
  (rectinrect(x1,y1,w1,h1,x2,y2,w2,h2)&&rectinrect(x2,y2,w2,h2,x1,y1,w1,h1))

////////////
// arrays //
////////////

///////////////////
// locker object //
///////////////////
// this is supposed to manage array systems with locking array indices

typedef struct {

  // the counter for the array to start looking from
  int    counter;
  // the size of the array it is managing
  int    length;
  // an array of active flags for locking resources
  int  * active;

} locker_t;

#define locker_counter(self) (self->counter)
#define locker_length(self)  (self->length)
#define locker_active(self)  (self->active)


locker_t * locker( int l ) {
  locker_t * r = alloc(locker_t);
  locker_counter(r) = 0;
  locker_length(r)  = l;
  locker_active(r)  = allocv(bool_t,l);
  return r;
}