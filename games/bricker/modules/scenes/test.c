
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

void scene_test_fn_start( void * _base ) {
  scene_test_t * self = scene_to_spec(_base);
}

void scene_test_fn_update( void * _base ) {
  scene_test_t * self = scene_to_spec(_base);
}

void scene_test_fn_draw( void * _base ) {
  scene_test_t * self = scene_to_spec(_base);
}

void scene_test_fn_quit( void * _base ) {
  scene_test_t * self = scene_to_spec(_base);
}

/////////
// new //
/////////

void init_scene_test( scene_test_t * self ) {
  scene_test_scene(self) = scene("test",self);
  scene_set_funcs( scene_menu_scene(self),
    scene_test_fn_start, scene_test_fn_update,
    scene_test_fn_draw, scene_test_fn_quit
  );
}

scene_test_t * scene_test() {
  scene_test_t * self = alloc(scene_test_t);
  init_scene_test(self);
  return self;
}

void free_scene_test( scene_test_t * self ) {
  free_scene( scene_test_scene(self) );
  free(self);
}