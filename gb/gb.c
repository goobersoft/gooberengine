
#define gb_version_major() 0
#define gb_version_minor() 1
#define gb_version_patch() 8

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

#include "modules/core/globals.c"            // includes many macro definitions
#include "modules/core/functions.c"          // core functions
#include "modules/core/functions_sine.c"     // sine and cosine function
#include "modules/core/functions_sqroot.c"   // sqroot function
#include "modules/core/functions_base64.c"   // sqroot function
#include "modules/core/types.c"              // complex numerical types
#include "modules/core/tag.c"                // descriptive tag
#include "modules/core/locker.c"
//#include "modules/rng.c"              // implementation of mersenne twister in c
#include "modules/core/string.c"             // fixed-length character arrays
#include "modules/core/list.c"               // linked list data type
#include "modules/core/dict.c"               // dictionary
#include "modules/core/pstack.c"             // LIFO-style priority stack object
#include "modules/core/point.c"              // 2d integer point
#include "modules/core/point3.c"              // 2d integer point
#include "modules/core/rect.c"
#include "modules/core/number.c"             // integer number with min and max bounds
#include "modules/core/alarm.c"              // timers

#include "modules/graphical/color.c"              // 6-bit color with transparency bit
#include "modules/graphical/board.c"              // drawing durfaces
#include "modules/graphical/visual.c"             // window and renderer interface
#include "modules/graphical/image.c"              // uses SDL2_image
#include "modules/graphical/palette.c"            // storage of a list of 6-bit colors.
#include "modules/graphical/colormap.c"           // a 2d array of colors
#include "modules/graphical/font.c"               // bitmap fonts which utilize image colormaps
#include "modules/graphical/sprite.c"             // instanced object with a reference to a colormap
#include "modules/graphical/tilemap.c"            // instanceable grids of variable colormap sections


#include "modules/input/input.c"              // user input
#include "modules/input/mouse.c"              // mouse interface
#include "modules/input/controller.c"         // gamepads

#include "modules/audio/sound.c"              // sound interface
#include "modules/audio/audio.c"              // audio mixer

#include "modules/network/network.c"            // networking (TCP/UDP)

#include "modules/components/assets.c"             // asset container object (sound/image/font)
#include "modules/components/timing.c"             // fps/timing interface
#include "modules/components/graph.c"              // drawing interface

///////////////////////
// top layer objects //
///////////////////////

#include "modules/objects/gbs.c"                // GooberScript
#include "modules/objects/gbml.c"               // markup
#include "modules/objects/entity.c"             // AABB collisions
#include "modules/objects/scene.c"              // scene base object

#include "modules/objects/graphical/sprayer.c"
#include "modules/objects/graphical/camera.c"             // determines how a scene should render

#include "modules/objects/toys/jake.c"
#include "modules/objects/toys/ball.c"
#include "modules/objects/toys/fountain.c"



/////////////
// gb type //
/////////////

