
#define statuseffect_max_level() 20
#define statuseffect_max_time()  999999

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

} statuseffect_t;

#define statuseffect_level(self) (self->level)
#define statuseffect_time(self)  (self->time)
#define statuseffect_actor(self) (self->actor)

/////////
// new //
/////////

void statuseffect_init( statuseffect_t * self ) {
  statuseffect_level(self)  = 0;
  statuseffect_time(self)   = 0;
  statuseffect_actor(self)  = null();
}

statuseffect_t * statuseffect( ) {
  statuseffect_t * r = alloc(statuseffect_t);
  statuseffect_init(r);
  return r;
}

///////////
// funcs //
///////////

void statuseffect_set_time( statuseffect_t * self, int t ) {
  statuseffect_time(self) = clamp(t,0,statuseffect_max_time());
}

void statuseffect_set_level( statuseffect_t * self, int l ) {
  statuseffect_level(self) = clamp(l,0,statuseffect_max_level());
}