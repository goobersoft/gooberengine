


#define mouse_max_buttons()     5
#define mouse_button_released() 0
#define mouse_button_pressed()  2
#define mouse_button_held()     1

#define mouse_button_left()     1
#define mouse_button_middle()   2
#define mouse_button_right()    3
#define mouse_button_back()     4
#define mouse_button_forward()  5

//////////
// type //
//////////

type() {
  // visible
  bool_t  visible;

  // position data
  point_t pos;
  point_t pos_old;
  point_t pos_diff;

  // mouse icon
  colormap_t * colormap;
  point_t colormap_pos;
  point_t colormap_size;

  // buttons
  int buttons[mouse_max_buttons()];

  // visual
  visual_t * visual;

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

#define mouse_buttons(self) (self->buttons)
#define mouse_visual(self)  (self->visual)

/////////
// new //
/////////

mouse_t * mouse( visual_t * v, colormap_t * c ) {
  mouse_t * r = alloc(mouse_t);
  mouse_visible(r) = true();
  point_set(mouse_pos(r), 0,0);
  point_set(mouse_pos_old(r), 0,0);
  point_set(mouse_pos_diff(r), 0,0);
  
  mouse_visual(r) = v;
  
  mouse_colormap(r) = c;
  point_set(mouse_colormap_pos(r),  0,0);
  point_set(mouse_colormap_size(r), 10,10);

  loop(i,0,mouse_max_buttons()) {
    mouse_buttons(r)[i] = mouse_button_released();
  }
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

  loop(i,0,mouse_max_buttons()) {
    if ((btn & (1<<i)) != 0) {
      if (mouse_buttons(self)[i] == mouse_button_released()) {
        mouse_buttons(self)[i] = mouse_button_pressed();
      }
      else if (mouse_buttons(self)[i] == mouse_button_pressed()) {
        mouse_buttons(self)[i] = mouse_button_held();
      }
    }
    else {
      mouse_buttons(self)[i] = mouse_button_released();
    }
    
  }

  /*
  log("%d%d%d%d%d",
    mouse_buttons(self)[0],
    mouse_buttons(self)[1],
    mouse_buttons(self)[2],
    mouse_buttons(self)[3],
    mouse_buttons(self)[4]);
  */

  point_set( mouse_pos_old(self), point_x(mouse_pos(self)), point_y(mouse_pos(self)) );
  point_set( mouse_pos(self), xx, yy );
  point_set( mouse_pos_diff(self), 
    point_x(mouse_pos(self)) - point_x(mouse_pos_old(self)),
    point_y(mouse_pos(self)) - point_y(mouse_pos_old(self))
  );
}