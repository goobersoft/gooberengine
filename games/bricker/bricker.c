

#include "modules/paddle.c"
#include "modules/pball.c"

type() {

  // paddle object
  local( paddle_t * paddle );

  // ball object(s)
  local( list_t * balls );

  // score number, max at 99999999
  int score;
  // number of lives left
  int tries;
  // current level
  int level;

} bricker_t;

#define bricker_paddle(self) (self->paddle)
#define bricker_balls(self)  (self->balls)
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
  bricker_paddle(self) = paddle();
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
}

void bricker_load() {
  
  // type casting is for pussies :-)
  void * u;
  
  // asset loading is done from top-level, so you have to indicate
  // the full path to this source file.
  u = image("games/bricker/media/images/bricker-0.png");
  log("%p",u);

  gb_set_image("bricker-0",u);


  
  u = colormap_from_image(gb_get_image("bricker-0"));
  gb_set_colormap("bricker-0",u);
  

  font_t * f = font(null());
  font_set(f,
    gb_get_colormap("bricker-0"),
    350,0, 5,10, 10,10);
  gb_set_font("bricker-0",f);
  
}

void bricker_start() {
}

void bricker_update() {
}

void bricker_draw() {
  
  graph_set_font(gb_graph(),gb_get_font("bricker-0"));
  graph_draw_text(gb_graph(),0,100,"Hello world.");
  
}

void bricker_quit() {
}