
type() {

  int hours;
  int minutes;
  int seconds;
  int nanos;

} wallclock_t;

#define wallclock_hours(self)   (self->hours)
#define wallclock_minutes(self) (self->minutes)
#define wallclock_seconds(self) (self->seconds)
#define wallclock_nanos(self)   (self->nanos)

/////////
// new //
/////////

void init_wallclock( wallclock_t * self ) {
  wallclock_hours(self)   = 0;
  wallclock_minutes(self) = 0;
  wallclock_seconds(self) = 0;
  wallclock_nanos(self)   = 0;
}

wallclock_t * wallclock() {
  wallclock_t * self = alloc(wallclock_t);
  init_wallclock(self);
  return self;
}

void free_wallclock( wallclock_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

// add nanoseconds.
void wallclock_add_ns( wallclock_t * self, ulong_t ns ) {
  wallclock_nanos(self) += ns;
  if (wallclock_nanos(self) >= 1000000) {
    wallclock_nanos(self)   -= 1000000;
    wallclock_seconds(self) += 1;
  }
  if (wallclock_seconds(self) >= 60) {
    wallclock_seconds(self) -= 60;
    wallclock_minutes(self) += 1;
  }
  if (wallclock_minutes(self) >= 60) {
    wallclock_minutes(self) -= 60;
    wallclock_hours(self)   += 1;
  }
}

void wallclock_add_ms( wallclock_t * self, ulong_t ms ) {
  wallclock_add_ns( self, ms*1000 );
}

void wallclock_add_tick( wallclock_t * self, int u ) {
  wallclock_add_ns( self, u*16667 );
}