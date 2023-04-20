
#define bricker_scenemgr_curr_scene_attract() "attract"
#define bricker_scenemgr_curr_scene_game()    "game"
#define bricker_scenemgr_curr_scene_menu()    "menu"
#define bricker_scenemgr_curr_scene_test()    "test"


type() {

  scene_t * scene;

  scene_attract_t * scene_attract;
  scene_game_t    * scene_game;
  scene_menu_t    * scene_menu;
  scene_test_t    * scene_test;

} bricker_scenemgr_t;

#define bricker_scenemgr_scene(self)          (self->scene)
#define bricker_scenemgr_scene_attract(self)  (self->scene_attract)
#define bricker_scenemgr_scene_game(self)     (self->scene_game)
#define bricker_scenemgr_scene_menu(self)     (self->scene_menu)
#define bricker_scenemgr_scene_test(self)     (self->scene_test)

#define bricker_scenemgr_curr_scene(self)     scene_id(bricker_scenemgr_scene(self))

/////////
// new //
/////////

void init_bricker_scenemgr( bricker_scenemgr_t * self ) {
  bricker_scenemgr_scene(self)         = scene("none");
  bricker_scenemgr_scene_attract(self) = null();
  bricker_scenemgr_scene_game(self)    = null();
  bricker_scenemgr_scene_menu(self)    = null();
  bricker_scenemgr_scene_test(self)    = null();
}

bricker_scenemgr_t * bricker_scenemgr(void) {
  bricker_scenemgr_t * self = alloc(bricker_scenemgr_t);
  init_bricker_scenemgr(self);
  return self;
}

///////////////
// functions //
///////////////

void bricker_scenemgr_clear_scene( bricker_scenemgr_t * self ) {
  
  char * s = bricker_scenemgr_curr_scene(self);

  if (s == bricker_scenemgr_curr_scene_attract()) {
    scene_attract_quit( bricker_scenemgr_scene_attract(self) );
    free_scene_attract( bricker_scenemgr_scene_attract(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_game()) {
    scene_game_quit( bricker_scenemgr_scene_game(self) );
    free_scene_game( bricker_scenemgr_scene_game(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_menu()) {
    scene_menu_quit( bricker_scenemgr_scene_menu(self) );
    free_scene_menu( bricker_scenemgr_scene_menu(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_test()) {
    scene_test_quit( bricker_scenemgr_scene_test(self) );
    free_scene_test( bricker_scenemgr_scene_test(self) );
  }
  else {
    free_scene( bricker_scenemgr_scene(self) );
  }
  bricker_scenemgr_scene(self) = null();
}

void bricker_scenemgr_set_scene( bricker_scenemgr_t * self, char * s ) {
  bricker_scenemgr_clear_scene(self);

  if (s == bricker_scenemgr_curr_scene_attract()) {
    bricker_scenemgr_scene_attract(self) = scene_attract();
    bricker_scenemgr_scene(self) = 
      scene_attract_scene( bricker_scenemgr_scene_attract(self) );
    scene_attract_start(bricker_scenemgr_scene_attract(self));
  }
  else if (s == bricker_scenemgr_curr_scene_game()) {
    bricker_scenemgr_scene_game(self) = scene_game();
    bricker_scenemgr_scene(self) = 
      scene_game_scene( bricker_scenemgr_scene_game(self) );
    scene_game_start(bricker_scenemgr_scene_game(self));
  }
  else if (s == bricker_scenemgr_curr_scene_menu()) {
    bricker_scenemgr_scene_menu(self) = scene_menu();
    bricker_scenemgr_scene(self) = 
      scene_menu_scene( bricker_scenemgr_scene_menu(self) );
    scene_menu_start(bricker_scenemgr_scene_menu(self));
  }
  else if (s == bricker_scenemgr_curr_scene_test()) { 
    bricker_scenemgr_scene_test(self) = scene_test();
    bricker_scenemgr_scene(self) = 
      scene_test_scene( bricker_scenemgr_scene_test(self) );
    scene_test_start(bricker_scenemgr_scene_test(self));
  }
  else {
    bricker_scenemgr_scene(self) = scene("none");
  }
}

////////////
// events //
////////////

void bricker_scenemgr_update( bricker_scenemgr_t * self ) {
  scene_update( bricker_scenemgr_scene(self) );

  if (scene_finished(bricker_scenemgr_scene(self))) {
    bricker_scenemgr_set_scene(self, scene_next(bricker_scenemgr_scene(self)) );
  }

  char * s = scene_id( bricker_scenemgr_scene(self) );
  if (s == bricker_scenemgr_curr_scene_attract()) {
    scene_attract_update( bricker_scenemgr_scene_attract(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_game()) {
    scene_game_update( bricker_scenemgr_scene_game(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_menu()) { 
    scene_menu_update( bricker_scenemgr_scene_menu(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_test()) {
    scene_test_update( bricker_scenemgr_scene_test(self) );
  }
}

void bricker_scenemgr_draw( bricker_scenemgr_t * self ) {
  char * s = scene_id( bricker_scenemgr_scene(self) );
  if (s == bricker_scenemgr_curr_scene_attract()) {
    scene_attract_draw( bricker_scenemgr_scene_attract(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_game()) {
    scene_game_draw( bricker_scenemgr_scene_game(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_menu()) { 
    scene_menu_draw( bricker_scenemgr_scene_menu(self) );
  }
  else if (s == bricker_scenemgr_curr_scene_test()) {
    scene_test_draw( bricker_scenemgr_scene_test(self) );
  }
}