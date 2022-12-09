

//////////////
// includes //
//////////////

// modules
#include "modules/statuseffect.c"
#include "modules/actor.c"

// actors
#include "modules/actors/makeii.c"

// scenes
#include "modules/scenes/logo.c"
#include "modules/scenes/title.c"

// debug
#include "modules/debug/debug.c"

//////////
// type //
//////////


type() {

  // generic scene object, which will contain the parent scene
  scene_t * scene;

} ma_t;

#define ma_scene() (ma->scene)

/////////////
// globals //
/////////////

ma_t * ma;

///////////////
// functions //
///////////////

//
void ma_clear_scene() {
  if (ma_scene() != null()) {
    char * u  = scene_id(ma_scene());
    // check the tag ID and free appropriately.
    if (streq(u,"logo")) {
      free_ma_scene_logo( scene_parent(ma_scene()) );
    }
    else if (streq(u,"title")) {
      free_ma_scene_title( scene_parent(ma_scene()) );
    }
  }
}

void ma_set_scene( char * id ) {
  // clear the scene if it currently exists.
  ma_clear_scene();
  // check the ID param and make the scene appropriately.
  if (streq(id,"logo")) {
    ma_scene_logo_t * r = ma_scene_logo();
    ma_scene()          = ma_scene_logo_scene(r);
    ma_scene_logo_start(r);
  }
  else if (streq(id,"title")) {
    ma_scene_title_t * r = ma_scene_title();
    ma_scene()           = ma_scene_title_scene(r);
    ma_scene_title_start(r);
  }
}

void ma_update_scene() {
  char * c = scene_id(ma_scene());
  if (streq(c,"logo")) {
    ma_scene_logo_update(scene_parent(ma_scene()));
    if (scene_flag_finished(ma_scene())) ma_set_scene("title");
  }
}

void ma_draw_scene() {
  char * c = scene_id(ma_scene());

  if (streq(c,"logo")) {
    ma_scene_logo_draw(scene_parent(ma_scene()));
  }
}

////////////
// events //
////////////

void ma_init() {
  ma = alloc(ma_t);
  ma_debug_init();
}

void ma_load() {
  image_t * a = image("ma/media/images/ma-0.png", gb_visual() );
  assets_set_image( gb_assets(), "ma-0", a );
  assets_set_colormap( gb_assets(), "ma-0", colormap_from_image(a) );
  ma_debug_load();
}

void ma_start() {
  // start off by setting the scene to the logo scene.
  ma_set_scene("logo");
  ma_debug_start();
}

void ma_update() {
  ma_update_scene();
  ma_debug_update();
}

void ma_draw() {
  ma_draw_scene();
  ma_debug_draw();
}

void ma_quit() {
  ma_debug_quit();
}