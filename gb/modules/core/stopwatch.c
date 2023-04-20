

type() {

  timeval_t timeval;
  uint_t    time_start;
  uint_t    time_end;
  uint_t    time_diff;

} stopwatch_t;

#define stopwatch_timeval(self)    (self->timeval)
#define stopwatch_time_start(self) (self->time_start)
#define stopwatch_time_end(self)   (self->time_end)
#define stopwatch_time_diff(self)  (self->time_diff)

void init_stopwatch( stopwatch_t * self ) {
  stopwatch_time_start(self)  = 0;
  stopwatch_time_end(self)    = 0;
  stopwatch_time_diff(self)   = 0;
} 

stopwatch_t * stopwatch() {
  stopwatch_t * self = alloc(stopwatch_t);
  init_stopwatch(self);
  return self;
}

void free_stopwatch( stopwatch_t * self ) {
  free(self);
}



///////////
// funcs //
///////////

void stopwatch_start( stopwatch_t * self ) {
  gettimeofday( ref(stopwatch_timeval(self)), null() );
  stopwatch_time_start(self) = (stopwatch_timeval(self).tv_sec * 1000000)
    + stopwatch_timeval(self).tv_usec;
  stopwatch_time_end(self)  = stopwatch_time_start(self);
}

void stopwatch_stop( stopwatch_t * self ) {
  gettimeofday( ref(stopwatch_timeval(self)), null() );
  stopwatch_time_end(self) = (stopwatch_timeval(self).tv_sec * 1000000)
    + stopwatch_timeval(self).tv_usec;
  // use max(0,x) to make sure that if the timeofday wraps back because of
  // midnight, we could say the diff is 0.
  stopwatch_time_diff(self) = low(0,stopwatch_time_end(self)-stopwatch_time_start(self));
}

