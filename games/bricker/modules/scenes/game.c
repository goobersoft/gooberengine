
type() {

  local( scene_t * scene );
  local( playfield_t * playfield );

} scene_game_t;

#define scene_game_scene(self)      (self->scene)
#define scene_game_playfield(self)  (self->playfield)

///////////////
// callbacks //
///////////////

void scene_game_fn_start( void * _base ) {
  scene_t * _self = _base;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_fn_update( void * _base ) {
  scene_t * _self = _base;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_fn_draw( void * _base ) {
  scene_t * _self = _base;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_fn_quit( void * _base ) {
  scene_t * _self = _base;
  scene_game_t * self = scene_get_spec(_self);

  free_scene      (scene_game_scene(self));
  free_playfield  (scene_game_playfield(self));
  free            (self);
}

/////////
// new //
/////////

void init_scene_game( scene_game_t * self ) {
  scene_game_scene(self)      = scene("game",self);
  scene_game_playfield(self)  = playfield();
  scene_set_funcs( scene_game_scene(self),
    scene_game_fn_start, scene_game_fn_update,
    scene_game_fn_draw, scene_game_fn_quit
  );
}

scene_game_t * scene_game() {
  scene_game_t * self = alloc(scene_game_t);
  init_scene_game(self);
  return self;
}

void free_scene_game( scene_game_t * self ) {
  free_scene      (scene_game_scene(self));
  free_playfield  (scene_game_playfield(self));
  free            (self);
}

