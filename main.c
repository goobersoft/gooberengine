

#include <stdio.h>
#include <stdlib.h>

#include "/usr/include/SDL2/SDL.h"
#include "/usr/include/SDL2/SDL_mixer.h"
#include "/usr/include/SDL2/SDL_image.h"

#include "gb/gb.c"

//////////
// main //
//////////

int main(int c, char ** v) {
  // set prng seed
  srand(time(NULL));
  // initialize SDL2
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);
  Mix_Init(MIX_INIT_OGG|MIX_INIT_MOD);
  
  gb_init();
  gb_load();
  while (gb_running()) {
    gb_update();
    gb_draw();
    SDL_Delay(10);
  }
  gb_quit();
  SDL_Quit();

  return 0;
}