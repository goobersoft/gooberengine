
#define scene_name_size() 128

type() {

  // a counter for the scene's lifespan
  uint_t ticks;
  // a flag indicating that the scene is finished playing.
  bool_t finished;
  // a constant string which determines the next scene id
  char * next;
  // list of events
  list_t * events;
  //
  char * id;

} scene_t;

#define scene_ticks(self)         (self->ticks)
#define scene_finished(self)      (self->finished)
#define scene_next(self)          (self->next)
#define scene_events(self)        (self->events)
#define scene_id(self)            (self->id)

#define scene_add_event(self,b)   list_add_last(scene_events(self),b)
#define scene_clear_events(self)  list_clear(scene_events(self))



/////////
// new //
/////////

void init_scene( scene_t * self, char * id ) {
  scene_id(self)             = id;
  scene_finished(self)       = false();
  scene_next(self)           = "";
  scene_events(self)         = list();
}

scene_t * scene( char * id ) {
  scene_t * self    = alloc(scene_t);
  init_scene        (self,id);
  return self;
}

void free_scene( scene_t * self ) {
  free_list   (scene_events(self));
  free        (self);
}

///////////////
// functions //
///////////////

void scene_finish( scene_t * self, char * v ) {
  scene_finished(self) = true();
  scene_next(self)     = v;
}

////////////
// events //
////////////

void scene_start( scene_t * self ) {
}

void scene_update( scene_t * self ) {
  // a scene will stop playing if it is flagged as finished.
  if (scene_finished(self) == false()) {
    scene_ticks(self) += 1;
  }
}

void scene_draw( scene_t * self ) {
}

void scene_quit( scene_t * self ) {
}

//////////////////////////////////////////////////////////////////////////////