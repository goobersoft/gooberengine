
type() {

  // original scene object
  local( scene_t * scene );

  // playfield
  local( playfield_t * playfield );

  // UI
  local( brickerui_t * brickerui );

  foreign( colormap_t * colormap );

  int mtimer;
  int mdir;
  int mbend;

} scene_attract_t;

#define scene_attract_scene(self)       (self->scene)
#define scene_attract_playfield(self)   (self->playfield)
#define scene_attract_brickerui(self)   (self->brickerui)
#define scene_attract_mtimer(self)      (self->mtimer)
#define scene_attract_mdir(self)        (self->mdir)
#define scene_attract_mbend(self)       (self->mbend)
#define scene_attract_colormap(self)    (self->colormap)

/////////
// new //
/////////

void init_scene_attract( scene_attract_t * self ) {
  // set scene object id to "attract"
  scene_attract_scene(self)      = scene("attract");
  scene_attract_playfield(self)  = playfield();
  scene_attract_brickerui(self)  = brickerui();
  scene_attract_mtimer(self)     = 0;
  scene_attract_mdir(self)       = 0;
  scene_attract_mbend(self)      = 0;
  scene_attract_colormap(self)   = gb_get_colormap("bricker-bg");

  leddisplay_activate( brickerui_leddisplay(scene_attract_brickerui(self)),
    true());
}

scene_attract_t * scene_attract(void) {
  scene_attract_t * self = alloc(scene_attract_t);
  init_scene_attract(self);
  // return the scene, not itself.
  // Since the scene object has this object as its source,
  // we can cast the internal source pointer later.
  return self;
}

void free_scene_attract( scene_attract_t * self ) {
  free_scene      (scene_attract_scene(self));
  free_playfield  (scene_attract_playfield(self));
  free_brickerui  (scene_attract_brickerui(self));
  free            (self);
}

////////////
// events //
////////////

void scene_attract_start( scene_attract_t * self ) {
  playfield_t * pf = scene_attract_playfield(self);
  
  int uu;
  foreach( playfield_bricks(pf), dt) {
    brick_t * eb = dt;

    uu = rnd(1,7);
    if (uu==1)      brick_set_id(eb,"1");
    else if (uu==2) brick_set_id(eb,"2");
    else if (uu==3) brick_set_id(eb,"3");
    else if (uu==4) brick_set_id(eb,"4");
    else if (uu==5) brick_set_id(eb,"5");
    else if (uu==6) brick_set_id(eb,"6");
  }
  
  pball_t * a;
  a = pball();
  point_set( pball_velo(a), 2000,1000 );
  list_add_last( playfield_balls(pf), a );
  a = pball();
  point_set( pball_velo(a), -1000,2000 );
  list_add_last( playfield_balls(pf), a );
  a = pball();
  point_set( pball_velo(a), 1000,3000 );
  list_add_last( playfield_balls(pf), a );

  brickerui_pressstart_active( scene_attract_brickerui(self) ) = true();
}

void scene_attract_update( scene_attract_t * self ) {
  // update the ui
  brickerui_update( scene_attract_brickerui(self) );
  // update base scene object
  playfield_update( scene_attract_playfield(self) );

  scene_attract_mtimer(self) += 1;
  if (scene_attract_mtimer(self) == 30) {
    scene_attract_mtimer(self) = 0;
    scene_attract_mdir(self)   = rnd(-1,2);
    scene_attract_mbend(self)  = rnd(-1,2);
    leddisplay_set_cells_random( 
      brickerui_leddisplay( scene_attract_brickerui(self) ) );

  }

  if (scene_attract_mdir(self) == -1) {
    paddle_move_left(playfield_paddle(scene_attract_playfield(self)));
  }
  else if (scene_attract_mdir(self) == 1) {
    paddle_move_right(playfield_paddle(scene_attract_playfield(self)));
  }

  if (scene_attract_mbend(self) == -1) {
    paddle_bend_down(playfield_paddle(scene_attract_playfield(self)));
  }
  else if (scene_attract_mbend(self) == 1) {
    paddle_bend_up(playfield_paddle(scene_attract_playfield(self)));
  }

  if (gb_button(controller_button_select()) == 2) {
    scene_finish( scene_attract_scene(self), "menu" );
  }
}

void scene_attract_draw( scene_attract_t * self ) {
  
  graph_set_intensity(gb_graph(),100);
  graph_draw_colormap(gb_graph(),0,0,scene_attract_colormap(self));
  graph_reset_intensity(gb_graph());

  playfield_draw( scene_attract_playfield(self) );
  brickerui_draw( scene_attract_brickerui(self) );
}

void scene_attract_quit( scene_attract_t * self ) {
}
