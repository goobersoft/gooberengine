
type() {

  scene_t   * scene;
  sprite_t  * sprite_logo;
  sprite_t  * sprite_bg;

} ma_scene_logo_t;

#define ma_scene_logo_scene(self)       (self->scene)
#define ma_scene_logo_sprite_logo(self) (self->sprite_logo)
#define ma_scene_logo_sprite_bg(self)   (self->sprite_bg)

#define ma_scene_logo_finished(self)    scene_flag_finished(ma_scene_logo_scene(self))

/////////
// new //
/////////

void ma_scene_logo_init( ma_scene_logo_t * self ) {
  // alloc the base scene type and send itself as the
  // parent.
  ma_scene_logo_scene(self) = scene(self);
  // get the logo colormap and make the sprite
  ma_scene_logo_sprite_logo(self)   = sprite(gb_get_colormap("gb-0"),20,220,80,20);
  ma_scene_logo_sprite_bg(self)     = sprite(gb_get_colormap("gb-0"),100,100,20,20);
  // uniquely identify this scene id as "ma_scene_logo"
  scene_set_id(ma_scene_logo_scene(self),"logo");
}

ma_scene_logo_t * ma_scene_logo() {
  // alloc the new scene
  ma_scene_logo_t * r = alloc(ma_scene_logo_t);
  // initialize this scene
  ma_scene_logo_init(r);
  // return this scene
  return r;
}

void free_ma_scene_logo( ma_scene_logo_t * self ) {
  free_sprite(ma_scene_logo_sprite_logo(self));
  free_sprite(ma_scene_logo_sprite_bg(self));
  free_scene(ma_scene_logo_scene(self));
  free(self);
}

////////////
// events //
////////////

void ma_scene_logo_start( ma_scene_logo_t * self ) {
  mouse_set_visible( gb_mouse(), false() );
  graph_set_intensity( gb_graph(), 100 );
}

void ma_scene_logo_update( ma_scene_logo_t * self ) {
  // update the base scene
  if (ma_scene_logo_finished(self)==false()) {
    scene_update(ma_scene_logo_scene(self));

    if (scene_ticks(ma_scene_logo_scene(self))>320) {
      scene_finish(ma_scene_logo_scene(self));
    }
  }
}

void ma_scene_logo_draw( ma_scene_logo_t * self ) {
  if (ma_scene_logo_finished(self)==false()) {

    if (scene_ticks(ma_scene_logo_scene(self))==1) {
      graph_set_layer_base(gb_graph());
      int u = graph_set_intensity_max(gb_graph());
      graph_draw_sprite_tiled(gb_graph(),0,0,ma_scene_logo_sprite_bg(self),20,12);
      graph_set_intensity(gb_graph(),u);
    }
    else if (inrange(scene_ticks(ma_scene_logo_scene(self)),2,200)) {
      graph_draw_sprite(gb_graph(),160,110,ma_scene_logo_sprite_logo(self));
    }
    else if (inrange(scene_ticks(ma_scene_logo_scene(self)),201,320)) {
      graph_draw_cls( gb_graph() );
    }
  }
}

void ma_scene_logo_quit( ma_scene_logo_t * self ) {
  free_ma_scene_logo(self);
}