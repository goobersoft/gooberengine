
type() {

  // original scene object
  local( scene_t * scene );

  // playfield
  local( playfield_t * playfield );

} scene_attract_t;

#define scene_attract_scene(self)       (self->scene)
#define scene_attract_playfield(self)   (self->playfield)

/////////
// new //
/////////

void init_scene_attract( scene_attract_t * self ) {
  // set scene object id to "attract"
  scene_attract_scene(self)      = scene("attract",self);
  scene_attract_playfield(self)  = playfield();
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
    entity_t * e = dt;
    entity_brick_t * eb = entity_get_spec(e);

    uu = rnd(1,6);
    if (uu==1)      entity_brick_set_id(eb,"1");
    else if (uu==2) entity_brick_set_id(eb,"2");
    else if (uu==3) entity_brick_set_id(eb,"3");
    else if (uu==4) entity_brick_set_id(eb,"4");
    else if (uu==5) entity_brick_set_id(eb,"5");
    else if (uu==6) entity_brick_set_id(eb,"6");
  }
  
  entity_pball_t * a;
  a = entity_pball();
  point_set( entity_pball_velo(a), 2,1 );
  list_add_last( playfield_balls(pf), entity_pball_entity(a) );
  a = entity_pball();
  point_set( entity_pball_velo(a), -1,2 );
  list_add_last( playfield_balls(pf), entity_pball_entity(a) );
  a = entity_pball();
  point_set( entity_pball_velo(a), 1,3 );
  list_add_last( playfield_balls(pf), entity_pball_entity(a) );

}

void scene_attract_update( scene_attract_t * self ) {
  // update base scene object
  scene_update( scene_attract_scene(self) );
  playfield_update( scene_attract_playfield(self) );

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