
type() {

  // original scene object
  local( scene_t * scene );

  // playfield
  local( playfield_t * playfield );

} sceneattract_t;

#define sceneattract_scene(self)       (self->scene)
#define sceneattract_playfield(self)   (self->playfield)

/////////
// new //
/////////

void init_sceneattract( sceneattract_t * self ) {
  // set scene object id to "attract"
  sceneattract_scene(self)      = scene(self,"attract");
  sceneattract_playfield(self)  = playfield();
}

sceneattract_t * sceneattract() {
  sceneattract_t * self = alloc(sceneattract_t);
  init_sceneattract(self);
  // return the scene, not itself.
  // Since the scene object has this object as its source,
  // we can cast the internal source pointer later.
  return self;
}

void free_sceneattract( sceneattract_t * self ) {
  free_playfield  (sceneattract_playfield(self));
  free_scene      (sceneattract_scene(self));
  free            (self);
}

////////////
// events //
////////////


void sceneattract_start( sceneattract_t * self ) {
  playfield_t * pf = sceneattract_playfield(self);
  

  int uu;
  foreach( playfield_bricks(pf), dt) {
    uu = rnd(1,6);
    if (uu==1)      brick_set_id(dt,"1");
    else if (uu==2) brick_set_id(dt,"2");
    else if (uu==3) brick_set_id(dt,"3");
    else if (uu==4) brick_set_id(dt,"4");
    else if (uu==5) brick_set_id(dt,"5");
    else if (uu==6) brick_set_id(dt,"6");
  }
  
  pball_t * a;
  a = pball();
  point_set( pball_velo(a), 2,1 );
  list_add_last( playfield_balls(pf), a );
  a = pball();
  point_set( pball_velo(a), -1,2 );
  list_add_last( playfield_balls(pf), a );
  a = pball();
  point_set( pball_velo(a), 1,3 );
  list_add_last( playfield_balls(pf), a );
  
}

void sceneattract_update( sceneattract_t * self ) {
  // update base scene object
  scene_update( sceneattract_scene(self) );
  playfield_update( sceneattract_playfield(self) );

  if (gb_button(controller_button_select()) == 2) {
    scene_finished( sceneattract_scene(self) ) = true();
    scene_next( sceneattract_scene(self) ) = "game";
  }
}

void sceneattract_draw( sceneattract_t * self ) {
  playfield_draw( sceneattract_playfield(self) );
}

void sceneattract_quit( sceneattract_t * self ) {

}