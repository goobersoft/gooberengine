
#define scene_name_size() 128

type() {

  // a counter for the scene's lifespan
  uint_t ticks;
  // a flag indicating that the scene is finished playing.
  bool_t finished;
  // a constant integer which determines the next scene id
  char * next;
  //
  local( object_t * spec );
  //


} scene_t;

#define scene_ticks(self)         (self->ticks)
#define scene_finished(self)      (self->finished)
#define scene_next(self)          (self->next)
#define scene_spec(self)          (self->spec)

#define scene_set_spec(self,a,b)  object_set(scene_spec(self),a,b)
#define scene_class(self)         object_class(scene_spec(self))
#define scene_get_spec(self)      object_data(scene_spec(self))

/////////
// new //
/////////

void scene_init( scene_t * self, char * cls, void * dt) {
  scene_spec(self)           = object(cls,dt);
  scene_finished(self)       = false();
  scene_next(self)           = "";
  
}

scene_t * scene(char * cls, void * dt) {
  scene_t * self    = alloc(scene_t);
  scene_init        (self,cls,dt);
  return self;
}

void free_scene( scene_t * self ) {
  free_object (scene_spec(self));
  free        (self);
}

///////////////
// functions //
///////////////

void scene_finish( scene_t * self ) {
  scene_finished(self) = true();
}

////////////
// events //
////////////

void scene_update( scene_t * self ) {
  // a scene will stop playing if it is flagged as finished.
  if (scene_finished(self) == false()) {
    scene_ticks(self) += 1;
  }
}