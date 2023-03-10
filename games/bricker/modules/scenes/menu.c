
////////////
// consts //
////////////

#define scene_menu_entmenu_selection_start()       0
#define scene_menu_entmenu_selection_customgame()  1
#define scene_menu_entmenu_selection_options()     2
#define scene_menu_entmenu_selection_exit()        3

//////////
// type //
//////////

type() {

  local( scene_t * scene );
  local( entity_menu_t * entmenu );

  foreign( sound_t * snd_menu1 );
  foreign( sound_t * snd_menu2 );

} scene_menu_t;

// getters
#define scene_menu_scene(self)         (self->scene)
#define scene_menu_entmenu(self)       (self->entmenu)
#define scene_menu_snd_menu1(self)     (self->snd_menu1)
#define scene_menu_snd_menu2(self)     (self->snd_menu2)

///////////////
// callbacks //
///////////////

void scene_menu_start( void * _ ) {
  scene_t * _self     = _;
  scene_menu_t * self = scene_get_spec(_self);
}

void scene_menu_update( void * _ ) {
  scene_t * _self     = _;
  scene_menu_t * self = scene_get_spec(_self);
  // handle the cursor moving
  if (gb_button(controller_button_down())==controller_button_pressed()) {
    entity_menu_cursor_down( scene_menu_entmenu(self) );
    gb_sound( scene_menu_snd_menu1(self), 0 );
  }
  else if (gb_button(controller_button_up())==controller_button_pressed()) {
    entity_menu_cursor_up( scene_menu_entmenu(self) );
    gb_sound( scene_menu_snd_menu1(self), 0 );
  }
  // handle selecting an action with spacebar
  if (gb_button(controller_button_select())==controller_button_pressed()) {
    entity_menu_select(scene_menu_entmenu(self));
    gb_sound( scene_menu_snd_menu2(self), 0 );

    if (entity_menu_cursor(scene_menu_entmenu(self))==scene_menu_entmenu_selection_exit()) {
      gb_exit();
    }
  }
}

void scene_menu_draw( void * _ ) {
  scene_t * _self     = _;
  scene_menu_t * self = scene_get_spec(_self);
  gb_set_intensity_max();
  entity_menu_draw( scene_menu_entmenu(self) );
  gb_reset_intensity();
}

void scene_menu_quit( void * _ ) {
  scene_t * _self     = _;
  scene_menu_t * self = scene_get_spec(_self);
}

/////////
// new //
/////////

void init_scene_menu( scene_menu_t * self ) {
  // make the base scene object
  scene_menu_scene(self) = scene("menu",self);
  // 4 entries
  scene_menu_entmenu(self) = entity_menu(4);

  entity_menu_colormap(scene_menu_entmenu(self)) = gb_get_colormap("bricker-0");
  tilemap_set_offset            (entity_menu_tilemap(scene_menu_entmenu(self)), 80, 70 );
  entity_menu_set_cursor_pos    (scene_menu_entmenu(self), 30, 30);
  entity_set_pos                (entity_menu_entity(scene_menu_entmenu(self)), 140, 20 );

  entity_menu_set_title( scene_menu_entmenu(self), "    --MAIN MENU--" );
  entity_menu_set_label( scene_menu_entmenu(self), 0, "Start!" );
  entity_menu_set_label( scene_menu_entmenu(self), 1, "Custom Game" );
  entity_menu_set_label( scene_menu_entmenu(self), 2, "Options" );
  entity_menu_set_label( scene_menu_entmenu(self), 3, "Exit" );

  entity_menu_decorate_tilemap (scene_menu_entmenu(self));

  scene_menu_snd_menu1(self) = gb_get_sound("menu-1");
  scene_menu_snd_menu2(self) = gb_get_sound("menu-2");

  scene_set_funcs( scene_menu_scene(self),
    scene_menu_start, scene_menu_update,
    scene_menu_draw, scene_menu_quit
  );
}

scene_menu_t * scene_menu() {
  scene_menu_t * self = alloc(scene_menu_t);
  init_scene_menu(self);
  return self;
}

void free_scene_menu( scene_menu_t * self ) {
  free_scene        (scene_menu_scene(self));
  free_entity_menu  (scene_menu_entmenu(self));
  free              (self);
}





