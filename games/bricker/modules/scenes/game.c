
type() {

  local( scene_t * scene );
  local( playfield_t * playfield );

} scene_game_t;

#define scene_game_scene(self)      (self->scene)
#define scene_game_playfield(self)  (self->playfield)

///////////////
// callbacks //
///////////////

void scene_game_start( void * _ ) {
  scene_t * _self = _;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_update( void * _ ) {
  scene_t * _self = _;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_draw( void * _ ) {
  scene_t * _self = _;
  scene_game_t * self = scene_get_spec(_self);
}

void scene_game_quit( void * _ ) {
  scene_t * _self = _;
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
    scene_game_start, scene_game_update,
    scene_game_draw, scene_game_quit
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

