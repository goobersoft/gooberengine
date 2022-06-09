
//  submodules stack
//  ----------------
//  instead of using header files, GB64 uses a source stack.
//  this means that the modules that come after it have access
//  to the data and information of all previous modules in the
//  stack. This also means that modules earlier in the stack do
//  not have access to the modules that come after it.
//  The reason for this is to create safe dependencies in the code
//  and not to have too many cyclic references. Typically one module
//  will depend on one or modules defined before it.

/////////////
// modules //
/////////////

#include "modules/globals.c"            // includes many macro definitions
#include "modules/types.c"              // complex numerical types
#include "modules/functions.c"          // core functions
#include "modules/functions_sine.c"     // sine and cosine function
#include "modules/functions_sqroot.c"   // sqroot function
//#include "modules/rng.c"                // implementation of mersenne twister in 
#include "modules/tag.c"                // descriptive tag
#include "modules/list.c"               // linked list data type
#include "modules/string.c"             // helper function for immutable strings
#include "modules/point.c"              // 2d integer point
#include "modules/stack.c"              // LIFO-style stack object
#include "modules/number.c"             // integer number with min and max bounds
#include "modules/color.c"              // 6-bit color with transparency bit
#include "modules/image.c"              // uses SDL2_image
#include "modules/palette.c"            // storage of a list of 6-bit colors.
#include "modules/colormap.c"           // a 2d array of colors
#include "modules/sound.c"              // sound interface
#include "modules/font.c"               // bitmap fonts which utilize image colormaps
#include "modules/board.c"              // drawing durfaces
#include "modules/visual.c"             // window and renderer interface
#include "modules/mouse.c"              // mouse interface
#include "modules/assets.c"             // asset container object (sound/image/font)
#include "modules/audio.c"              // audio mixer
#include "modules/graph.c"              // drawing interface
#include "modules/alarm.c"              // timers
#include "modules/timing.c"             // fps/timing interface
#include "modules/gbs.c"                // GooberScript
#include "modules/sprite.c"             // instanced object with a reference to a colormap
#include "modules/scene.c"              // scene base object
#include "modules/controller.c"         // gamepads
#include "modules/network.c"            // networking (TCP/UDP)


/////////////
// gb type //
/////////////

typedef struct {

  int            running;
  int            paused;
  visual_t     * visual;
  timing_t     * timing;
  graph_t      * graph;
  assets_t     * assets;
  mouse_t      * mouse;
  audio_t      * audio;
  controller_t * controller;
  
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
#define gb()            (gb)
#define gb_assets()     (gb->assets)
#define gb_running()    (gb->running)
#define gb_paused()     (gb->paused)
#define gb_visual()     (gb->visual)
#define gb_timing()     (gb->timing)
#define gb_graph()      (gb->graph)
#define gb_mouse()      (gb->mouse)
#define gb_audio()      (gb->audio)
#define gb_controller() (gb->controller)

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
  gb()             = alloc(gb_t);
  gb_running()     = true();
  gb_paused()      = false();
  gb_visual()      = visual();
  gb_timing()      = timing();
  gb_assets()      = assets();
  gb_mouse()       = mouse(gb_visual(), null());
  gb_audio()       = audio();
  gb_graph()       = graph(gb_visual());
  gb_controller()  = controller();

  ///////////////
  // debugging //
  ///////////////

  debug_init();
  
}

void gb_load() {
  // load the default assets
  assets_load(gb_assets(),gb_visual());

  // set the mouse's icon
  mouse_colormap(gb_mouse())      = assets_get_colormap(gb_assets(),0);
  mouse_set_colormap_rect         (gb_mouse(),40,200,10,10);
  mouse_set_visible               (gb_mouse(),false());

  // debug
  debug_load();
}

void gb_update() {

  ////////////
  // timing //
  ////////////

  timing_update(gb_timing());

  ////////////////
  // sdl events //
  ////////////////

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      log("Quit" nl());
      gb_running() = false();
    }
  }
  
  /////////////
  // modules //
  /////////////

  mouse_update      (gb_mouse());
  controller_update (gb_controller());
  
  ///////////
  // debug //
  ///////////

  debug_update();
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
  graph_present(gb_graph());
  visual_draw(gb_visual());
}

void gb_quit() {

  debug_quit();

  free_visual(gb_visual());
  free_timing(gb_timing());
  free(gb);
}