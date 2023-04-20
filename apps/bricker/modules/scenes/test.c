
//////////
// type //
//////////

type() {

  scene_t * scene;


} scene_test_t;

#define scene_test_scene(self) (self->scene)

////////////
// events //
////////////

/////////
// new //
/////////

void init_scene_test( scene_test_t * self ) {
  scene_test_scene(self) = scene("test");
}

scene_test_t * scene_test(void) {
  scene_test_t * self = alloc(scene_test_t);
  init_scene_test(self);
  return self;
}

void free_scene_test( scene_test_t * self ) {
  free_scene( scene_test_scene(self) );
  free(self);
}

////////////
// events //
////////////

void scene_test_start( scene_test_t * self ) {
}

void scene_test_update( scene_test_t * self ) {
}

void scene_test_draw( scene_test_t * self ) {
}

void scene_test_quit( scene_test_t * self ) {
}