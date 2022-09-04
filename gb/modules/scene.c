
#define scene_name_size() 128

type() {

  // the name of the scene
  string_t * name;
  // the dimensions of the scene in pixels
  point_t  * size;
  // the position of the scene on the screen
  point_t * pos;
  // a list of entities contained in the scene
  list_t * entities;

} scene_t;

#define scene_name(self)     (self->name)
#define scene_size(self)     (self->size)
#define scene_pos(self)      (self->pos)
#define scene_entities(self) (self->entities)

/////////
// new //
/////////

scene_t * scene() {
  scene_t * r       = alloc(scene_t);
  scene_name(r)     = string(scene_name_size());
  scene_size(r)     = point(400,240);
  scene_pos(r)      = point(0,0);
  scene_entities(r) = list();
}

///////////////
// functions //
///////////////

void scene_add_entity() {
}