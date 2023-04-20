
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

  int curr_menu;
  foreign( entmenu_t * entmenu );
  local( entmenu_t * entmenu_main );
  local( entmenu_t * entmenu_options );
  local( brickerui_t * brickerui );

  foreign( sound_t * snd_menu1 );
  foreign( sound_t * snd_menu2 );

  foreign( colormap_t * colormap );

} scene_menu_t;

// getters
#define scene_menu_curr_menu(self)       (self->curr_menu)
#define scene_menu_scene(self)           (self->scene)
#define scene_menu_entmenu(self)         (self->entmenu)
#define scene_menu_entmenu_main(self)    (self->entmenu_main)
#define scene_menu_entmenu_options(self) (self->entmenu_options)
#define scene_menu_brickerui(self)       (self->brickerui)
#define scene_menu_snd_menu1(self)       (self->snd_menu1)
#define scene_menu_snd_menu2(self)       (self->snd_menu2)
#define scene_menu_colormap(self)        (self->colormap)

///////////////
// callbacks //
///////////////

/////////
// new //
/////////

void init_scene_menu( scene_menu_t * self ) {
  // make the base scene object
  scene_menu_scene(self) = scene("menu");

  entmenu_t * e;

  //-- main menu --//

  e = entmenu(4);
  entmenu_colormap(e)      = gb_get_colormap("bricker-0");
  tilemap_set_offset       ( entmenu_tilemap(e), 80, 70 );
  entmenu_set_cursor_pos   ( e, 30, 30);
  point_set                ( entmenu_pos(e), 140, 20 );
  entmenu_set_title        ( e, "    --MAIN MENU--" );
  entmenu_set_label        ( e, 0, "Start!" );
  entmenu_set_label        ( e, 1, "Custom Game" );
  entmenu_set_label        ( e, 2, "Options" );
  entmenu_set_label        ( e, 3, "Exit" );
  entmenu_decorate_tilemap ( e );
  scene_menu_entmenu_main(self) = e;

  //-- options menu --//
  
  e = entmenu(4);
  entmenu_colormap(e)      = gb_get_colormap("bricker-0");
  tilemap_set_offset       ( entmenu_tilemap(e), 80, 70 );
  entmenu_set_cursor_pos   ( e, 30, 30);
  point_set                ( entmenu_pos(e), 140, 20 );
  entmenu_set_title        ( e, "    -- OPTIONS --" );
  entmenu_set_label        ( e, 0, "Sound: On" );
  entmenu_set_label        ( e, 1, "Music: On" );
  entmenu_set_label        ( e, 2, "Scale: 3x" );
  entmenu_set_label        ( e, 3, "<- Back" );
  entmenu_decorate_tilemap ( e );
  scene_menu_entmenu_options(self) = e;

  // make the ui
  scene_menu_brickerui(self) = brickerui();

  // sounds
  scene_menu_snd_menu1(self) = gb_get_sound("menu-1");
  scene_menu_snd_menu2(self) = gb_get_sound("menu-2");

  // colormap
  scene_menu_colormap(self)  = gb_get_colormap("bricker-bg");

  // set initial menu
  scene_menu_curr_menu(self) = 1;
  scene_menu_entmenu(self) = scene_menu_entmenu_main(self);
}

scene_menu_t * scene_menu(void) {
  scene_menu_t * self = alloc(scene_menu_t);
  init_scene_menu(self);
  return self;
}

void free_scene_menu( scene_menu_t * self ) {
  free_scene        (scene_menu_scene(self));
  free_entmenu      (scene_menu_entmenu_main(self));
  free_entmenu      (scene_menu_entmenu_options(self));
  free_brickerui    (scene_menu_brickerui(self));
  free              (self);
}

////////////
// events //
////////////

void scene_menu_start( scene_menu_t * self ) {
}

void scene_menu_update( scene_menu_t * self ) {
  // handle the cursor moving
  if (gb_button(controller_button_down())==controller_button_pressed()) {
    entmenu_cursor_down( scene_menu_entmenu(self) );
    gb_sound( scene_menu_snd_menu1(self), 0 );
  }
  else if (gb_button(controller_button_up())==controller_button_pressed()) {
    entmenu_cursor_up( scene_menu_entmenu(self) );
    gb_sound( scene_menu_snd_menu1(self), 0 );
  }
  // handle selecting an action with spacebar
  if (gb_button(controller_button_select())==controller_button_pressed()) {
    entmenu_select(scene_menu_entmenu(self));
    gb_sound( scene_menu_snd_menu2(self), 0 );

    if (scene_menu_curr_menu(self)==1) {
      if (entmenu_selection(scene_menu_entmenu(self))==scene_menu_entmenu_selection_start()) {
        scene_finish(scene_menu_scene(self),"game");
      }
      else if (entmenu_selection(scene_menu_entmenu(self))==scene_menu_entmenu_selection_exit()) {
        gb_exit();
      }
    }
  }
  // update the UI
  brickerui_update(scene_menu_brickerui(self));
}

void scene_menu_draw( scene_menu_t * self ) {
  gb_draw_colormap( 0,0, scene_menu_colormap(self) );
  brickerui_draw(scene_menu_brickerui(self));
  gb_set_intensity_max();
  entmenu_draw( scene_menu_entmenu(self) );
  gb_reset_intensity();
}

void scene_menu_quit( scene_menu_t * self ) {
}



