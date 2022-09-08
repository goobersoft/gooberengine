
type() {

  bool_t      active;
  // ref to actor object
  actor_t   * actor;
  // life points
  number_t  * lp;
  // ability points
  number_t  * ap;


} makeii_t;

#define makeii_active(self) (self->active)
#define makeii_actor(self)  (self->actor)
#define makeii_lp(self)     (self->lp)
#define makeii_ap(self)     (self->ap)

/////////
// new //
/////////

makeii_t * makeii() {
  makeii_t * r      = alloc(makeii_t);
  makeii_active(r)  = true();
  makeii_actor(r)   = actor();
  makeii_lp(r)      = number(8,0,8);
  makeii_ap(r)      = number(8,0,8);
  return r;
}