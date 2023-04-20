


type() {

  // base scene
  local( scene_t      * scene );

  // Playfield 
  local( playfield_t  * playfield );
  
  // UI
  local( brickerui_t  * brickerui );

  // background layer
  foreign( colormap_t   * colormap_bg );

  // ready sprite
  local( sprite_t     * sprite_ready );
  int rdyspr_dest_y;
  int rdyspr_curr_y;

} scene_game_t;

#define scene_game_scene(self)          (self->scene)
#define scene_game_playfield(self)      (self->playfield)
#define scene_game_sprite_ready(self)   (self->sprite_ready)
#define scene_game_brickerui(self)      (self->brickerui)
#define scene_game_colormap_bg(self)    (self->colormap_bg)
#define scene_game_rdyspr_curr_y(self)  (self->rdyspr_curr_y)
#define scene_game_rdyspr_dest_y(self)  (self->rdyspr_dest_y)

///////////////
// callbacks //
///////////////

/////////
// new //
/////////

void init_scene_game( scene_game_t * self ) {
  scene_game_scene(self)        = scene("game");
  scene_game_playfield(self)    = playfield();
  scene_game_colormap_bg(self)  = gb_get_colormap("bricker-bg");
  scene_game_brickerui(self)    = brickerui();
  scene_game_sprite_ready(self) = sprite(gb_get_colormap("bricker-0"),
    110,70,80,20);
  scene_game_rdyspr_dest_y(self) = 110000;
  scene_game_rdyspr_curr_y(self) = 300000;
}

scene_game_t * scene_game(void) {
  scene_game_t * self = alloc(scene_game_t);
  init_scene_game(self);
  return self;
}

void free_scene_game( scene_game_t * self ) {
  free_scene      (scene_game_scene(self));
  free_playfield  (scene_game_playfield(self));
  free_brickerui  (scene_game_brickerui(self));
  free            (self);
}

////////////
// events //
////////////

void scene_game_start( scene_game_t * self ) {
}

void scene_game_update( scene_game_t * self ) {
  int u = scene_game_rdyspr_dest_y(self)-scene_game_rdyspr_curr_y(self);
  u = u*10/100;
  scene_game_rdyspr_curr_y(self) += u;
  brickerui_update(scene_game_brickerui(self));
}

void scene_game_draw( scene_game_t * self ) {
  gb_draw_colormap( 0, 0, scene_game_colormap_bg(self) );
  brickerui_draw(scene_game_brickerui(self));
  gb_set_intensity(1000);  
  gb_draw_sprite( center(400,80), scene_game_rdyspr_curr_y(self)/1000, 
    scene_game_sprite_ready(self) );
  gb_reset_intensity();
}

void scene_game_quit( scene_game_t * self ) {
}