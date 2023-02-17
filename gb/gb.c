
#define gb_version_major() 2023
#define gb_version_minor() 2
#define gb_version_patch() 07

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

#include "modules/core/globals.c"             // includes many macro definitions

#include "modules/core/functions.c"           // core functions
#include "modules/core/functions_sine.c"      // sine and cosine function
#include "modules/core/functions_sqroot.c"    // sqroot function
#include "modules/core/functions_base64.c"    // sqroot function

#include "modules/core/tag.c"                 // descriptive tag
#include "modules/core/types.c"               // complex numerical types
#include "modules/core/bitfield.c"              // bitfield
#include "modules/core/bintree.c"             // binary tree
/*
#include "modules/rng.c"                      // implementation of mersenne twister in c
*/
#include "modules/core/string.c"              // fixed-length character arrays
#include "modules/core/list.c"                // linked list data type
#include "modules/core/dict.c"                // dictionary
#include "modules/core/pstack.c"              // LIFO-style priority stack object
#include "modules/core/point.c"               // 2d integer point
#include "modules/core/point3.c"              // 2d integer point
#include "modules/core/rect.c"                // 
#include "modules/core/file.c"                // file type
#include "modules/core/number.c"              // integer number with min and max bounds
#include "modules/core/alarm.c"               // timers
#include "modules/core/entity.c"              // AABB collisions
#include "modules/core/gbs.c"                 // GooberScript
#include "modules/core/gbml.c"                // markup
#include "modules/core/scene.c"               // scene base object
#include "modules/core/settings.c"            // settings container

#include "modules/mechanical/menu.c"          // works for UIs

#include "modules/graphical/color.c"          // 6-bit color with transparency bit
#include "modules/graphical/board.c"          // drawing durfaces
#include "modules/graphical/visual.c"         // window and renderer interface
#include "modules/graphical/image.c"          // uses SDL2_image
#include "modules/graphical/palette.c"        // storage of a list of 6-bit colors.
#include "modules/graphical/colormap.c"       // a 2d array of colors
#include "modules/graphical/font.c"           // bitmap fonts which utilize image colormaps
#include "modules/graphical/sprite.c"         // instanced object with a reference to a colormap
#include "modules/graphical/tilemap.c"        // instanceable grids of variable colormap sections

#include "modules/input/input.c"              // user input
#include "modules/input/mouse.c"              // mouse interface
#include "modules/input/controller.c"         // gamepads

#include "modules/audio/sound.c"              // sound interface

#include "modules/components/audio.c"         // audio mixer
#include "modules/components/network.c"       // networking (TCP/UDP)
#include "modules/components/assets.c"        // asset container object (sound/image/font)
#include "modules/components/timing.c"        // fps/timing interface
#include "modules/components/graph.c"         // drawing interface



/////////////
// gb type //
/////////////

