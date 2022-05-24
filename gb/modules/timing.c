
////////////
// struct //
////////////

typedef struct timeval timeval_t;

typedef struct {

  timeval_t     timeval;
  ulong_t       time_old;
  ulong_t       time_curr;
  ulong_t       time_diff;
  int           fps;
  ulong_t       fps_time;
  int           fps_ticks;

} timing_t;

#define timing_timeval(self)   (&self->timeval)
#define timing_curr(self)      (self->time_curr)
#define timing_old(self)       (self->time_old)
#define timing_diff(self)      (self->time_diff)
#define timing_fps(self)       (self->fps)
#define timing_fps_time(self)  (self->fps_time)
#define timing_fps_ticks(self) (self->fps_ticks)

/////////
// new //
/////////

timing_t * timing() {
  timing_t * r = alloc(timing_t);
  timing_curr(r) = 0;
  timing_old(r)  = 0;
  timing_diff(r) = 0;
  return r;
}

/////////
// del //
/////////

// no pointers inside of timing
#define free_timing(self) free(self)

///////////
// funcs //
///////////

int timing_get_fps( timing_t * self ) {
  return timing_fps(self);
}

////////////
// events //
////////////

void timing_update(timing_t * self) {
  gettimeofday(timing_timeval(self),NULL);

  timing_old(self)  = timing_curr(self);
  timing_curr(self) = (timing_timeval(self)->tv_sec * 1000000) + (timing_timeval(self)->tv_usec);
  if (timing_old(self) == 0) timing_old(self) = timing_curr(self);
  timing_diff(self) = timing_curr(self) - timing_old(self);
  
  // fps
  timing_fps_ticks(self) += 1;
  timing_fps_time(self) += timing_diff(self);
  if (timing_fps_time(self) > microsecond(1)) {
    timing_fps_time(self) -= microsecond(1);
    timing_fps(self) = timing_fps_ticks(self);
    timing_fps_ticks(self) = 0;
  }
}

