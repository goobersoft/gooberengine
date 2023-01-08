
// note that there's seven 9s here.
#define gameinfo_max_score()    9999999
#define gameinfo_max_time()     9999999
#define gameinfo_max_lives()    99
#define gameinfo_max_level()    999
#define gameinfo_max_hiscore()  9999999

//////////
// type //
//////////

type() {

  // score number, max at 9999999
  local( number_t * score );
  // high score (max 9999999)
  local( number_t * hiscore );
  // number of lives left
  local( number_t * lives );
  // current level
  local( number_t * level );
  // current time left
  local( number_t * time );

} gameinfo_t;

#define gameinfo_score(self)    (self->score)
#define gameinfo_hiscore(self)  (self->hiscore)
#define gameinfo_lives(self)    (self->lives)
#define gameinfo_level(self)    (self->level)
#define gameinfo_time(self)     (self->time)

#define gameinfo_set_score(self,n)    number_set(gameinfo_score(self),n)
#define gameinfo_add_score(self,n)    number_add(gameinfo_score(self),n)
#define gameinfo_set_hiscore(self,n)  number_set(gameinfo_hiscore(self),n)
#define gameinfo_set_lives(self,n)    number_set(gameinfo_lives(self),n)
#define gameinfo_add_lives(self,n)    number_add(gameinfo_lives(self),n)
#define gameinfo_set_level(self,n)    number_set(gameinfo_level(self),n)
#define gameinfo_inc_level(self)      number_add(gameinfo_level(self),1)
#define gameinfo_set_time(self,n)     number_set(gameinfo_time(self),n)
#define gameinfo_dec_time(self)       number_add(gameinfo_time(self),-1)

/////////
// new //
/////////

void init_gameinfo( gameinfo_t * self ) {
  gameinfo_score(self)    = number(0,0,gameinfo_max_score());
  gameinfo_hiscore(self)  = number(20000,0,gameinfo_max_hiscore());
  gameinfo_lives(self)    = number(3,0,gameinfo_max_lives());
  gameinfo_level(self)    = number(0,0,gameinfo_max_lives());
  gameinfo_time(self)     = number(0,0,gameinfo_max_time());
}

gameinfo_t * gameinfo() {
  gameinfo_t * self = alloc(gameinfo_t);
  init_gameinfo(self);
  return self;
}

///////////////
// functions //
///////////////


////////////
// events //
////////////

void gameinfo_update( gameinfo_t * self ) {
  // always dec time
  gameinfo_dec_time(self);
}

void gameinfo_draw( gameinfo_t * self ) {
}