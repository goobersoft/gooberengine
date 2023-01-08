
type() {

  local( scene_t * scene );
  local( playfield_t * playfield );

} scene_game_t;

#define scene_game_scene(self)      (self->scene)
#define scene_game_playfield(self)  (self->playfield)

/////////
// new //
/////////

void init_scene_game( scene_game_t * self ) {
  scene_game_scene(self)      = scene(self,"game");
  scene_game_playfield(self)  = playfield();
}

scene_t * scene_game() {
  scene_game_t * self = alloc(scene_game_t);
  init_scene_game(self);
  return scene_game_scene(self);
}

void free_scene_game( scene_game_t * self ) {
  free_playfield  (scene_game_playfield(self));
  free_scene      (scene_game_scene(self));
  free            (self);
}