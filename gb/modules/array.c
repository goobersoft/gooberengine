
#define array_cellsize_byte()   1
#define array_cellsize_short()  2
#define array_cellsize_int()    4
#define array_cellsize_long()   8

// arrays are as generic as they can get.
// instead of holding which datatype is being used,
// it simply understands how many bytes a single cell is.
// you can then typecast to whichever kind of data type
// permits for that cell size.
type() {

  int      cellsize;
  int      length;
  int      size;
  void **  data;

} array_t;

#define array_cellsize(self)  (self->cellsize)
#define array_length(self)    (self->length)
#define array_size(self)      (self->size)
#define array_data(self)      (self->data)

/////////
// new //
/////////

void array_init( array_t * self, int l, int cs ) {
  array_cellsize(self)  = cs;
  array_length(self)    = l;
  int u = l * cs;
  array_size(self)      = u;
  array_data(self)      = malloc( u );
  
  byte_t * du = cast(array_data(self),byte_t*);
  loop(i,0,u) {
    
  }
}

array_t * array( int l, int cs ) {
  array_t * r = alloc(array_t);
  array_init(r,l,cs);
  return r;
}