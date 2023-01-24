
//////////
// type //
//////////

type() {

  // tag object
  local( tag_t * tag );
  // position on screen
  local( point_t * pos );
  // size of the camera viewport
  local( point_t * size );
  // owner of camera
  foreign( void * source );

} camera_t;

#define camera_tag(self)    (self->tag)
// the camera reuses the tag's ID field for its
// own purposes. This is the conventional way to handle
// 'inheritance' in C with this project.
#define camera_id(self)     tag_id(camera_tag(self))
#define camera_pos(self)    (self->pos)
#define camera_size(self)   (self->size)
#define camera_source(self) (self->source)


/////////
// new //
/////////

void camera_init( camera_t * self ) {
  // allocate the tag.
  camera_tag(self)     = tag(self,"camera");
  camera_pos(self)     = point(0,0);
  camera_size(self)    = point(400,240);
  // the camera initially does not have an owner.
  camera_source(self)  = null();
}

camera_t * camera() {
  camera_t * r = alloc(camera_t);
  camera_init(r);
  return r;
}

///////////////
// functions //
///////////////

void camera_set_source( camera_t * self, void * src ) {
  camera_source(self) = src;
}

////////////
// events //
////////////

void camera_draw( camera_t * self ) {
}