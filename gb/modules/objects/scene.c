
#define scene_name_size() 128

type() {

  tag_t * tag;
  // the name of the scene
  string_t * name;
  // the dimensions of the scene in pixels
  point_t * size;
  // the position of the scene on the screen
  point_t * pos;
  // a list of entities contained in the scene
  list_t * entities;
  // a counter for the scene's lifespan
  uint_t ticks;
  // a flag indicating that the scene is finished playing. 
  bool_t flag_finished;
  // a pointer to the parent scene
  void * parent;

} scene_t;

#define scene_tag(self)           (self->tag)
#define scene_name(self)          (self->name)
#define scene_size(self)          (self->size)
#define scene_pos(self)           (self->pos)
#define scene_entities(self)      (self->entities)
#define scene_ticks(self)         (self->ticks)
#define scene_flag_finished(self) (self->flag_finished)
#define scene_parent(self)        (self->parent)

// helper macros
#define scene_id(self) tag_id(scene_tag(self))
// helper functions
#define scene_set_class(self,cs)      tag_set_class(scene_tag(self),cs)
#define scene_set_id(self,id)         tag_set_id(scene_tag(self),id)


/////////
// new //
/////////

scene_t * scene(void * p) {
  scene_t * r             = alloc(scene_t);
  scene_tag(r)            = tag(r,"scene");
  scene_name(r)           = string(scene_name_size());
  scene_size(r)           = point(400,240);
  scene_pos(r)            = point(0,0);
  scene_entities(r)       = list();
  scene_flag_finished(r)  = false();
  scene_parent(r)         = p;
  return r;
}

void free_scene( scene_t * self ) {
  free_tag(scene_tag(self));
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

void scene_set_parent( scene_t * self, void * p ) {
  scene_parent(self) = p;
}

void scene_finish( scene_t * self ) {
  scene_flag_finished(self) = true();
}

////////////
// events //
////////////

void scene_update( scene_t * self ) {
  // a scene will stop playing if it is flagged as finished.
  if (scene_flag_finished(self) == false()) {
    scene_ticks(self) += 1;
  }
}