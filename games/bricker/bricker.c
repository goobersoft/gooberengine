
// entities
#include "modules/entities/pball.c"
#include "modules/entities/paddle.c"
#include "modules/entities/brick.c"

#include "modules/core/gameinfo.c"

#include "modules/graphical/ui.c"
#include "modules/graphical/playfield.c"

#include "modules/scenes/attract.c"
#include "modules/scenes/game.c"
#include "modules/scenes/menu.c"

#include "modules/debug.c"

////////////
// consts //
////////////

#define bricker_scene_none()    0
#define bricker_scene_attract() 1
#define bricker_scene_menu()    2
#define bricker_scene_game()    3

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
  bricker_scene(self)     = null();
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

void bricker_set_scene( bricker_t * self, int n ) {
  
  // delete the old scene
  if (bricker_scene(self)) {
    tag_t * ss = scene_tag(bricker_scene(self));

    if streq(tag_id(ss),"attract") {
      sceneattract_t * u = scene_source(bricker_scene(self));
      sceneattract_quit(u);
      free_sceneattract(u);
    }
    else if streq(tag_id(ss),"menu") {
      scenemenu_t * u = scene_source(bricker_scene(self));
      scenemenu_quit(u);
      free_scenemenu(u);
    }
    else if streq(tag_id(ss),"game") {
      scenegame_t * u = scene_source(bricker_scene(self));
      scenegame_quit(u);
      free_scenegame(u);
    }
  }

  // set the new scene
  if (n == bricker_scene_none()) {
    bricker_scene(_bricker) = null();
  }
  else if (n == bricker_scene_attract()) {
    sceneattract_t * u = sceneattract(_bricker);
    sceneattract_start(u);
    // set the scene to the new attract scene
    bricker_scene(_bricker) = sceneattract_scene(u);
  }
  else if (n == bricker_scene_menu()) {
    scenemenu_t * u = scenemenu(_bricker);
    scenemenu_start(u);
    bricker_scene(_bricker) = scenemenu_scene(u);
  }
  else if (n == bricker_scene_game()) {
    scenegame_t * u = scenegame(_bricker);
    scenegame_start(u);
    // set the scene to the new attract scene
    bricker_scene(_bricker) = scenegame_scene(u);
  }

  // set the gb scene ref so debugpanel can see it
  gb_scene() = bricker_scene(_bricker);
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
  u = image("games/bricker/media/images/bricker-0.png");
  gb_set_image("bricker-0",u);
  u = colormap_from_image(gb_get_image("bricker-0"));
  gb_set_colormap("bricker-0",u);

  // background
  u = image("games/bricker/media/images/bricker-bg2.png");
  gb_set_image("bricker-bg",u);
  u = colormap_from_image(gb_get_image("bricker-bg"));
  gb_set_colormap("bricker-bg",u);

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
  gb_set_sound("ball-1",sound("games/bricker/media/sounds/ball1.wav"));
  gb_set_sound("ball-2",sound("games/bricker/media/sounds/ball2.wav"));
  gb_set_sound("bounce-1",sound("games/bricker/media/sounds/bounce1.wav"));
  gb_set_sound("bounce-2",sound("games/bricker/media/sounds/bounce2.wav"));
  gb_set_sound("brick-1",sound("games/bricker/media/sounds/brick1.wav"));
  gb_set_sound("brick-2",sound("games/bricker/media/sounds/brick2.wav"));
  gb_set_sound("dizzy-1",sound("games/bricker/media/sounds/dizzy1.wav"));
  gb_set_sound("fall-1",sound("games/bricker/media/sounds/fall1.wav"));
  gb_set_sound("laser-1",sound("games/bricker/media/sounds/laser1.wav"));
  gb_set_sound("menu-1",sound("games/bricker/media/sounds/menu1.wav"));
  gb_set_sound("menu-2",sound("games/bricker/media/sounds/menu2.wav"));
  gb_set_sound("next-1",sound("games/bricker/media/sounds/next1.wav"));
  gb_set_sound("ok-1",sound("games/bricker/media/sounds/ok1.wav"));
  gb_set_sound("respawn-1",sound("games/bricker/media/sounds/respawn1.wav"));

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
  bricker_set_scene(_bricker, bricker_scene_attract() );
  // start the debug event
  bricker_debug_start(_bricker);
}

//-- Update --//
void bricker_update() {
  // update the gameinfo
  gameinfo_update( bricker_gameinfo(_bricker) );
  // set the time for the ui
  brickerui_set_time( bricker_ui(_bricker), number_value(gameinfo_time(bricker_gameinfo(_bricker))) );

  // is there a scene?
  if (bricker_scene(_bricker)) {
    tag_t * y = scene_tag( bricker_scene(_bricker) );
    // attract
    if (streq(tag_id(y),"attract")) {
      sceneattract_t * s;
      s = scene_source( bricker_scene(_bricker) );
      sceneattract_update(s);
      // if user presses space, change to game
      if (scene_finished(sceneattract_scene(s))) {
        // disable the press start ui element
        brickerui_pressstart_active(bricker_ui(_bricker)) = false();
        // switch to game scene
        bricker_set_scene( _bricker, bricker_scene_menu() );
      }

    }
    else if streq(tag_id(y),"menu") {
      scenemenu_t * s;
      s = scene_source( bricker_scene(_bricker) );
      scenemenu_update(s);
    }
    // game
    else if streq(tag_id(y),"game") {
      scenegame_t * s;
      s = scene_source( bricker_scene(_bricker) );
      scenegame_update(s);
    }
  }

  // update the ui
  brickerui_update(bricker_ui(_bricker));
  // update the debug module
  bricker_debug_update(_bricker);
}

//-- Draw --//
void bricker_draw() {

  graph_set_intensity(gb_graph(),50);
  graph_draw_colormap(gb_graph(),0,0,gb_get_colormap("bricker-bg"));
  graph_reset_intensity(gb_graph());

  bricker_debug_draw_pre(_bricker);

  // is there a scene?
  if (bricker_scene(_bricker)) {
    tag_t * y = scene_tag( bricker_scene(_bricker) );
    // attract
    if (streq(tag_id(y),"attract")) {
      sceneattract_t * s;
      s = scene_source( bricker_scene(_bricker) );
      sceneattract_draw(s);
    }
    else if streq(tag_id(y),"menu") {
      scenemenu_t * s;
      s = scene_source( bricker_scene(_bricker) );
      scenemenu_draw(s);
    }
    // game
    else if streq(tag_id(y),"game") {
      scenegame_t * s;
      s = scene_source( bricker_scene(_bricker) );
      scenegame_draw(s);
    }
  }

  // draw the ui
  brickerui_draw( bricker_ui(_bricker) );
  // draw the debug
  bricker_debug_draw_post(_bricker);
}

//-- Quit --//
void bricker_quit() {
  bricker_debug_quit(_bricker);
}