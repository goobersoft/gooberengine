

#include "modules/paddle.c"
#include "modules/pball.c"
#include "modules/ui.c"

#include "scenes/attract.c"

#include "modules/debug.c"

////////////
// consts //
////////////

#define bricker_score_max() 9999999

//////////
// type //
//////////

type() {

  // ui object
  local ( brickerui_t * ui );

  // scene object
  local ( scene_t * scene );

  // score number, max at 9999999
  int score;
  // high score (max 9999999)
  int hiscore;
  // number of lives left
  int lives;
  // current level
  int level;
  // current time left
  int time;

} bricker_t;

#define bricker_ui(self)      (self->ui)
#define bricker_scene(self)   (self->scene)
#define bricker_score(self)   (self->score)
#define bricker_hiscore(self) (self->hiscore)
#define bricker_lives(self)   (self->lives)
#define bricker_level(self)   (self->level)
#define bricker_time(self)    (self->time)

/////////////
// globals //
/////////////

bricker_t * _bricker;

/////////
// new //
/////////

void init_bricker( bricker_t * self ) {
  bricker_ui(self) = brickerui();
  bricker_scene(self)    = scene_attract(self);
  bricker_score(self)    = 0;
  bricker_hiscore(self)  = seconds(10000);
  bricker_lives(self)    = 3;
  bricker_level(self)    = 0;
  bricker_time(self)     = 60000;
}

bricker_t * bricker() {
  bricker_t * r = alloc(bricker_t);
  init_bricker( r );
  return r;
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void bricker_init() {
  _bricker = bricker();
  bricker_debug_init(_bricker);
}

void bricker_load() {
  
  // type casting is for pussies :-)
  void * u;
  
  // asset loading is done from top-level, so you have to indicate
  // the full path to this source file.

  // tileset
  u = image("games/bricker/media/images/bricker-0.png");
  gb_set_image("bricker-0",u);
  u = colormap_from_image(gb_get_image("bricker-0"));
  gb_set_colormap("bricker-0",u);

  // background
  u = image("games/bricker/media/images/bricker-bg2.png");
  gb_set_image("bricker-bg",u);
  u = colormap_from_image(gb_get_image("bricker-bg"));
  gb_set_colormap("bricker-bg",u);

  font_t * f = font(null());
  font_set(f,
    gb_get_colormap("bricker-0"),
    350,0, 5,10, 10,10);
  gb_set_font("bricker-0",f);
  
  bricker_debug_load(_bricker);
}

void bricker_start() {
  visual_set_title( gb_visual(), "--- BRICKER!! ---");
  graph_set_font(gb_graph(),gb_get_font("bricker-0"));
  bricker_debug_start(_bricker);
}

void bricker_update() {

  bricker_time(_bricker) = low(bricker_time(_bricker)-1,0);
  brickerui_set_time( bricker_ui(_bricker), bricker_time(_bricker) );

  brickerui_update(bricker_ui(_bricker));
  bricker_debug_update(_bricker);
}

void bricker_draw() {

  int u = graph_set_intensity(gb_graph(),50);
  graph_draw_colormap(gb_graph(),0,0,gb_get_colormap("bricker-bg"));
  graph_set_intensity(gb_graph(),u);

  brickerui_draw( bricker_ui(_bricker) );

  bricker_debug_draw(_bricker);
  
}

void bricker_quit() {
  bricker_debug_quit(_bricker);
}