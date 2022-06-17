
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

statuseffect_t * statuseffect( ) {
  statuseffect_t * r = alloc(statuseffect_t);
  statuseffect_level(r)  = 0;
  statuseffect_time(r)   = 0;
  statuseffect_actor(r)  = null();
  return r;
}

///////////
// funcs //
///////////

void statuseffect_set_time( statuseffect_t * self, int t ) {
  statuseffect_time(self) = t;
}