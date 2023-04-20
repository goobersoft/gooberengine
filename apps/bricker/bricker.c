
// entities

#include "modules/actors/paddle.c"
#include "modules/actors/brick.c"
#include "modules/actors/pball.c"


#include "modules/graphical/playfield.c"
#include "modules/graphical/leddisplay.c"
#include "modules/graphical/brickerui.c"

#include "modules/scenes/attract.c"
#include "modules/scenes/game.c"
#include "modules/scenes/menu.c"
#include "modules/scenes/test.c"

#include "modules/components/gameinfo.c"
#include "modules/components/scenemgr.c"

#include "modules/debug.c"

//////////
// type //
//////////

type() {
  // scene object
  //local( scene_t * scene );
  // scene manager
  local( bricker_scenemgr_t * scmgr );
  // gameinfo
  local( gameinfo_t * gameinfo );

} bricker_t;

//#define bricker_scene(self)     (self->scene)
#define bricker_scmgr(self)     (self->scmgr)
#define bricker_gameinfo(self)  (self->gameinfo)

#define bricker_scene(self)     bricker_scenemgr_scene(bricker_scmgr(self))

/////////////
// globals //
/////////////

bricker_t * _bricker;

/////////
// new //
/////////

void init_bricker( bricker_t * self ) {
  //bricker_scene(self)   = scene("none",null());
  bricker_scmgr(self)     = bricker_scenemgr();
  bricker_gameinfo(self)  = gameinfo();
}

bricker_t * bricker(void) {
  bricker_t * r = alloc(bricker_t);
  init_bricker( r );
  return r;
}

///////////////
// functions //
///////////////

void bricker_clear_scene( bricker_t * self ) {
  bricker_scenemgr_clear_scene( bricker_scmgr(self) );
}

void bricker_set_scene( bricker_t * self, char * id ) {
  bricker_scenemgr_set_scene(bricker_scmgr(self),id);
}

////////////
// events //
////////////

//-- Init --//
void bricker_init() {
  _bricker = bricker();
  bricker_debug_init(_bricker);
}

//-- Load --//
void bricker_load() {
  
  // type casting is for pussies :-)
  void * u;
  
  // asset loading is done from top-level, so you have to indicate
  // the full path to this source file.

  // tileset
  u = image("apps/bricker/media/images/bricker-0.png");
  gb_set_image("bricker-0",u);
  u = colormap_from_image(gb_get_image("bricker-0"));
  gb_set_colormap("bricker-0",u);
  

  // background
  u = image("apps/bricker/media/images/bricker-bg2.png");
  gb_set_image("bricker-bg",u);
  u = colormap_from_image(gb_get_image("bricker-bg"));
  gb_set_colormap("bricker-bg",u);

  // background (opaque)
  u = image("apps/bricker/media/images/bricker-bg.png");
  gb_set_image("bricker-bg-opaque",u);
  u = colormap_from_image(gb_get_image("bricker-bg-opaque"));
  gb_set_colormap("bricker-bg-opaque",u);

  // font (normal)
  font_t * f = font(null());
  font_set(f,
    gb_get_colormap("bricker-0"),
    350,0, 5,10, 10,10);
  gb_set_font("bricker-0",f);
  // font (underlined)
  f = font(null());
  font_set(f,
    gb_get_colormap("bricker-0"),
    300,0, 5,10, 10,10);
  gb_set_font("bricker-1",f);

  // sounds
  gb_set_sound("ball-1",sound("apps/bricker/media/sounds/ball1.wav"));
  gb_set_sound("ball-2",sound("apps/bricker/media/sounds/ball2.wav"));
  gb_set_sound("bounce-1",sound("apps/bricker/media/sounds/bounce1.wav"));
  gb_set_sound("bounce-2",sound("apps/bricker/media/sounds/bounce2.wav"));
  gb_set_sound("brick-1",sound("apps/bricker/media/sounds/brick1.wav"));
  gb_set_sound("brick-2",sound("apps/bricker/media/sounds/brick2.wav"));
  gb_set_sound("dizzy-1",sound("apps/bricker/media/sounds/dizzy1.wav"));
  gb_set_sound("fall-1",sound("apps/bricker/media/sounds/fall1.wav"));
  gb_set_sound("laser-1",sound("apps/bricker/media/sounds/laser1.wav"));
  gb_set_sound("menu-1",sound("apps/bricker/media/sounds/menu1.wav"));
  gb_set_sound("menu-2",sound("apps/bricker/media/sounds/menu2.wav"));
  gb_set_sound("next-1",sound("apps/bricker/media/sounds/next1.wav"));
  gb_set_sound("ok-1",sound("apps/bricker/media/sounds/ok1.wav"));
  gb_set_sound("respawn-1",sound("apps/bricker/media/sounds/respawn1.wav"));

  // debugging
  bricker_debug_load(_bricker);
}

//-- Start --//
void bricker_start() {
  // set the window title
  visual_set_title( gb_visual(), "--- BRICKER!! ---");
  // set the font to the new bricker font
  graph_set_font(gb_graph(),gb_get_font("bricker-0"));
  // set to attract scene

  bricker_set_scene(_bricker, "attract" );
  // start the debug event

  bricker_debug_start(_bricker);
}

//-- Update --//
void bricker_update() {
  // update the gameinfo
  gameinfo_update( bricker_gameinfo(_bricker) );
  // update the scene manager
  bricker_scenemgr_update(bricker_scmgr(_bricker));
  gb_scene() = bricker_scene(_bricker);
  // update the debug module
  bricker_debug_update(_bricker);
}

//-- Draw --//
void bricker_draw() {
  bricker_debug_draw_pre(_bricker);
  bricker_scenemgr_draw(bricker_scmgr(_bricker));
  // draw the debug
  bricker_debug_draw_post(_bricker);
}

//-- Quit --//
void bricker_quit() {
  bricker_debug_quit(_bricker);
}