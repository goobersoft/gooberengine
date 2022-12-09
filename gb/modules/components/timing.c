
#define timing_desired_fps()   60
#define timing_max_cpu_time()  16667

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

  int           clock_hours;
  int           clock_minutes;
  int           clock_seconds;
  int           clock_nano;

  int           cpu_usage;

} timing_t;

#define timing_timeval(self)   (&self->timeval)
#define timing_curr(self)      (self->time_curr)
#define timing_old(self)       (self->time_old)
#define timing_diff(self)      (self->time_diff)
#define timing_fps(self)       (self->fps)
#define timing_fps_time(self)  (self->fps_time)
#define timing_fps_ticks(self) (self->fps_ticks)
#define timing_cpu_usage(self) (self->cpu_usage)

#define timing_clock_hours(self)   (self->clock_hours)
#define timing_clock_minutes(self) (self->clock_minutes)
#define timing_clock_seconds(self) (self->clock_seconds)
#define timing_clock_nano(self)    (self->clock_nano)

/////////
// new //
/////////

void timing_init( timing_t * self ) {
  timing_curr(self)      = 0;
  timing_old(self)       = 0;
  timing_diff(self)      = 0;
  timing_fps(self)       = 0;
  timing_fps_time(self)  = 0;
  timing_fps_ticks(self) = 0;
  timing_cpu_usage(self) = 0;

  timing_clock_hours(self)    = 0;
  timing_clock_minutes(self)  = 0;
  timing_clock_seconds(self)  = 0;
  timing_clock_nano(self)     = 0;
}

timing_t * timing() {
  timing_t * r = alloc(timing_t);
  timing_init(r);
  return r;
}

///////////
// funcs //
///////////

int timing_get_fps( timing_t * self ) {
  return timing_fps(self);
}

void timing_record( timing_t * self ) {
  gettimeofday(timing_timeval(self),NULL);
  timing_old(self)  = timing_curr(self);
  timing_curr(self) = (timing_timeval(self)->tv_sec * 1000000) + (timing_timeval(self)->tv_usec);
  if (timing_old(self) == 0) timing_old(self) = timing_curr(self);
  timing_diff(self) = timing_curr(self) - timing_old(self);

  // fps
  timing_fps_time(self) += timing_diff(self);
}

void timing_update_clock( timing_t * self ) {
  timing_clock_nano(self) += timing_diff(self);
  if (timing_clock_nano(self) >= 1000000) {
    timing_clock_nano(self) -= 1000000;
    timing_clock_seconds(self) += 1;
  }
  if (timing_clock_seconds(self) >= 60) {
    timing_clock_seconds(self) -= 60;
    timing_clock_minutes(self) += 1;
  }
  if (timing_clock_minutes(self) >= 60) {
    timing_clock_minutes(self) -= 60;
    timing_clock_hours(self) += 1;
  }
}

////////////
// events //
////////////

void timing_update_pre( timing_t * self ) {
  timing_record(self);
  timing_update_clock(self);
}

void timing_update_post(timing_t * self) {
  timing_record(self);
  timing_update_clock(self);

  // fps
  timing_fps_ticks(self) += 1;
  if (timing_fps_time(self) > microseconds(1)) {
    timing_fps_time(self) -= microseconds(1);
    timing_fps(self) = timing_fps_ticks(self);
    timing_fps_ticks(self) = 0;
  }

  timing_cpu_usage(self) = frac(100,timing_diff(self),timing_max_cpu_time());
}



