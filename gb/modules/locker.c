
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

#define locker_counter(self) (self.counter)
#define locker_length(self)  (self.length)
#define locker_active(self)  (self.active)

void locker_init( locker_t * self, int l ) {
  locker_counter(self)   = 0;
  locker_length(self)    = l;
  locker_active(self)    = allocv(bool_t,l);
}

locker_t * locker( int l ) {
  locker_t r = alloc(locker_t);
  locker_init(r,l);
  return r;
}