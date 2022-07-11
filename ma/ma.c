

//////////////
// includes //
//////////////

#include "modules/statuseffect.c"
#include "modules/actor.c"


////////////
// events //
////////////

void ma_init() {
}

void ma_load() {
}

void ma_start() {
}

void ma_update() {
}

void ma_draw() {
}

void ma_quit() {
}


/////////////////////////////
// load cartridge function //
/////////////////////////////

cartridge_t * load_cartridge() {
  cartridge_t * c = cartridge();
  cartridge_set_name(c, "Magus Aora");

  cartridge_fn_init(c)   = ma_init;
  cartridge_fn_load(c)   = ma_load;
  cartridge_fn_start(c)  = ma_start;
  cartridge_fn_update(c) = ma_update;
  cartridge_fn_draw(c)   = ma_draw;
  cartridge_fn_quit(c)   = ma_quit;

  return c;
}