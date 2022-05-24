


//////////
// type //
//////////

type() {

  point_t pos;
  point_t pos_old;
  point_t pos_diff;

  image_t * image;
  point_t image_pos;
  point_t image_size;

} mouse_t;

#define mouse_pos(self)        (&self->pos)
#define mouse_pos_old(self)    (&self->pos_old)
#define mouse_pos_diff(self)   (&self->pos_diff)
#define mouse_image(self)      (self->image)
#define mouse_image_pos(self)  (&self->image_pos)
#define mouse_image_size(self) (&self->image_size)

/////////
// new //
/////////

void mouse_init( mouse_t * self ) {
  point_init(mouse_pos(self),      0,0);
  point_init(mouse_pos_old(self),  0,0);
  point_init(mouse_pos_diff(self), 0,0);

}

mouse_t * mouse() {
  mouse_t * r = alloc(mouse_t);
  mouse_init(r);
  return r;
}

/////////
// del //
/////////

#define free_mouse(m) free(m)

///////////////
// functions //
///////////////

void mouse_set_image( mouse_t * self, image_t * u ) {
  mouse_image(self) = u;
}

void mouse_set_image_pos( mouse_t * self, int x, int y ) {
  point_set( mouse_image_pos(self), x, y );
}

void mouse_set_image_size( mouse_t * self, int w, int h ) {
  point_set( mouse_image_pos(self), w, h );
}

////////////
// events //
////////////

void mouse_update( mouse_t * self ) {
  int btn, xx, yy;
  btn = SDL_GetMouseState(ref(xx),ref(yy));
  point_set( mouse_pos_old(self), point_x(mouse_pos(self)), point_y(mouse_pos(self)) );
  point_set( mouse_pos(self), xx, yy );
  point_set( mouse_pos_diff(self), 
    point_x(mouse_pos(self)) - point_x(mouse_pos_old(self)),
    point_y(mouse_pos(self)) - point_y(mouse_pos_old(self))
  );
}