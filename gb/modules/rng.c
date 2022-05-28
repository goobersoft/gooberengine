
type() {

  int *  mt;
  int    index;
  int    lower_mask;
  int    upper_mask;


} rng_t;

#define rng_mt(self)    (self->mt)    // word size (in number of bits)
#define rng_index(self) (self->index) // degree of recurrence

#define rng_bitwidth()  32                  // w
#define rng_degree()    624                 // n
#define rng_mword()     397                 // m, 1 <= m < n
#define rng_rsep()      31                  // r, 0 <= r <= w-1
#define rng_lowermask() bits(_0111_1111_1111_1111_1111_1111_1111_1111)
#define rng_uppermask() bits(_1000_0000_0000_0000_0000_0000_0000_0000)
#define rng_coeff()     0x9908b0df          // a, coefficients
#define rng_b()         0x71d67fffeda60000  // TGFSR(R) tempering bitmask
#define rng_c()         0xfff7eee000000000  // TGFSR(R) tempering bitmask
#define rng_s()         17                  // TGFSR(R) tempering bit shift 
#define rng_t()         37                  // TGFSR(R) tempering bit shift
#define rng_u()         11                  // additional Mersenne Twister tempering bit shift/mask
#define rng_d()         0xffffffff          // additional Mersenne Twister tempering bit shift/mask
#define rng_l()         18                  // additional Mersenne Twister tempering bit shift/mask
#define rng_f()         1812433253          // used for generation

 10000000 00000000 00000000 00000000

/////////
// new //
/////////

rng_t * rng() {
  rng_t * r = alloc(rng_t);

  rng_mt(r)     = allocv(int,rng_degree());
  rng_index(r)  = rng_degree()+1;
  
}

///////////////
// functions //
///////////////

void rng_seed( rng_t * self, int s ) {
  rng_index(self) = rng_degree();
  rng_mt(self)[0] = s;
  loop(i,1,rng_degree()) {
    rng_mt(self)[i] = (rng_f() * (rng_mt(self)[i-1] ^ (rng_mt(self)[i-1] >> (rng_bitwidth()-2)) + i);
  }
}

int rng_twist( rng_t * self ) {
  int x;
  int xa;
  loop(i,0,rng_degree()) {
    x  = (rng_mt(self)[i] & rng_uppermask()) + (rng_mt(self)[(i+1)%rng_degree()] & rng_lowermask());
    xa = x >> 1;
    if ((x&1)==1) {
      xa = xa ^ rng_coeff();
    }
  }
}

int rng_get( rng_t * self ) {
  if (rng_index(self) >= rng_degree()) {
    if (rng_index(self) > rng_degree()) {
      return 0;
    }
    rng_twist( rng_t * self );
  }

  int y = rng_mt(self)[rng_index(self)];
  y = y ^ ((y >> rng_u()) & rng_d());
  y = y ^ ((y << rng_u()) & rng_d());
  y = y ^ ((y << rng_t()) & rng_c());
  y = y ^ (y >> rng_l());

  rng_index(self) += 1;
  return y;
}

/*
  source:
  https://en.wikipedia.org/wiki/Mersenne_Twister#Pseudocode

 // Create a length n array to store the state of the generator
 int[0..n-1] MT
 int index := n+1
 const int lower_mask = (1 << r) - 1 // That is, the binary number of r 1's
 const int upper_mask = lowest w bits of (not lower_mask)
 
 // Initialize the generator from a seed
 function seed_mt(int seed) {
     index := n
     MT[0] := seed
     for i from 1 to (n - 1) { // loop over each element
         MT[i] := lowest w bits of (f * (MT[i-1] xor (MT[i-1] >> (w-2))) + i)
     }
 }
 
 // Extract a tempered value based on MT[index]
 // calling twist() every n numbers
 function extract_number() {
     if index >= n {
         if index > n {
           error "Generator was never seeded"
           // Alternatively, seed with constant value; 5489 is used in reference C code[53]
         }
         twist()
     }
 
     int y := MT[index]
     y := y xor ((y >> u) and d)
     y := y xor ((y << s) and b)
     y := y xor ((y << t) and c)
     y := y xor (y >> l)
 
     index := index + 1
     return lowest w bits of (y)
 }
 
 // Generate the next n values from the series x_i 
 function twist() {
     for i from 0 to (n-1) {
         int x := (MT[i] and upper_mask)
                   + (MT[(i+1) mod n] and lower_mask)
         int xA := x >> 1
         if (x mod 2) != 0 { // lowest bit of x is 1
             xA := xA xor a
         }
         MT[i] := MT[(i + m) mod n] xor xA
     }
     index := 0
 }
*/