typedef struct {

  int            running;
  int            paused;
  bool_t         flag_cls;
  bool_t         flag_debug;
  visual_t     * visual;
  timing_t     * timing;
  graph_t      * graph;
  assets_t     * assets;
  mouse_t      * mouse;
  audio_t      * audio;
  controller_t * controller;

  scene_t      * scene;
  
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
#define gb_flag_cls()   (gb->flag_cls)
#define gb_flag_debug() (gb->flag_debug)
#define gb_paused()     (gb->paused)
#define gb_visual()     (gb->visual)
#define gb_timing()     (gb->timing)
#define gb_graph()      (gb->graph)
#define gb_mouse()      (gb->mouse)
#define gb_audio()      (gb->audio)
#define gb_controller() (gb->controller)
#define gb_scene()      (gb->scene)

//////////////////////
// helper functions //
//////////////////////

#define gb_get_colormap(c)   assets_get_colormap(gb_assets(),c)
#define gb_get_sound(s)      assets_get_sound(gb_assets(),s)
#define gb_get_font(f)       assets_get_font(gb_assets(),f)

//////////////////////////
// special debug module //
//////////////////////////
// the debug module has visibility of the gb globals
// this will help with debugging immensely.

#include "modules/debug/debugpanel.c"
#include "modules/debug/debug.c"

////////////
// events //
////////////

void gb_init() {
  log("gooberengine - VER: %d.%d.%d",gb_version_major(),gb_version_minor(),gb_version_patch());

  // the master object does not have an allocation function.
  // instead it is done here in init()
  gb()             = alloc(gb_t);
  gb_running()     = true();
  gb_paused()      = false();
  gb_flag_cls()    = false();
  gb_flag_debug()  = true();//false();
  gb_visual()      = visual();
  gb_timing()      = timing();
  gb_assets()      = assets(gb_visual());
  gb_mouse()       = mouse(gb_visual(), null());
  gb_audio()       = audio();
  gb_graph()       = graph(gb_visual());
  gb_controller()  = controller();

  gb_scene()       = scene(gb());
  scene_set_id(gb_scene(),"none");

  //test2

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

  // using a dictionary internally for assets instead of arrays
  // this will make it easier not to accidentally step over the same indices
  // when creating games for gb
  assets_set_image(a,"gb-0",          image("gb/media/images/gb-0.png",v));
  assets_set_image(a,"gb-1",          image("gb/media/images/gb-1.png",v));
  assets_set_image(a,"editor-bg",     image("gb/media/images/editor-bg.png",v));
  assets_set_image(a,"test",          image("gb/media/images/test.png",v));
  assets_set_image(a,"gb-color-test", image("gb/media/images/gb-color-test.png",v));

  // the assets object understands that we're looking for image_t* pointers.
  // internally the pointer is automatically cast in the dictionary from void* to image_t*
  assets_set_colormap(a,"gb-0",           colormap_from_image(assets_get_image(a,"gb-0")));
  assets_set_colormap(a,"gb-1",           colormap_from_image(assets_get_image(a,"gb-1")));
  assets_set_colormap(a,"editor-bg",      colormap_from_image(assets_get_image(a,"editor-bg")));
  assets_set_colormap(a,"test",           colormap_from_image(assets_get_image(a,"test")));
  assets_set_colormap(a,"gb-color-test",  colormap_from_image(assets_get_image(a,"gb-color-test")));

  assets_set_sound(a,"jake-1",sound("gb/media/sounds/jake-1.ogg"));
  assets_set_sound(a,"jake-2",sound("gb/media/sounds/jake-2.ogg"));
  assets_set_sound(a,"jake-3",sound("gb/media/sounds/jake-3.ogg"));
  assets_set_sound(a,"jake-4",sound("gb/media/sounds/jake-4.ogg"));

  /*
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
  */
  // load font 1
  font_t * f;

  f = font( assets_get_colormap(a,"gb-0") );
  font_set_pos         ( f, 0, 100  );
  font_set_tile_size   ( f, 5, 10   );
  font_set_tiles_size  ( f, 10, 10  );
  assets_set_font      ( a, "thin", f );

  graph_set_font( gb_graph(), f );
  
  f = font( assets_get_colormap(a,"gb-0") );
  font_set_pos         ( f, 0, 0   );
  font_set_tile_size   ( f, 10, 10 );
  font_set_tiles_size  ( f, 10, 10 );
  assets_set_font      ( a, "original", f );

  // set the mouse's icon
  mouse_colormap(gb_mouse())      = assets_get_colormap(gb_assets(),"gb-0");
  mouse_set_colormap_rect         (gb_mouse(),40,200,10,10);
  mouse_set_visible               (gb_mouse(),true());

  // debug
  debug_load();
}

void gb_start() {
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
  
  if (b[SDL_SCANCODE_P]) {
    gb_paused() = ! gb_paused();
  }
  
  
  if (b[SDL_SCANCODE_ESCAPE]) {
    log("quit.");
    exit(0);
  }

  /////////////
  // modules //
  /////////////

  mouse_update      (gb_mouse());
  controller_update (gb_controller());

  debug_update_pre();

  ///////////
  // debug //
  ///////////

  debug_update_post();

}

void gb_update_post() {

}

void gb_draw_pre() {
  
  // clear the graph colormap if the cls flag is enabled.
  if (gb_flag_cls()) {
    graph_cls(gb_graph());
  }

  // clear screen to black and set draw color back to white.
  visual_draw_pre(gb_visual());  
  // background drawing for debug
  if (gb_flag_debug()) debug_draw_pre();
}

void gb_draw_post() {
  // all of the module drawing goes in here
  graph_draw_mouse( gb_graph(), gb_mouse() );
  // do some debug drawing at the end of everything else
  if (gb_flag_debug()) debug_draw_post();
  // present the screen
  graph_present(gb_graph());
  visual_draw_post(gb_visual());

  timing_update_post(gb_timing());
}

void gb_quit() {

  debug_quit();

}