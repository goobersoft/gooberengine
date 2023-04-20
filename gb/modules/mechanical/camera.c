
//////////
// type //
//////////

type() {

  // position on screen
  local( point_t * pos );
  // size of the camera viewport
  local( point_t * size );

} camera_t;


// the camera reuses the tag's ID field for its
// own purposes. This is the conventional way to handle
// 'inheritance' in C with this project.
#define camera_pos(self)    (self->pos)
#define camera_size(self)   (self->size)

#define camera_set_pos(self,x,y)    point_set(camera_pos(self),x,y)
#define camera_set_size(self,w,h)   point_set(camera_size(self),w,h)


/////////
// new //
/////////

void init_camera( camera_t * self ) {
  // allocate the tag.
  camera_pos(self)     = point(0,0);
  camera_size(self)    = point(400,240);
}

camera_t * camera(void) {
  camera_t * self = alloc(camera_t);
  init_camera(self);
  return self;
}

///////////////
// functions //
///////////////

////////////
// events //
////////////