
type() {

  local( scene_t * scene );
  local( entmenu_t * entmenu );

  foreign( sound_t * snd_menu1 );
  foreign( sound_t * snd_menu2 );

} scenemenu_t;

#define scenemenu_scene(self)     (self->scene)
#define scenemenu_entmenu(self)   (self->entmenu)
#define scenemenu_snd_menu1(self) (self->snd_menu1)
#define scenemenu_snd_menu2(self) (self->snd_menu2)

/////////
// new //
/////////

void init_scenemenu( scenemenu_t * self ) {
  // make the base scene object
  scenemenu_scene(self) = scene(self,"menu");
  // 4 entries
  scenemenu_entmenu(self) = entmenu(4);

  entmenu_colormap(scenemenu_entmenu(self)) = gb_get_colormap("bricker-0");
  tilemap_set_offset        (entmenu_tilemap(scenemenu_entmenu(self)), 80, 70 );
  entmenu_set_cursor_pos    (scenemenu_entmenu(self), 30, 30);
  entity_set_pos            (entmenu_entity(scenemenu_entmenu(self)), 140, 20 );

  entmenu_set_title( scenemenu_entmenu(self), "    --MAIN MENU--" );
  entmenu_set_label( scenemenu_entmenu(self), 0, "Start!" );
  entmenu_set_label( scenemenu_entmenu(self), 1, "Custom Game" );
  entmenu_set_label( scenemenu_entmenu(self), 2, "Options" );
  entmenu_set_label( scenemenu_entmenu(self), 3, "Exit" );

  entmenu_decorate_tilemap (scenemenu_entmenu(self));

  scenemenu_snd_menu1(self) = gb_get_sound("menu-1");
  scenemenu_snd_menu2(self) = gb_get_sound("menu-2");
}

scenemenu_t * scenemenu() {
  scenemenu_t * self = alloc(scenemenu_t);
  init_scenemenu(self);
  return self;
}

void free_scenemenu( scenemenu_t * self ) {
  free_scene    (scenemenu_scene(self));
  free_entmenu  (scenemenu_entmenu(self));
  free          (self);
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void scenemenu_start( scenemenu_t * self ) {
}

void scenemenu_update( scenemenu_t * self ) {
  // handle the cursor moving
  if (gb_button(controller_button_down())==controller_button_pressed()) {
    entmenu_cursor_down( scenemenu_entmenu(self) );
    gb_sound( scenemenu_snd_menu1(self), 0 );
  }
  else if (gb_button(controller_button_up())==controller_button_pressed()) {
    entmenu_cursor_up( scenemenu_entmenu(self) );
    gb_sound( scenemenu_snd_menu1(self), 0 );
  }
  // handle selecting an action with spacebar
  if (gb_button(controller_button_select())==controller_button_pressed()) {
    entmenu_select(scenemenu_entmenu(self));
    gb_sound( scenemenu_snd_menu2(self), 0 );
  }
}

void scenemenu_draw( scenemenu_t * self ) {
  gb_set_intensity_max();
  entmenu_draw( scenemenu_entmenu(self) );
  gb_reset_intensity();
}

void scenemenu_quit( scenemenu_t * self ) {
}