typedef struct {

  bool_t         f_running;
  bool_t         f_paused;
  bool_t         f_cls;
  bool_t         f_debug;
  visual_t     * visual;
  timing_t     * timing;
  graph_t      * graph;
  assets_t     * assets;
  //mouse_t      * mouse;
  audio_t      * audio;
  controller_t * controller;
  input_t      * input;
  settings_t   * settings;

  scene_t      * scene;

  //lua_State    * lua;
  
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
#define gb_running()    (gb->f_running)
#define gb_f_cls()      (gb->f_cls)
#define gb_f_debug()    (gb->f_debug)
#define gb_paused()     (gb->f_paused)
#define gb_visual()     (gb->visual)
#define gb_timing()     (gb->timing)
#define gb_graph()      (gb->graph)
#define gb_audio()      (gb->audio)
#define gb_controller() (gb->controller)
#define gb_input()      (gb->input)
#define gb_settings()   (gb->settings)
#define gb_scene()      (gb->scene)
#define gb_lua()        (gb->lua)

//////////////////////
// helper functions //
//////////////////////

// input
#define gb_get_key_state(a)   input_get_key_state   (gb_input(),a)
#define gb_get_key_pressed(a) input_get_key_pressed (gb_input(),a)
#define gb_get_key_held(a)    input_get_key_held    (gb_input(),a)

// assets
#define gb_get_image(a)       assets_get_image(gb_assets(),a)
#define gb_get_colormap(a)    assets_get_colormap(gb_assets(),a)
#define gb_get_sound(a)       assets_get_sound(gb_assets(),a)
#define gb_get_font(a)        assets_get_font(gb_assets(),a)

#define gb_set_image(a,p)     assets_set_image     (gb_assets(),a,p)
#define gb_set_colormap(a,p)  assets_set_colormap  (gb_assets(),a,p)
#define gb_set_sound(a,p)     assets_set_sound     (gb_assets(),a,p)   
#define gb_set_font(a,p)      assets_set_font      (gb_assets(),a,p)

// controller
#define gb_button(a)          controller_get_button(gb_controller(),a)

// graph
#define gb_cls()                      graph_cls(gb_graph())
#define gb_set_intensity(x)           graph_set_intensity(gb_graph(),x)
#define gb_set_intensity_max()        graph_set_intensity_max(gb_graph())
#define gb_reset_intensity()          graph_reset_intensity(gb_graph())
#define gb_set_color(r,g,b)           graph_set_color(gb_graph(),make_color(r,g,b))
#define gb_draw_dot(x,y)              graph_draw_dot(gb_graph(),x,y)
#define gb_draw_rect(x,y,w,h)         graph_draw_rect(gb_graph(),x,y,w,h)
#define gb_draw_rect_line(x,y,w,h)    graph_draw_rect_line(gb_graph(),x,y,w,h)
#define gb_draw_circle(x,y,r)         graph_draw_circle(gb_graph(),x,y,r)
#define gb_draw_circle_line(x,y,r)    graph_draw_rect(gb_graph(),x,y,r)
#define gb_draw_colormap(x,y,c)       graph_draw_colormap(gb_graph(),x,y,c)
#define gb_draw_colormap_sub(x,y,c,cx,cy,cw,ch) \
  graph_draw_colormap_sub(gb_graph(),x,y,c,cx,cy,cw,ch)
#define gb_draw_text(x,y,t)           graph_draw_text(gb_graph(),x,y,t)
#define gb_draw_tilemap(x,y,t)        graph_draw_tilemap(gb_graph(),x,y,t)

// audio
#define gb_sound(s,c)                 audio_play(gb_audio(),s,c)

///////////////
// functions //
///////////////

void gb_set_cls( bool_t b ) {
  gb_f_cls() = bool(b);
}

void gb_set_scene( scene_t * s ) {
  gb_scene() = s;
}

void gb_exit() {
  gb_running() = false();
}

//////////////////////////
// special debug module //
//////////////////////////
// the debug module has visibility of the gb globals
// this will help with debugging immensely.

///////////////////////
// top layer objects //
///////////////////////
// one thing to note is that top layer objects are aware of the graph module.
// this means that these objects will have their own drawing functions which reference
// the graph module instead.

#include "objects/graphical/sprayer.c"    // sprays the screen with colored pixels
#include "objects/graphical/camera.c"     // determines how a scene should render
#include "objects/graphical/menu.c"

#include "objects/toys/jake.c"            // slithers across the screen slowly
#include "objects/toys/ball.c"            // bounces based on arrow keys
#include "objects/toys/fountain.c"        // sprays sprites from a center

#include "modules/debug/debugpanel.c"
#include "modules/debug/debug.c"

////////////
// events //
////////////

void gb_init() {
  log("gooberengine - VER: %d.%d.%d",gb_version_major(),gb_version_minor(),gb_version_patch());

  // the master object does not have an allocation function.
  // instead it is done here in init()
  gb()              = alloc(gb_t);
  gb_running()      = true();
  gb_paused()       = false();
  gb_f_cls()        = false();
  gb_f_debug()      = true();
  gb_visual()       = visual();
  gb_timing()       = timing();
  gb_assets()       = assets(gb_visual());
  gb_audio()        = audio();
  gb_graph()        = graph(gb_visual());
  gb_input()        = input();
  gb_settings()     = settings();
  gb_controller()   = controller(gb_input());
  
  // current scene is null.
  gb_scene()       = null();
  /*
  gb_lua() = lua_open();
  luaL_openlibs(gb_lua());
  */
  debug_init();
  
}

//////////
// LOAD //
//////////

void gb_load() {
  // load the default assets
  // load images
  assets_t * a = gb_assets();
  visual_t * v = gb_visual();

  // using a dictionary internally for assets instead of arrays
  // this will make it easier not to accidentally step over the same indices
  // when creating games for gb
  assets_set_image(a,"gb-0",          image("gb/media/images/gb-0.png"));
  assets_set_image(a,"gb-1",          image("gb/media/images/gb-1.png"));
  assets_set_image(a,"editor-bg",     image("gb/media/images/editor-bg.png"));
  assets_set_image(a,"test",          image("gb/media/images/test.png"));
  assets_set_image(a,"gb-color-test", image("gb/media/images/gb-color-test.png"));

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

  // debug
  debug_load();
}

void gb_start() {
  debug_start();
}

////////////
// UPDATE //
////////////

void gb_update() {

  //-- Timing --//

  timing_update_pre(gb_timing());

  //-- SDL Events --//

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      log("Quit" nl());
      gb_running() = false();
    }
  }

  /*
  if (b[SDL_SCANCODE_P]) {
    gb_paused() = ! gb_paused();
  }
  */
  
  // panic button
  const Uint8 * b = SDL_GetKeyboardState( null() );
  if (b[SDL_SCANCODE_ESCAPE]) {
    log("quit.");
    exit(0);
  }

  //-- Modules --//

  input_update      (gb_input());
  controller_update (gb_controller());

  //-- debugging --//

  debug_update_pre();

  debug_update_post();

}

void gb_update_post() {

}

//////////
// DRAW //
//////////

// pre-draw event
void gb_draw_pre() {
  
  // clear the graph colormap if the cls flag is enabled.
  if (gb_f_cls()) {
    graph_cls(gb_graph());
  }

  // clear screen to black and set draw color back to white.
  visual_draw_pre(gb_visual());  
  // background drawing for debug
  if (gb_f_debug()) debug_draw_pre();
}

// post-draw event
void gb_draw_post() {
  // all of the module drawing goes in here
  //graph_draw_mouse( gb_graph(), gb_mouse_x(), gb_mouse_y(), gb_input() );
  // do some debug drawing at the end of everything else
  if (gb_f_debug()) debug_draw_post();
  // present the screen
  graph_present(gb_graph());
  visual_draw_post(gb_visual());

  timing_update_post(gb_timing());
}

//////////
// QUIT //
//////////

void gb_quit() {
  debug_quit();
}