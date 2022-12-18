

#include "modules/paddle.c"
#include "modules/pball.c"
#include "modules/ui.c"
#include "modules/debug.c"

type() {

  // paddle object
  local( paddle_t * paddle );

  // ball object(s)
  local( list_t * balls );

  local ( br_ui_t * br_ui );

  // score number, max at 99999999
  int score;
  // number of lives left
  int lives;
  // current level
  int level;

} bricker_t;

#define bricker_paddle(self) (self->paddle)
#define bricker_balls(self)  (self->balls)
#define bricker_br_ui(self)  (self->br_ui)
#define bricker_score(self)  (self->score)
#define bricker_tries(self)  (self->tries)
#define bricker_level(self)  (self->level)

/////////////
// globals //
/////////////

bricker_t * _bricker;

/////////
// new //
/////////

void init_bricker( bricker_t * self ) {
  bricker_br_ui(self) = br_ui();
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
  graph_set_font(gb_graph(),gb_get_font("bricker-0"));
  bricker_debug_start(_bricker);
}

void bricker_update() {
  br_ui_update(bricker_br_ui(_bricker));
  bricker_debug_update(_bricker);
}

void bricker_draw() {

  int u = graph_set_intensity(gb_graph(),50);
  graph_draw_colormap(gb_graph(),0,0,gb_get_colormap("bricker-bg"));
  graph_set_intensity(gb_graph(),u);

  br_ui_draw( bricker_br_ui(_bricker) );

  bricker_debug_draw(_bricker);
  
}

void bricker_quit() {
  bricker_debug_quit(_bricker);
}