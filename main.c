
///////////////
// libraries //
///////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include </usr/include/SDL2/SDL.h>
#include </usr/include/SDL2/SDL_mixer.h>
#include </usr/include/SDL2/SDL_image.h>
#include </usr/include/SDL2/SDL_net.h>

///////////////////////////////////////
// include the GB master module      //
// as well as the game being loaded. //
///////////////////////////////////////

#include "gb/gb.c"

///////////////////////////////////////
// choose which game to compile with //
///////////////////////////////////////

#define cart_none()      0
#define cart_ma()        1
#define cart_mac()       2

#define cart_current()   cart_ma()

#if cart_current()!=0
  #define cart_inserted()  1
#else
  #define cart_inserted()  0
#endif


#if cart_current()==cart_mac()
  #include "mac/mac.c"
#endif

#if cart_current()==cart_ma()
  #include "ma/ma.c"
#endif

////////////
// events //
////////////

void init() {
  // set prng seed
  srand        (time(NULL));
  // initialize SDL2
  SDL_Init     (SDL_INIT_EVERYTHING);
  IMG_Init     (IMG_INIT_PNG);
  Mix_Init     (MIX_INIT_OGG|MIX_INIT_MOD);
  SDLNet_Init  ();

  // open the audio device.
  Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 1, 4096 );


  cartridge_t * cartridge;

  if (cart_inserted()) {
    cartridge = load_cartridge();
  }

  gb_init(cartridge);
}

void load() {
  gb_load();
}

void start() {
  gb_start();
}

void update() {
  gb_update();
  gb_draw();
}

void draw() {
}

void quit() {
  gb_quit();
  SDL_Quit();
}

//////////
// main //
//////////

int main(int c, char ** v) {
  init();
  load();
  start();
  while (gb_running()) {
    update();
    draw();
  }
  quit();

  return 0;
}