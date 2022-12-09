
#define ma_statuseffect_max_level() 20
#define ma_statuseffect_max_time()  999999

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

  int  level;
  int  time;

  // ref to the actor holding the SE.
  void * actor;

} ma_statuseffect_t;

#define ma_statuseffect_level(self) (self->level)
#define ma_statuseffect_time(self)  (self->time)
#define ma_statuseffect_actor(self) (self->actor)

/////////
// new //
/////////

void ma_statuseffect_init( ma_statuseffect_t * self ) {
  ma_statuseffect_level(self)  = 0;
  ma_statuseffect_time(self)   = 0;
  ma_statuseffect_actor(self)  = null();
}

ma_statuseffect_t * ma_statuseffect( ) {
  ma_statuseffect_t * r = alloc(ma_statuseffect_t);
  ma_statuseffect_init(r);
  return r;
}

///////////
// funcs //
///////////

void ma_statuseffect_set_time( ma_statuseffect_t * self, int t ) {
  ma_statuseffect_time(self) = clamp(t,0,ma_statuseffect_max_time());
}

void ma_statuseffect_set_level( ma_statuseffect_t * self, int l ) {
  ma_statuseffect_level(self) = clamp(l,0,ma_statuseffect_max_level());
}