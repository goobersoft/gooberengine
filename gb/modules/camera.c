
//////////
// type //
//////////

type() {

  // position on screen
  point_t * pos;
  // size of the camera viewport
  point_t * size;
  // entity of camera used for scenes
  entity_t * entity;
  // the scene
  scene_t * scene;

} camera_t;

#define camera_pos(self)    (self->pos)
#define camera_size(self)   (self->size)
#define camera_entity(self) (self->entity)
#define camera_scene(self)  (self->scene)

/////////
// new //
/////////

void camera_init( camera_t * self ) {
  camera_pos(self)     = point(0,0);
  camera_size(self)    = point(400,240);
  camera_entity(self)  = entity(self);
}

camera_t * camera() {
  camera_t * r = alloc(camera_t);
  camera_init(r);
  return r;
}

///////////////
// functions //
///////////////

void camera_set_scene( camera_t * self, scene_t * s ) {
  camera_scene(self) = s;
}