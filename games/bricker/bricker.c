

#include "modules/pball.c"
#include "modules/paddle.c"

#include "modules/ui.c"
#include "modules/brick.c"
#include "modules/playfield.c"
#include "modules/gameinfo.c"

#include "scenes/attract.c"

#include "modules/debug.c"

////////////
// consts //
////////////



//////////
// type //
//////////

type() {

  // ui object
  local( brickerui_t * ui );
  // scene object
  local( scene_t * scene );
  // gameinfo
  local( gameinfo_t * gameinfo );

} bricker_t;

#define bricker_ui(self)          (self->ui)
#define bricker_scene(self)       (self->scene)
#define bricker_gameinfo(self)    (self->gameinfo)

/////////////
// globals //
/////////////

bricker_t * _bricker;

/////////
// new //
/////////

void init_bricker( bricker_t * self ) {
  bricker_ui(self)        = brickerui();
  bricker_scene(self)     = scene_attract(self);
  bricker_gameinfo(self)  = gameinfo();
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

  // font
  font_t * f = font(null());
  font_set(f,
    gb_get_colormap("bricker-0"),
    350,0, 5,10, 10,10);
  gb_set_font("bricker-0",f);
  
  // debugging
  bricker_debug_load(_bricker);
}

void bricker_start() {
  visual_set_title( gb_visual(), "--- BRICKER!! ---");
  graph_set_font(gb_graph(),gb_get_font("bricker-0"));

  bricker_debug_start(_bricker);
}

void bricker_update() {

  gameinfo_update( bricker_gameinfo(_bricker) );
  brickerui_set_time( bricker_ui(_bricker), number_value(gameinfo_time(bricker_gameinfo(_bricker))) );

  tag_t * y = scene_tag( bricker_scene(_bricker) );
  if (streq(tag_id(y),"attract")) {
    scene_attract_t * s;
    s = scene_source( bricker_scene(_bricker) );
    scene_attract_update(s);
  }

  brickerui_update(bricker_ui(_bricker));
  bricker_debug_update(_bricker);
}

void bricker_draw() {

  int u = graph_set_intensity(gb_graph(),50);
  graph_draw_colormap(gb_graph(),0,0,gb_get_colormap("bricker-bg"));
  graph_set_intensity(gb_graph(),u);

  bricker_debug_draw_pre(_bricker);

  tag_t * y = scene_tag( bricker_scene(_bricker) );
  if (streq(tag_id(y),"attract")) {
    scene_attract_t * s;
    s = scene_source( bricker_scene(_bricker) );
    scene_attract_update(s);
  }

  //playfield_draw( bricker_playfield(_bricker) );
  brickerui_draw( bricker_ui(_bricker) );
  
  bricker_debug_draw_post(_bricker);
}

void bricker_quit() {
  bricker_debug_quit(_bricker);
}