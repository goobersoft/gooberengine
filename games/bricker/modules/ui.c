
// maximums
#define brickerui_hiscore_max()      99999999
#define brickerui_score_max()        99999999
#define brickerui_lives_max()        999
#define brickerui_level_max()        999
#define brickerui_timer_max_sec()    9999
#define brickerui_timer_max_dsec()   99

// positions
#define brickerui_score_pos()        make_point(320,170)
#define brickerui_hiscore_pos()      make_point(10,220)
#define brickerui_lives_pos()        make_point(320,130)
#define brickerui_level_pos()        make_point(360,130)
#define brickerui_items_pos(x)       make_point(310+(30*x),90)
#define brickerui_timer_sec_pos()    make_point(320,50)
#define brickerui_timer_dsec_pos()   make_point(370,50)


type() {

  // score element
  int score;
  // destination score (score will accumulate until it hits
  // this value
  int score_dest;
  // hiscore value
  int hiscore;
  // lives value
  int lives;
  // level value
  int level;
  // items values (ids)
  int items[3];
  // seconds on the timer
  int timer_sec;
  // 100ths of a second on the timer
  // (this will be calculated by x*100/60)
  int timer_dsec;

  bool_t pressstart_active;
  int    pressstart_time;
  bool_t pressstart_yoff;

  foreign( colormap_t * colormap );

} brickerui_t;

#define brickerui_score(self)          (self->score)
#define brickerui_score_dest(self)     (self->score_dest)
#define brickerui_hiscore(self)        (self->hiscore)
#define brickerui_lives(self)          (self->lives)
#define brickerui_level(self)          (self->level)
#define brickerui_timer_sec(self)      (self->timer_sec)
#define brickerui_timer_dsec(self)     (self->timer_dsec)

#define brickerui_pressstart_active(self)   (self->pressstart_active)
#define brickerui_pressstart_yoff(self)     (self->pressstart_yoff)
#define brickerui_pressstart_time(self)     (self->pressstart_time)

#define brickerui_colormap(self) (self->colormap)

/////////
// new //
/////////

void free_brickerui( brickerui_t * self ) {
  // the object contains no pointers, so it's fine
  // to just free itself only.
  free(self);
}

void init_brickerui( brickerui_t * self ) {
  brickerui_score(self)        = 0;
  brickerui_score_dest(self)   = 0;
  brickerui_lives(self)        = 0;
  brickerui_level(self)        = 0;
  brickerui_timer_sec(self)    = 0;
  brickerui_timer_dsec(self)   = 0;
  brickerui_pressstart_active(self)  = true();
  brickerui_pressstart_yoff(self)    = 0;
  brickerui_pressstart_time(self)    = 0;
}

brickerui_t * brickerui() {
  brickerui_t * self = alloc(brickerui_t);
  init_brickerui(self);
  return self;
}

////////////
// events //
////////////

void brickerui_update( brickerui_t * self ) {

  if brickerui_pressstart_active(self) {
    brickerui_pressstart_time(self) += 1;
    if (brickerui_pressstart_time(self) >= 30) {
      brickerui_pressstart_yoff(self) = 1 - brickerui_pressstart_yoff(self);
      brickerui_pressstart_time(self) = 0;
    }
  }

}

void brickerui_draw( brickerui_t * self ) {
  // set the colormap if it is not set.
  if (brickerui_colormap(self) == null()) {
    brickerui_colormap(self) = gb_get_colormap("bricker-0");
  }

  // "press space" ui element
  if brickerui_pressstart_active(self) {
    graph_set_intensity_max( gb_graph() );
    graph_draw_colormap_sub( gb_graph(), 160, 115, brickerui_colormap(self),
      0,220+(10*brickerui_pressstart_yoff(self)),80,10);
    graph_reset_intensity( gb_graph() );
  }

  // hiscore element
  // stringify the hiscore value
  foreign() char * u = str(brickerui_hiscore(self));
  // pad right with 0s to the left
  u = right(u,8,'0');
  // 
}