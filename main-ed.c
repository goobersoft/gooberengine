
///////////////
// libraries //
///////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include </usr/include/SDL2/SDL.h>
#include </usr/include/SDL2/SDL_mixer.h>
#include </usr/include/SDL2/SDL_image.h>
#include </usr/include/SDL2/SDL_net.h>

/*
#include </usr/include/lua5.3/lua.h>
#include </usr/include/lua5.3/lauxlib.h>
#include </usr/include/lua5.3/lualib.h>
*/

///////////////////////////////////////
// include the GB master module      //
// as well as the game being loaded. //
///////////////////////////////////////

#include "gb/gb.c"
#include "apps/ed/ed.c"

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
  Mix_OpenAudio( 48000, MIX_DEFAULT_FORMAT, 1, 4096 );

  gb_init();
  ed_init();
}

void load() {
  gb_load();
  ed_load();
}

void start() {
  gb_start();
  ed_start();
}

void update() {
  gb_update();
  ed_update();
}

void draw() {
  gb_draw_pre();
  ed_draw();
  gb_draw_post();
}

void quit() {
  gb_quit();
  ed_quit();
  SDL_Quit();
}

//////////
// main //
//////////

int main(int c, char ** v) {
  init();
  load();
  start();
  while gb_running() {
    update();
    draw();
  }
  quit();

  return 0;
}