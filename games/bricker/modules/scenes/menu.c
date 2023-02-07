
type() {

  local( scene_t * scene );
  local( ent_menu_t * ent_menu );

} scene_menu_t;

#define scene_menu_scene(self)     (self->scene)
#define scene_menu_ent_menu(self)  (self->ent_menu)

/////////
// new //
/////////

void init_scene_menu( scene_menu_t * self ) {
  // make the base scene object
  scene_menu_scene(self) = scene(self,"menu");
  // 4 entries
  scene_menu_ent_menu(self) = ent_menu(4);

  ent_menu_colormap(scene_menu_ent_menu(self)) = gb_get_colormap("bricker-0");
  tilemap_set_offset        (ent_menu_tilemap(scene_menu_ent_menu(self)), 80, 70 );
  ent_menu_set_cursor_pos   (scene_menu_ent_menu(self), 30, 30);
  entity_set_pos            (ent_menu_entity(scene_menu_ent_menu(self)), 140, 20 );

  ent_menu_set_title( scene_menu_ent_menu(self), "    --MAIN MENU--" );
  ent_menu_set_label( scene_menu_ent_menu(self), 0, "Start!" );

  ent_menu_decorate_tilemap (scene_menu_ent_menu(self));
}

scene_menu_t * scene_menu() {
  scene_menu_t * self = alloc(scene_menu_t);
  init_scene_menu(self);
  return self;
}

void free_scene_menu( scene_menu_t * self ) {
  free_scene    (scene_menu_scene(self));
  free_ent_menu (scene_menu_ent_menu(self));
  free          (self);
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void scene_menu_start( scene_menu_t * self ) {
}

void scene_menu_update( scene_menu_t * self ) {
}

void scene_menu_draw( scene_menu_t * self ) {
  gb_set_intensity_max();
  ent_menu_draw( scene_menu_ent_menu(self) );
  gb_reset_intensity();
}

void scene_menu_quit( scene_menu_t * self ) {
}


