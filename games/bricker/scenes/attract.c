
type() {

  // original scene object
  local( scene_t * scene );

} scene_attract_t;

#define scene_attract_scene(self) (self->scene)

/////////
// new //
/////////

void init_scene_attract( scene_attract_t * self ) {
  // set scene object id to "attract"
  scene_attract_scene(self) = scene(self,"attract");

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

void scene_attract_update( scene_attract_t * self ) {
  // update base scene object
  scene_update( scene_attract_scene(self) );
}