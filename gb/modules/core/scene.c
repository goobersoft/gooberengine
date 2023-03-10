
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
  // extender
  local( object_t * spec );
  // function pointers
  void (*fn_start)(void*);
  void (*fn_update)(void*);
  void (*fn_draw)(void*);
  void (*fn_quit)(void*);

} scene_t;

#define scene_ticks(self)         (self->ticks)
#define scene_finished(self)      (self->finished)
#define scene_next(self)          (self->next)
#define scene_events(self)        (self->events)
#define scene_spec(self)          (self->spec)

#define scene_set_spec(self,a,b)  object_set(scene_spec(self),a,b)
#define scene_class(self)         object_class(scene_spec(self))
#define scene_get_spec(self)      object_data(scene_spec(self))

#define scene_add_event(self,b)   list_add_last(scene_events(self),b)
#define scene_clear_events(self)  list_clear(scene_events(self))

#define scene_fn_start(self)      (self->fn_start)
#define scene_fn_update(self)     (self->fn_update)
#define scene_fn_draw(self)       (self->fn_draw)
#define scene_fn_quit(self)       (self->fn_quit)

/////////
// new //
/////////

void scene_emp( void * _ ) {
}

void init_scene( scene_t * self, char * cls, void * dt) {
  scene_spec(self)           = object(cls,dt);
  scene_finished(self)       = false();
  scene_next(self)           = "";
  scene_events(self)         = list(); 
  scene_fn_start(self)       = scene_emp;
  scene_fn_update(self)      = scene_emp;
  scene_fn_draw(self)        = scene_emp;
  scene_fn_quit(self)        = scene_emp;
}

scene_t * scene(char * cls, void * dt) {
  scene_t * self    = alloc(scene_t);
  init_scene        (self,cls,dt);
  return self;
}

void free_scene( scene_t * self ) {
  free_list   (scene_events(self));
  free_object (scene_spec(self));
  free        (self);
}

///////////////
// functions //
///////////////

void scene_finish( scene_t * self ) {
  scene_finished(self) = true();
}

void scene_set_fn_start( scene_t * self, void (*fn)(void*) ) {
  scene_fn_start(self) = fn;
}

void scene_set_fn_update( scene_t * self, void (*fn)(void*) ) {
  scene_fn_update(self) = fn;
}

void scene_set_fn_draw( scene_t * self, void (*fn)(void*) ) {
  scene_fn_draw(self) = fn;
}

void scene_set_fn_quit( scene_t * self, void (*fn)(void*) ) {
  scene_fn_quit(self) = fn;
}

void scene_set_funcs( scene_t * self, void (*fn1)(void*), void (*fn2)(void*),
void (*fn3)(void*), void (*fn4)(void*) ) {
  scene_fn_start(self)   = fn1;
  scene_fn_update(self)  = fn2;
  scene_fn_draw(self)    = fn3;
  scene_fn_quit(self)    = fn4;
}

////////////
// events //
////////////

void scene_start( scene_t * self ) {
  scene_fn_start(self)(self);
}

void scene_update( scene_t * self ) {
  // a scene will stop playing if it is flagged as finished.
  if (scene_finished(self) == false()) {
    scene_ticks(self) += 1;
    scene_fn_update(self)(self);
  }
}

void scene_draw( scene_t * self ) {
  scene_fn_draw(self)(self);
}

void scene_quit( scene_t * self ) {
  scene_fn_quit(self)(self);
}

//////////////////////////////////////////////////////////////////////////////

type() {

  scene_t * scene_curr;
  dict_t  * scenes;

} scenebook_t;

#define scenebook_curr(self)   (self->scene_curr)
#define scenebook_scenes(self) (self->scenes)

void init_scenebook( scenebook_t * self ) {
  scenebook_curr(self)   = null();
  scenebook_scenes(self) = dict();
}

scenebook_t * scenebook() {
  scenebook_t * self = alloc(scenebook_t);
  init_scenebook(self);
  return self;
}

///////////
// funcs //
///////////

void scenebook_set_scene( scenebook_t * self, char * id ) {

}

void scenebook_add_scene( scenebook_t * self, char * id, void * dt ) {
  dict_set(scenebook_scenes(self), id, dt);
}