


/////////////
// generic //
/////////////

// empty function
void emp() {
}

int sign( int n ) {
  // if n is 0
  if (n == 0) return 0;
  // if n > 0
  if (n > 0) return 1;
  // if n < 0
  return -1;
}

int digits( int n ) {
  if (n == 0) return 1;
  int d = 0;
  // while n is not 0
  while (n != 0) {
    // number of digits inc 1
    d += 1;
    // divide n by 10
    n /= 10;
  }
  // return number of digits
  return d;
}


char _str[20];
char * str( int n ) {
  // init (runs only once)
  static bool_t init = 0;
  if (init == 0) {
    loop(i,20) {
      _str[i] = '\0';
    }
    init = 1;
  } 
  // get the number of digits
  int d = digits(n);
  // determine if the number is negative
  int s = sign(n);
  // if the number is negative, the first
  // cell in the string array is a hyphen
  if (s == -1) {
    // add 1 to d to accomodate for hyphen
    d += 1;
    // set index 0 to '-'
    _str[0] = '-';
  }
  // get absolute number of n
  // otherwise modulo won't work right
  n = abs(n);
  // begin looping from [0 to d-1]

  _str[d] = '\0';

  do {
    d-=1;
    _str[d] = '0' + (n % 10);
    n /= 10;
  } while(n>0);
  
  return _str;
}


char _right[100];
// s: input string
// l: length needed
// p: padding char
char * right( char * s, int l, char p ) {
  // create temporary char pointer to _right[] array
  char * r = _right;
  // set the default padding char to ' ' if it is not provided.
  if (p == 0) p = ' ';
  // get the current length of s
  int ll = strlen(s);
  // get the amount of padding we need
  // number of pad chars = (desired length - length of string)
  int q = l - ll;
  // is q 0?
  if (q == 0) {
    // copy the string
    loop(i,l) {
      r[i] = s[i];
    }
    // terminate string and return
    r[l] = '\0';
    return r;
  }
  // is q positive?
  else if (q >= 0) {
    // pad the left side with p
    loop(i,q) {
      r[i] = p;
    }
    // now copy the string
    loop(i,ll) {
      r[q+i] = s[i];
    }
    // terminate string and return
    r[l] = '\0';
    return r;
  }
  // otherwise is q negative?
  else if (q < 0) {
    q=abs(q);
    // copy only a portion of the string
    loop(i,l) {
      r[i] = s[q+i];
    }
    // terminate string and return
    r[l] = '\0';
    return r;
  }
}

char * rstr( int n, int l ) {
  return right(str(n),l,'0');
}

/*
int  _tokenize_len;
char _tokenize_str[1024][64];
char ** tokenize ( char * s, char u ) {
  // set current tokenized list to 0
  _tokenize_len = 0;
  // current character
  char c;
  // current word
  char w[64];
  // current word index
  int wi = 0;
  // length of input str
  int l = strlen(s);

  loop(i,l) {
    
    if (s[i] == u) {
      if (wi > 0) {
        copy( _tokenize_str[_tokenize_len], w, 0, wi );
        wi = 0;
        _tokenize_len += 1;
      }
    }
    else {
      w[wi]  = u;
      wi    += 1;
    }

  }

  return _tokenize_str;

}
*/

char _bits[36];
char * bits(uint_t n) {
  loop(i,32) {
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

#define max(a,b) low(a,b)
#define min(a,b) high(a,b)

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

#define rnd(a,b)     (((b-a)==0)?(a):(a+(rand()%(b-a))))
#define chance(x,y)  (rnd(0,y)<x)
#define prob(x)      ((x==1000)?(1):(chance(x,1000)))
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
  (inrect(x1,y1,x2,y2,w2,h2)||inrect(x1+w1-1,y1,x2,y2,w2,h2)|| \
   inrect(x1,y1+h1-1,x2,y2,w2,h2)||inrect(x1+w1-1,y1+h1-1,x2,y2,w2,h2))

#define rectinrect2(x1,y1,w1,h1,x2,y2,w2,h2) \
  (rectinrect(x1,y1,w1,h1,x2,y2,w2,h2)||rectinrect(x2,y2,w2,h2,x1,y1,w1,h1))

