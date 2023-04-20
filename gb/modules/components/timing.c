
#define timing_desired_fps()   60
#define timing_max_cpu_time()  16667

////////////
// struct //
////////////



typedef struct {

  // uses a stopwatch internally.
  stopwatch_t * stopwatch;
  wallclock_t * wallclock;

  ulong_t       time_old;
  ulong_t       time_curr;
  ulong_t       time_diff;

  int           fps;
  ulong_t       fps_time;
  int           fps_ticks;

  ulong_t       sleep_nanos;

} timing_t;

#define timing_stopwatch(self)     (self->stopwatch)
#define timing_wallclock(self)     (self->wallclock)
#define timing_start(self)         stopwatch_time_start(timing_stopwatch(self))
#define timing_stop(self)          stopwatch_time_stop(timing_stopwatch(self))
#define timing_diff(self)          stopwatch_time_diff(timing_stopwatch(self))
#define timing_fps(self)           (self->fps)
#define timing_fps_time(self)      (self->fps_time)
#define timing_fps_ticks(self)     (self->fps_ticks)
#define timing_sleep_nanos(self)   (self->sleep_nanos)

/////////
// new //
/////////

void init_timing( timing_t * self ) {
  // make the stopwatch
  timing_stopwatch(self) = stopwatch();
  timing_wallclock(self) = wallclock();

  timing_fps(self)       = 0;
  timing_fps_time(self)  = 0;
  timing_fps_ticks(self) = 0;

}

timing_t * timing() {
  timing_t * r = alloc(timing_t);
  init_timing(r);
  return r;
}

///////////
// funcs //
///////////

int timing_get_fps( timing_t * self ) {
  return timing_fps(self);
}



////////////
// events //
////////////

void timing_update_pre( timing_t * self ) {
  stopwatch_start( timing_stopwatch(self) );
}

void timing_update_post(timing_t * self) {
  stopwatch_stop( timing_stopwatch(self) );
  
  
  
  // we want to sleep between 16-17ms ideally.
  // we can afford to sleep if we have less than 16667 nanoseconds in our
  // sleep nanos.
  int u = low(16667-stopwatch_time_diff(timing_stopwatch(self)),0);
  SDL_Delay(u/1000);

  timing_fps_ticks(self) += 1;
  if (wallclock_seconds(timing_wallclock(self))!=timing_fps_time(self)) {
    timing_fps_time(self)  = wallclock_seconds(timing_wallclock(self));
    timing_fps(self)       = timing_fps_ticks(self);
    timing_fps_ticks(self) = 0;
  }

  wallclock_add_tick( timing_wallclock(self), 1 );
}



