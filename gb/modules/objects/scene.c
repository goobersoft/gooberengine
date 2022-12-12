
#define scene_name_size() 128

type() {

  local( tag_t * tag );
  // the name of the scene
  local( string_t * name );
  // the dimensions of the scene in pixels
  local( point_t * size );
  // the position of the scene on the screen
  local( point_t * pos );
  // a list of entities contained in the scene
  local( list_t * entities );
  // a counter for the scene's lifespan
  uint_t ticks;
  // a flag indicating that the scene is finished playing. 
  bool_t f_finished;
  // a pointer to the parent scene
  foreign( void * source );

} scene_t;

#define scene_tag(self)           (self->tag)
#define scene_name(self)          (self->name)
#define scene_size(self)          (self->size)
#define scene_pos(self)           (self->pos)
#define scene_entities(self)      (self->entities)
#define scene_ticks(self)         (self->ticks)
#define scene_f_finished(self)    (self->f_finished)
#define scene_source(self)        (self->source)

// helper macros
#define scene_id(self) tag_id(scene_tag(self))
// helper functions
#define scene_set_class(self,cs)      tag_set_class(scene_tag(self),cs)
#define scene_set_id(self,id)         tag_set_id(scene_tag(self),id)


/////////
// new //
/////////

void scene_init( scene_t * self, void * p, char * id ) {
  scene_tag(self)            = tag(self,"scene");
  scene_id(self)             = id;
  scene_name(self)           = string(scene_name_size());
  scene_size(self)           = point(400,240);
  scene_pos(self)            = point(0,0);
  scene_entities(self)       = list();
  scene_f_finished(self)     = false();
  scene_source(self)         = p;
}

scene_t * scene(void * p, char * id) {
  scene_t * r       = alloc(scene_t);
  scene_init        (r,p,id);
  return r;
}

void free_scene( scene_t * self ) {
  free_tag    (scene_tag(self));
  free_string (scene_name(self));
  free_point  (scene_size(self));
  free_point  (scene_pos(self));
  free_list   (scene_entities(self));
}

///////////////
// functions //
///////////////

void scene_add_entity( scene_t * self, entity_t * e ) {
  // add the entity to the last of the entity list
  list_add_last(scene_entities(self), e);
}

bool_t scene_remove_entity( scene_t * self, entity_t * e ) {
  bool_t b = list_find(scene_entities(self),e);
  if (b) {
    list_remove_first(scene_entities(self));
  }
  return b;
}

void scene_set_name( scene_t * self, char * u ) {
  // despite the name, the copy function of the string means
  // to copy u to its internal data.
  string_copy(scene_name(self), u);
}

void scene_set_source( scene_t * self, void * src ) {
  scene_source(self) = src;
}

void scene_finish( scene_t * self ) {
  scene_f_finished(self) = true();
}

////////////
// events //
////////////

void scene_update( scene_t * self ) {
  // a scene will stop playing if it is flagged as finished.
  if (scene_f_finished(self) == false()) {
    scene_ticks(self) += 1;
  }
}