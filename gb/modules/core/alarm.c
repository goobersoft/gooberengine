
type() {


  ulong_t   time_start;
  ulong_t   time_end;

  // length of time in millisecs
  ulong_t   length;

  // this is true if the alarm is currently passed its
  // time period
  bool_t    ringing;

} alarm_t;

#define alarm_time_start(self) (self->time_start)
#define alarm_time_end(self)   (self->time_end)
#define alarm_length(self)     (self->length)
#define alarm_ringing(self)    (self->ringing)

/////////
// new //
/////////

void alarm_init( alarm_t * self, ulong_t n ) {
  alarm_length(self)      = n;
  alarm_time_start(self)  = SDL_GetTicks();
  alarm_time_end(self)    = alarm_time_start(self) + alarm_length(self);
  alarm_ringing(self)     = false();
}

alarm_t * alarm( ulong_t n ) {
  alarm_t * r = alloc(alarm_t);
  alarm_init(r,n);
  return r;
}

////////////
// events //
////////////

void alarm_update( alarm_t * self ) {
  if (alarm_time_end(self) < SDL_GetTicks()) {
    alarm_ringing(self) = true();
  }
  else {
    alarm_ringing(self) = false();
  }
}

bool_t alarm_is_ringing( alarm_t * self ) {
  return alarm_ringing(self);
}

void alarm_reset( alarm_t * self ) {
  alarm_time_start(self) = SDL_GetTicks();
  alarm_time_end(self) = alarm_time_start(self) + alarm_length(self);
}