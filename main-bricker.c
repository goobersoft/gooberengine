
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
#include </usr/include/lua5.1/lua.h>
#include </usr/include/lua5.1/lauxlib.h>
#include </usr/include/lua5.1/lualib.h>
*/

///////////////////////////////////////
// include the GB master module      //
// as well as the game being loaded. //
///////////////////////////////////////

#include "gb/gb.c"
#include "games/bricker/bricker.c"

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
  bricker_init();
}

void load() {
  gb_load();
  bricker_load();
}

void start() {
  gb_start();
  bricker_start();
}

void update() {
  gb_update();
  if (visual_fpstick(gb_visual())==0) {
    bricker_update();
  }
}

void draw() {
  gb_draw_pre();
  if (visual_fpstick(gb_visual())==0) {
    bricker_draw();
  }
  gb_draw_post();
}

void quit() {
  gb_quit();
  bricker_quit();
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