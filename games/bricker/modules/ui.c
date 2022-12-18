
// maximums
#define br_ui_hiscore_max()      99999999
#define br_ui_score_max()        99999999
#define br_ui_lives_max()        999
#define br_ui_level_max()        999
#define br_ui_timer_max_sec()    9999
#define br_ui_timer_max_dsec()   99

// positions
#define br_ui_score_pos()        make_point(320,170)
#define br_ui_hiscore_pos()      make_point(10,220)
#define br_ui_lives_pos()        make_point(320,130)
#define br_ui_level_pos()        make_point(360,130)
#define br_ui_items_pos(x)       make_point(310+(30*x),90)
#define br_ui_timer_sec_pos()    make_point(320,50)
#define br_ui_timer_dsec_pos()   make_point(370,50)


type() {

  bool_t visible;

  int score;
  int hiscore;
  int score_dest;
  int lives;
  int level;
  int items[3];
  int timer_sec;
  int timer_dsec;

  bool_t pressstart_active;
  int    pressstart_time;
  bool_t pressstart_yoff;

  foreign( colormap_t * colormap );

} br_ui_t;

#define br_ui_visible(self)        (self->visible)
#define br_ui_score(self)          (self->score)
#define br_ui_score_dest(self)     (self->score_dest)
#define br_ui_lives(self)          (self->lives)
#define br_ui_level(self)          (self->level)
#define br_ui_timer_sec(self)      (self->timer_sec)
#define br_ui_timer_dsec(self)     (self->timer_dsec)

#define br_ui_pressstart_active(self)   (self->pressstart_active)
#define br_ui_pressstart_yoff(self)     (self->pressstart_yoff)
#define br_ui_pressstart_time(self)     (self->pressstart_time)

#define br_ui_colormap(self) (self->colormap)

/////////
// new //
/////////

void free_br_ui( br_ui_t * self ) {
  // the object contains no pointers, so it's fine
  // to just free itself only.
  free(self);
}

void init_br_ui( br_ui_t * self ) {
  br_ui_visible(self)      = true();
  br_ui_score(self)        = 0;
  br_ui_score_dest(self)   = 0;
  br_ui_lives(self)        = 0;
  br_ui_level(self)        = 0;
  br_ui_timer_sec(self)    = 0;
  br_ui_timer_dsec(self)   = 0;
  br_ui_pressstart_active(self)  = true();
  br_ui_pressstart_yoff(self)    = 0;
  br_ui_pressstart_time(self)    = 0;
}

br_ui_t * br_ui() {
  br_ui_t * self = alloc(br_ui_t);
  init_br_ui(self);
  return self;
}

////////////
// events //
////////////

void br_ui_update( br_ui_t * self ) {

  if br_ui_pressstart_active(self) {
    br_ui_pressstart_time(self) += 1;
    if (br_ui_pressstart_time(self) >= 30) {
      br_ui_pressstart_yoff(self) = 1 - br_ui_pressstart_yoff(self);
      br_ui_pressstart_time(self) = 0;
    }
  }

}

void br_ui_draw( br_ui_t * self ) {

  if (br_ui_colormap(self) == null()) {
    br_ui_colormap(self) = gb_get_colormap("bricker-0");
  }

  if br_ui_pressstart_active(self) {
    int u = graph_set_intensity_max( gb_graph() );
    graph_draw_colormap_sub( gb_graph(), 160, 115, br_ui_colormap(self),
      0,220+(10*br_ui_pressstart_yoff(self)),80,10);
    graph_set_intensity( gb_graph(), u );
  }
}