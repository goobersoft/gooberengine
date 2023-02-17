
/////////////////////
// bitfield object //
/////////////////////

type() {

  int       size;
  byte_t  * data;

} bitfield_t;

#define bitfield_size(self)     (self->size)
#define bitfield_bytes(self)    ((self->size)/8)
#define bitfield_data(self)     (self->data)
#define bitfield_byte(self,n)   (self->data[n])

/////////
// new //
/////////

void bitfield_init( bitfield_t * self, int l ) {
  bitfield_size(self) = l;
  bitfield_data(self) = allocv(byte_t,1+((l-1)/8));
}

bitfield_t * bitfield( int l ) {
  l = low(l,1);
  bitfield_t * self = alloc(bitfield_t);
  bitfield_init(self,l);
  return self;
}

void free_bitfield( bitfield_t * self ) {
  free( bitfield_data(self) );
  free( self );
}

///////////////
// functions //
///////////////

int bitfield_get( bitfield_t * self, int b ) {
  if inrange(b,0,bitfield_size(self)) {
    return bget(bitfield_data(self)[b/8],b&7);
  }
  return -1;
}

void bitfield_on( bitfield_t * self, int b ) {
  if inrange(b,0,bitfield_size(self)) {
    bitfield_data(self)[b/8] = bon(bitfield_data(self)[b/8],b&7);
  }
}

void bitfield_off( bitfield_t * self, int b ) {
  if inrange(b,0,bitfield_size(self)) {
    bitfield_data(self)[b/8] = boff(bitfield_data(self)[b/8],b&7);
  }
}

void bitfield_toggle( bitfield_t * self, int b ) {
  if inrange(b,0,bitfield_size(self)) {
    bitfield_data(self)[b/8] = btog(bitfield_data(self)[b/8],b&7);
  }
}

void bitfield_print_bits( bitfield_t * self ) {
  loop(i,bitfield_bytes(self)) {
    log("%s",bits8(bitfield_byte(self,i)));
  }
}