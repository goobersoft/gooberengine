

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// submodules stack
//  instead of using header files, GB64 uses a source stack.
//  this means that the modules that come after it have access
//  to the data and information of all previous modules in the
//  stack. This also means that modules earlier in the stack do
//  not have access to the modules that come after it.
//  The reason for this is to create safe dependencies in the code
//  and not to have too many cyclic references. Typically one module
//  will depend on one or modules defined before it.

// generic
#include "modules/globals.c"
#include "modules/types.c"
#include "modules/functions.c"
#include "modules/functions_sine.c"
#include "modules/functions_sqroot.c"
// specific
#include "modules/string.c"
#include "modules/point.c"
#include "modules/stack.c"
#include "modules/number.c"
#include "modules/color.c"
#include "modules/image.c"
#include "modules/palette.c"
#include "modules/colormap.c"
#include "modules/mouse.c"
#include "modules/sound.c"
#include "modules/font.c"
#include "modules/board.c"
#include "modules/visual.c"
#include "modules/assets.c"
#include "modules/graph.c"
#include "modules/timing.c"

#include "modules/sprite.c"
#include "modules/scene.c"


/////////////
// globals //
/////////////

typedef struct {

  int           running;
  int           paused;
  visual_t    * visual;
  timing_t    * timing;
  graph_t     * graph;
  assets_t    * assets;
  mouse_t     * mouse;
  
} gb_t;

/////////////
// globals //
/////////////

gb_t * gb;

///////////////
// functions //
///////////////

// these are essentially convenience getters
// also I hate using the -> operator.
#define gb()         (gb)
#define gb_assets()  (gb->assets)
#define gb_running() (gb->running)
#define gb_paused()  (gb->paused)
#define gb_visual()  (gb->visual)
#define gb_timing()  (gb->timing)
#define gb_graph()   (gb->graph)
#define gb_mouse()   (gb->mouse)

//////////////////////////
// special debug module //
//////////////////////////
// the debug module has visibility of the gb globals
// this will help with debugging immensely.

#include "modules/debug.c"

////////////
// events //
////////////

void gb_init() {

  // the master object does not have an allocation function.
  // instead it is done here in init()
  gb()          = alloc(gb_t);
  gb_running()  = true();
  gb_paused()   = false();
  gb_visual()   = visual();
  gb_timing()   = timing();
  gb_assets()   = assets();
  gb_mouse()    = mouse();
  gb_graph()    = graph(gb_visual());

  ///////////////
  // debugging //
  ///////////////

  debug_init();
  
}

void gb_load() {
  // load the default assets
  assets_load(gb_assets(),gb_visual());
  
  // set the mouse's icon
  mouse_image(gb_mouse()) = assets_get_image(gb_assets(),0);
  point_set( mouse_image_pos(gb_mouse()), tile10(4), tile10(20) );
  point_set( mouse_image_size(gb_mouse()), tile10(1), tile10(1) );
  SDL_ShowCursor(SDL_DISABLE);
  // debug
  debug_load();
}

void gb_update() {

  // update timing module to be right before everything
  // is updated
  timing_update(gb_timing());
  // log("%ld" nl(), timing_diff(gb_timing()));

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      log("Quit" nl());
      gb_running() = false();
    }
  }

  // all of the module events go here.
  mouse_update(gb_mouse());
  // finally, debug
  debug_update();

  // update the timing module again to get the amount of time
  // that has passed since everything updated.
  // the difference will be used to determine the amount to
  // sleep for.
  // timing_update(gb_timing());
  // log("%ld" nl(), tick_delay()-timing_diff(gb_timing()));
}

void gb_draw() {
  
  // clear screen to black and set draw color back to white.
  visual_draw_pre(gb_visual());  
  // background drawing for debug
  debug_draw_pre();
  // all of the module drawing goes in here
  graph_draw_mouse( gb_graph(), gb_mouse() );
  // do some debug drawing at the end of everything else
  debug_draw_post();
  // present the screen
  visual_draw(gb_visual());
}

void gb_quit() {

  debug_quit();

  free_visual(gb_visual());
  free_timing(gb_timing());
  free(gb);
}