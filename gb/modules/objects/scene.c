
#define scene_name_size() 128

type() {

  // tag
  local( tag_t * tag );
  // a counter for the scene's lifespan
  uint_t ticks;
  // a flag indicating that the scene is finished playing. 
  bool_t finished;
  // a pointer to the parent scene
  foreign( void * source );

} scene_t;

#define scene_tag(self)           (self->tag)
#define scene_id(self)            tag_id(scene_tag(self))
#define scene_size(self)          (self->size)
#define scene_ticks(self)         (self->ticks)
#define scene_finished(self)      (self->finished)
#define scene_source(self)        (self->source)

// helper macros
#define scene_set_id(self,id)     tag_set_id(scene_tag(self),id)


/////////
// new //
/////////

void scene_init( scene_t * self, void * p, char * id ) {
  scene_tag(self)            = tag(self,"scene");
  scene_id(self)             = id;
  scene_finished(self)       = false();
  scene_source(self)         = p;
}

scene_t * scene(void * p, char * id) {
  scene_t * r       = alloc(scene_t);
  scene_init        (r,p,id);
  return r;
}

void free_scene( scene_t * self ) {
  free_tag    (scene_tag(self));
  free        (self);
}

///////////////
// functions //
///////////////

void scene_set_source( scene_t * self, void * src ) {
  scene_source(self) = src;
}

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