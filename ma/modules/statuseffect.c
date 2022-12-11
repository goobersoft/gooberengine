
#define ma_statuseffect_max_level() 20
#define ma_statuseffect_max_time()  999999
#define ma_statuseffect_tick_max()  60

///////////
// notes //
///////////
/*
  StatusEffect is tick-based.
*/

//////////
// type //
//////////

type() {

  // a tag object
  local( tag_t * tag );
  // an arbitrary level.
  int       level;
  // the amount of time left in "seconds"
  int       time;
  // a ticker, typically starting with the tick max (60).
  int       time_ticks;
  // a boolean to indicate if the statuseffect is currently active.
  // if false, then the SE is dead and can be removed.
  bool_t    active;
  // a pointer to the source object (generic).
  foreign( void * source );

} ma_statuseffect_t;

#define ma_statuseffect_tag(self)    (self->tag)
#define ma_statuseffect_id(self)     tag_id(ma_statuseffect_tag(self))
#define ma_statuseffect_level(self)  (self->level)
#define ma_statuseffect_time(self)   (self->time)
#define ma_statuseffect_active(self) (self->active)
#define ma_statuseffect_source(self) (self->source)

/////////
// new //
/////////

void ma_statuseffect_init( ma_statuseffect_t * self ) {
  ma_statuseffect_tag(self)      = tag("statuseffect",null(),self);
  ma_statuseffect_level(self)    = 0;
  ma_statuseffect_time(self)     = 0;
  ma_statuseffect_active(self)   = true();
  ma_statuseffect_source(self)   = null();
}

ma_statuseffect_t * ma_statuseffect() {
  ma_statuseffect_t * r = alloc(ma_statuseffect_t);
  ma_statuseffect_init(r);
  return r;
}

///////////
// funcs //
///////////

void ma_statuseffect_update( ma_statuseffect_t * self ) {

}

void ma_statuseffect_set_id( ma_statuseffect_t * self, char * id ) {
  ma_statuseffect_id(self) = id;
}

void ma_statuseffect_set_time( ma_statuseffect_t * self, int t ) {
  ma_statuseffect_time(self) = clamp(t,0,ma_statuseffect_max_time());
}

void ma_statuseffect_set_level( ma_statuseffect_t * self, int l ) {
  ma_statuseffect_level(self) = clamp(l,0,ma_statuseffect_max_level());
}

//////////////
// subtypes //
//////////////

// drains LP every second
ma_statuseffect_t * ma_statuseffect__poison( int l ) {
  ma_statuseffect_t * r       = ma_statuseffect();
  ma_statuseffect_set_id      (r,"poison");
  ma_statuseffect_set_level   (r,l);
  ma_statuseffect_set_time    (r,30 + (ma_statuseffect_level(r) * 15));
  return r;
}

// restricts movement
ma_statuseffect_t * ma_statuseffect__stun( int l ) {
  ma_statuseffect_t * r       = ma_statuseffect();
  ma_statuseffect_set_id      (r,"stun");
  ma_statuseffect_set_level   (r,l);
  ma_statuseffect_set_time    (r,3 + ma_statuseffect_level(r));
  return r;
}