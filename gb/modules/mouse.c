


//////////
// type //
//////////

type() {

  bool_t  visible;

  point_t pos;
  point_t pos_old;
  point_t pos_diff;

  colormap_t * colormap;
  point_t colormap_pos;
  point_t colormap_size;

} mouse_t;

#define mouse_visible(self)       (self->visible)
#define mouse_pos(self)           (&self->pos)
#define mouse_pos_old(self)       (&self->pos_old)
#define mouse_pos_diff(self)      (&self->pos_diff)
#define mouse_colormap(self)      (self->colormap)
#define mouse_colormap_pos(self)  (&self->colormap_pos)
#define mouse_colormap_size(self) (&self->colormap_size)

#define mouse_x(self) (self->pos.x)
#define mouse_y(self) (self->pos.y)

/////////
// new //
/////////

void mouse_init( mouse_t * self ) {
  point_init(mouse_pos(self),      0,0);
  point_init(mouse_pos_old(self),  0,0);
  point_init(mouse_pos_diff(self), 0,0);

}

mouse_t * mouse( colormap_t * c ) {
  mouse_t * r = alloc(mouse_t);
  mouse_visible(r) = true();
  point_set(mouse_pos(r), 0,0);
  point_set(mouse_pos_old(r), 0,0);
  point_set(mouse_pos_diff(r), 0,0);
  
  mouse_colormap(r) = c;
  point_set(mouse_colormap_pos(r),  0,0);
  point_set(mouse_colormap_size(r), 10,10);
  return r;
}

/////////
// del //
/////////

#define free_mouse(m) free(m)

///////////////
// functions //
///////////////

void mouse_set_visible( mouse_t * self, bool_t b ) {
  mouse_visible(self) = bool(b);
  if (mouse_visible(self)==true()) {
    SDL_ShowCursor(SDL_DISABLE);
  }
  else {
    SDL_ShowCursor(SDL_ENABLE);
  }
}

void mouse_set_colormap( mouse_t * self, colormap_t * c ) {
  mouse_colormap(self) = c;
}

void mouse_set_colormap_rect( mouse_t * self, int x, int y, int w, int h ) {
  point_set(mouse_colormap_pos(self),x,y);
  point_set(mouse_colormap_size(self),w,h);
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