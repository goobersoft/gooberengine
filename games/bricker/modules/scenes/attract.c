
type() {

  // original scene object
  local( scene_t * scene );

  // playfield
  local( playfield_t * playfield );

  int mtimer;
  int mdir;
  int mbend;

} scene_attract_t;

#define scene_attract_scene(self)       (self->scene)
#define scene_attract_playfield(self)   (self->playfield)
#define scene_attract_mtimer(self)      (self->mtimer)
#define scene_attract_mdir(self)        (self->mdir)
#define scene_attract_mbend(self)       (self->mbend)

/////////
// new //
/////////

void init_scene_attract( scene_attract_t * self ) {
  // set scene object id to "attract"
  scene_attract_scene(self)      = scene("attract",self);
  scene_attract_playfield(self)  = playfield();
  scene_attract_mtimer(self)     = 0;
  scene_attract_mdir(self)       = 0;
  scene_attract_mbend(self)      = 0;
}

scene_attract_t * scene_attract() {
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
  free            (self);
}

////////////
// events //
////////////


void scene_attract_start( scene_attract_t * self ) {
  
  playfield_t * pf = scene_attract_playfield(self);
  

  int uu;
  foreach( playfield_bricks(pf), dt) {
    entity_t      * e  = dt;
    actor_t       * ea = entity_get_spec(e);
    actor_brick_t * eb = actor_get_spec(ea);

    uu = rnd(1,6);
    if (uu==1)      actor_brick_set_id(eb,"1");
    else if (uu==2) actor_brick_set_id(eb,"2");
    else if (uu==3) actor_brick_set_id(eb,"3");
    else if (uu==4) actor_brick_set_id(eb,"4");
    else if (uu==5) actor_brick_set_id(eb,"5");
    else if (uu==6) actor_brick_set_id(eb,"6");
  }
  
  actor_pball_t * a;
  a = actor_pball();
  point_set( actor_pball_velo(a), 2,1 );
  list_add_last( playfield_balls(pf), actor_pball_entity(a) );
  a = actor_pball();
  point_set( actor_pball_velo(a), -1,2 );
  list_add_last( playfield_balls(pf), actor_pball_entity(a) );
  a = actor_pball();
  point_set( actor_pball_velo(a), 1,3 );
  list_add_last( playfield_balls(pf), actor_pball_entity(a) );

  
}

void scene_attract_update( scene_attract_t * self ) {
  // update base scene object
  scene_update( scene_attract_scene(self) );
  playfield_update( scene_attract_playfield(self) );

  scene_attract_mtimer(self) += 1;
  if (scene_attract_mtimer(self) == 30) {
    scene_attract_mtimer(self) = 0;
    scene_attract_mdir(self)   = rnd(-1,2);
    scene_attract_mbend(self)  = rnd(-1,2);
  }

  if (scene_attract_mdir(self) == -1) {
    actor_paddle_move_left(playfield_paddle(scene_attract_playfield(self)));
  }
  else if (scene_attract_mdir(self) == 1) {
    actor_paddle_move_right(playfield_paddle(scene_attract_playfield(self)));
  }

  if (scene_attract_mbend(self) == -1) {
    actor_paddle_bend_down(playfield_paddle(scene_attract_playfield(self)));
  }
  else if (scene_attract_mbend(self) == 1) {
    actor_paddle_bend_up(playfield_paddle(scene_attract_playfield(self)));
  }

  if (gb_button(controller_button_select()) == 2) {
    scene_finished( scene_attract_scene(self) ) = true();
    scene_next( scene_attract_scene(self) ) = "game";
  }
}

void scene_attract_draw( scene_attract_t * self ) {
  playfield_draw( scene_attract_playfield(self) );
}

void scene_attract_quit( scene_attract_t * self ) {

}