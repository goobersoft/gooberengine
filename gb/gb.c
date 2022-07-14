
#define gb_version_major() 0
#define gb_version_minor() 1
#define gb_version_patch() 5

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
#include "modules/locker.c"
//#include "modules/rng.c"                // implementation of mersenne twister in 
#include "modules/list.c"               // linked list data type
#include "modules/string.c"             // 
#include "modules/tag.c"                // descriptive tag
#include "modules/point.c"              // 2d integer point
#include "modules/collide.c"
#include "modules/stack.c"              // LIFO-style stack object
#include "modules/number.c"             // integer number with min and max bounds
#include "modules/color.c"              // 6-bit color with transparency bit
#include "modules/board.c"              // drawing durfaces
#include "modules/visual.c"             // window and renderer interface
#include "modules/image.c"              // uses SDL2_image
#include "modules/palette.c"            // storage of a list of 6-bit colors.
#include "modules/colormap.c"           // a 2d array of colors
#include "modules/sound.c"              // sound interface
#include "modules/font.c"               // bitmap fonts which utilize image colormaps
#include "modules/mouse.c"              // mouse interface
#include "modules/assets.c"             // asset container object (sound/image/font)
#include "modules/audio.c"              // audio mixer
#include "modules/alarm.c"              // timers
#include "modules/timing.c"             // fps/timing interface
#include "modules/gbs.c"                // GooberScript
#include "modules/sprite.c"             // instanced object with a reference to a colormap
#include "modules/scene.c"              // scene base object
#include "modules/controller.c"         // gamepads
#include "modules/network.c"            // networking (TCP/UDP)
#include "modules/cartridge.c"

#include "modules/graph.c"              // drawing interface

// load the toys submodule
#include "modules/toys/jake.c"


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

  cartridge_t  * cartridge;
  
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
#define gb_cartridge()  (gb->cartridge)

//////////////////////////
// special debug module //
//////////////////////////
// the debug module has visibility of the gb globals
// this will help with debugging immensely.

#include "modules/debugpanel.c"
#include "modules/debug.c"



////////////
// events //
////////////

void gb_init( cartridge_t * c ) {
  log("gooberengine - VER: %d.%d.%d",gb_version_major(),gb_version_minor(),gb_version_patch());

  // the master object does not have an allocation function.
  // instead it is done here in init()
  gb()             = alloc(gb_t);
  gb_running()     = true();
  gb_paused()      = false();
  gb_visual()      = visual();
  gb_timing()      = timing();
  gb_assets()      = assets(gb_visual());
  gb_mouse()       = mouse(gb_visual(), null());
  gb_audio()       = audio();
  gb_graph()       = graph(gb_visual());
  gb_controller()  = controller();

  // if the cart has been successfully loaded, call the init function.
  gb_cartridge() = c;
  if (gb_cartridge()) {
    cartridge_fn_init(gb_cartridge())();
  }

  ///////////////
  // debugging //
  ///////////////

  debug_init();
  
}

void gb_load() {
  // load the default assets
  // load images
  assets_t * a = gb_assets();
  visual_t * v = gb_visual();

  assets_set_image(a,0,image("gb/media/images/gb-0.png",v));
  assets_set_image(a,1,image("gb/media/images/gb-1.png",v));
  assets_set_image(a,2,image("gb/media/images/editor-bg.png",v));
  assets_set_image(a,3,image("gb/media/images/test.png",v));
  assets_set_image(a,4,image("gb/media/images/gb-color-test.png",v));

  // load colormaps from images
  assets_set_colormap(a,0,colormap_from_image(assets_get_image(a,0)));
  assets_set_colormap(a,1,colormap_from_image(assets_get_image(a,1)));
  assets_set_colormap(a,2,colormap_from_image(assets_get_image(a,2)));
  assets_set_colormap(a,3,colormap_from_image(assets_get_image(a,3)));
  assets_set_colormap(a,4,colormap_from_image(assets_get_image(a,4)));

  // load sounds
  assets_set_sound(a,0,sound("gb/media/sounds/jake-1.ogg"));
  assets_set_sound(a,1,sound("gb/media/sounds/jake-2.ogg"));
  assets_set_sound(a,2,sound("gb/media/sounds/jake-3.ogg"));
  assets_set_sound(a,3,sound("gb/media/sounds/jake-4.ogg"));

  // load font 1
  font_t * f;

  f = font( assets_get_colormap(a,0) );
  font_set_pos         ( f, 0, 100  );
  font_set_tile_size   ( f, 5, 10   );
  font_set_tiles_size  ( f, 10, 10  );
  assets_set_font      ( a, 0, f );

  graph_set_font( gb_graph(), f );
  
  f = font( assets_get_colormap(a,0) );
  font_set_pos         ( f, 0, 0   );
  font_set_tile_size   ( f, 10, 10 );
  font_set_tiles_size  ( f, 10, 10 );
  assets_set_font      ( a, 1, f );

  // set the mouse's icon
  mouse_colormap(gb_mouse())      = assets_get_colormap(gb_assets(),0);
  mouse_set_colormap_rect         (gb_mouse(),40,200,10,10);
  mouse_set_visible               (gb_mouse(),true());

  if (gb_cartridge()) {
    cartridge_fn_load(gb_cartridge())();
  }

  // debug
  debug_load();
}

void gb_start() {
  if (gb_cartridge()) {
    cartridge_fn_start(gb_cartridge())();
  }
  debug_start();
}

void gb_update() {

  ////////////
  // timing //
  ////////////

  timing_update_pre(gb_timing());

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

  const Uint8 * b = SDL_GetKeyboardState( null() );
  if (b[SDL_SCANCODE_ESCAPE]) {
    log("quit.");
    exit(0);
  }

  /////////////
  // modules //
  /////////////

  mouse_update      (gb_mouse());
  controller_update (gb_controller());
  
  if (gb_cartridge()) {
    cartridge_fn_update(gb_cartridge())();
  }

  debug_update_pre();

  /////////////////
  // timing post //
  /////////////////

  timing_update_post(gb_timing());

  ///////////
  // debug //
  ///////////

  debug_update_post();
  
}

void gb_draw() {
  
  // clear screen to black and set draw color back to white.
  visual_draw_pre(gb_visual());  
  // background drawing for debug
  debug_draw_pre();

  if (gb_cartridge()) {
    cartridge_fn_draw(gb_cartridge())();
  }

  // all of the module drawing goes in here
  graph_draw_mouse( gb_graph(), gb_mouse() );
  // do some debug drawing at the end of everything else
  debug_draw_post();
  // present the screen
  graph_present(gb_graph());
  visual_draw_post(gb_visual());
}

void gb_quit() {


  debug_quit();

  if (gb_cartridge()) {
    cartridge_fn_quit(gb_cartridge())();
  }

}