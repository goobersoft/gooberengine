

//////////////
// includes //
//////////////

#include "modules/statuseffect.c"
#include "modules/actor.c"

#include "modules/actors/makeii.c"

#include "modules/ma_debug/ma_debug.c"

////////////
// events //
////////////

void ma_init() {
  ma_debug_init();
}

void ma_load() {
  image_t * a = image("ma/media/images/ma-0.png", gb_visual() );
  assets_set_image( gb_assets(), "ma-0", a );
  assets_set_colormap( gb_assets(), "ma-0", colormap_from_image(a) );
  ma_debug_load();
}

void ma_start() {
  ma_debug_start();
}

void ma_update() {
  ma_debug_update();
}

void ma_draw() {
  ma_debug_draw();
}

void ma_quit() {
  ma_debug_quit();
}