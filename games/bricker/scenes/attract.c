
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
  scene_attract_scene(self)      = scene(self,"attract");
  scene_attract_playfield(self)  = playfield();
}

scene_t * scene_attract() {
  scene_attract_t * self = alloc(scene_attract_t);
  init_scene_attract(self);
  // return the scene, not itself.
  // Since the scene object has this object as its source,
  // we can cast the internal source pointer later.
  return scene_attract_scene(self);
}

////////////
// events //
////////////

void scene_attract_start( scene_attract_t * self ) {
  /*
  int uu = rnd(1,6);
  if (uu==1)      brick_set_id(b,"1");
  else if (uu==2) brick_set_id(b,"2");
  else if (uu==3) brick_set_id(b,"3");
  else if (uu==4) brick_set_id(b,"4");
  else if (uu==5) brick_set_id(b,"5");
  else if (uu==6) brick_set_id(b,"6");
  */
  /*
  pball_t * a;
  a = pball();
  point_set( pball_velo(a), 2,1 );
  list_add_last( playfield_balls(self), pball_entity(a) );
  a = pball();
  point_set( pball_velo(a), -1,2 );
  list_add_last( playfield_balls(self), pball_entity(a) );
  a = pball();
  point_set( pball_velo(a), 1,3 );
  list_add_last( playfield_balls(self), pball_entity(a) );
  */

}

void scene_attract_update( scene_attract_t * self ) {

  // update base scene object
  scene_update( scene_attract_scene(self) );
  playfield_update( scene_attract_playfield(self) );
}

void scene_attract_draw( scene_attract_t * self ) {
  playfield_draw( scene_attract_playfield(self) );
}