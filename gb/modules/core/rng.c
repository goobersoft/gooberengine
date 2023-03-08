/*

ChatGPT:
This implementation uses the original Mersenne Twister algorithm developed 
by Matsumoto and Nishimura. The init_genrand function initializes the generator 
with a seed value, and genrand_int32 generates a 32-bit pseudorandom integer.

Note that this implementation assumes a 32-bit integer size. If your system 
uses a different integer size, you may need to adjust the constants and shift 
operations used in the algorithm.



#define _rng_mt_len 624
#define _rng_mt_ia  397

unsigned long _rng_mt[_rng_mt_len];
int _rng_mti = _rng_mt_len + 1;

void init_genrand(unsigned long s)
{
    _rng_mt[0] = s & 0xffffffffUL;
    for (_rng_mti = 1; _rng_mti < _rng_mt_len; _rng_mti++) {
        _rng_mt[_rng_mti] = (1812433253UL * (_rng_mt[_rng_mti - 1] ^ (_rng_mt[_rng_mti - 1] >> 30)) + _rng_mti);
        _rng_mt[_rng_mti] &= 0xffffffffUL;
    }
}

unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2] = {0x0UL, 0x9908b0dfUL};
    if (_rng_mti >= _rng_mt_len) {
        int kk;
        if (_rng_mti == _rng_mt_len + 1) {
            init_genrand(5489UL);
        }
        for (kk = 0; kk < _rng_mt_len - _rng_mt_ia; kk++) {
            y = (_rng_mt[kk] & 0x80000000UL) | (_rng_mt[kk + 1] & 0x7fffffffUL);
            _rng_mt[kk] = _rng_mt[kk + _rng_mt_ia] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < _rng_mt_len - 1; kk++) {
            y = (_rng_mt[kk] & 0x80000000UL) | (_rng_mt[kk + 1] & 0x7fffffffUL);
            _rng_mt[kk] = _rng_mt[kk + (_rng_mt_ia - _rng_mt_len)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (_rng_mt[_rng_mt_len - 1] & 0x80000000UL) | (_rng_mt[0] & 0x7fffffffUL);
        _rng_mt[_rng_mt_len - 1] = _rng_mt[_rng_mt_ia - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        _rng_mti = 0;
    }
    y = _rng_mt[_rng_mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}
*/


// ---------------------------------------------------------------------------

#define rng_mt_len() 624
#define rng_mt_ia()  397
#define rng_mag01()  {0x0UL, 0x9908b0dfUL}

type() {

  ulong_t seed;
  ulong_t mt[rng_mt_len()];
  int    mti;


} rng_t;

#define rng_seed(self) (self->seed)
#define rng_mt(self)   (self->mt)
#define rng_mti(self)  (self->mti)

void init_rng_genrand( rng_t * self, uint_t s ) {
  rng_seed(self) = s;
  rng_mt(self)[0] = rng_seed(self) & 0xffffffffUL;
  for (rng_mti(self) = 1; rng_mti(self) < rng_mt_len(); rng_mti(self)++) {
    rng_mt(self)[rng_mti(self)] = (1812433253UL * (rng_mt(self)[rng_mti(self) - 1] 
      ^ (rng_mt(self)[rng_mti(self) - 1] >> 30)) + rng_mti(self));
    rng_mt(self)[rng_mti(self)] &= 0xffffffffUL;
  }
}

void init_rng( rng_t * self ) {
  rng_mti(self)  = rng_mt_len() + 1;
  init_rng_genrand(self,rng_seed(self));
}

rng_t * rng() {
  rng_t * self = alloc(rng_t);
  init_rng(self);
  return self;
}

ulong_t rng_get( rng_t * self ) {
  ulong_t y;
  static ulong_t mag01[2] = {0x0UL, 0x9908b0dfUL};
  if (rng_mti(self) >= rng_mt_len()) {
    int kk;
    if (rng_mti(self) == rng_mt_len() + 1) {
      init_rng_genrand(self,5489UL);
    }
    for (kk = 0; kk < rng_mt_len() - rng_mt_ia(); kk++) {
        y = (rng_mt(self)[kk] & 0x80000000UL) | (rng_mt(self)[kk + 1] & 0x7fffffffUL);
        rng_mt(self)[kk] = rng_mt(self)[kk + rng_mt_ia()] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (; kk < rng_mt_len() - 1; kk++) {
        y = (rng_mt(self)[kk] & 0x80000000UL) | (rng_mt(self)[kk + 1] & 0x7fffffffUL);
        rng_mt(self)[kk] = rng_mt(self)[kk + (rng_mt_ia() - rng_mt_len())] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (rng_mt(self)[rng_mt_len() - 1] & 0x80000000UL) | (rng_mt(self)[0] & 0x7fffffffUL);
    rng_mt(self)[rng_mt_len() - 1] = rng_mt(self)[rng_mt_ia() - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
    rng_mti(self) = 0;
  }
  y = rng_mt(self)[rng_mti(self)++];
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);
  return y;
}