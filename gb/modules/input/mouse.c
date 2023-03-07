


#define mouse_max_buttons()     5
#define mouse_button_released() 0
#define mouse_button_pressed()  2
#define mouse_button_held()     1

#define mouse_button_left()     1
#define mouse_button_middle()   2
#define mouse_button_right()    3
#define mouse_button_back()     4
#define mouse_button_forward()  5

#define mouse_click_time_max()  30 // frames

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

  point_t click_pos;
  int     click_time;

  // buttons
  int buttons[mouse_max_buttons()];

} mouse_t;

#define mouse_visible(self)       (self->visible)
#define mouse_pos(self)           (&self->pos)
#define mouse_pos_old(self)       (&self->pos_old)
#define mouse_pos_diff(self)      (&self->pos_diff)

#define mouse_click_pos(self)     (&self->click_pos)
#define mouse_click_time(self)    (self->click_time)

#define mouse_x(self)             (self->pos.x)
#define mouse_y(self)             (self->pos.y)

#define mouse_buttons(self)       (self->buttons)

/////////
// new //
/////////

void mouse_init( mouse_t * self ) {
  mouse_visible(self) = false();
  SDL_ShowCursor(SDL_ENABLE);

  point_set(mouse_pos(self), 0,0);
  point_set(mouse_pos_old(self), 0,0);
  point_set(mouse_pos_diff(self), 0,0);
  
  point_set(mouse_click_pos(self), 0,0);
  mouse_click_time(self) = mouse_click_time_max();

  loop(i,mouse_max_buttons()) {
    mouse_buttons(self)[i] = mouse_button_released();
  }
}

mouse_t * mouse( ) {
  mouse_t * self = alloc(mouse_t);
  mouse_init( self );
  return self;
}

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

void mouse_click( mouse_t * self, int x, int y, int b ) {
  point_set(mouse_click_pos(self), x, y);
  mouse_click_time(self) = 0;
}


////////////
// events //
////////////

void mouse_update( mouse_t * self ) {
  int btn, xx, yy;
  btn = SDL_GetMouseState(ref(xx),ref(yy));

  loop(i,mouse_max_buttons()) {
    if ((btn & (1<<i)) != 0) {
      if (mouse_buttons(self)[i] == mouse_button_released()) {
        mouse_buttons(self)[i] = mouse_button_pressed();
        mouse_click(self,xx,yy,i);
      }
      else if (mouse_buttons(self)[i] == mouse_button_pressed()) {
        mouse_buttons(self)[i] = mouse_button_held();
      }
    }
    else {
      mouse_buttons(self)[i] = mouse_button_released();
      //mouse_release(self,i);
    }
    
  }

  point_set( mouse_pos_old(self), point_x(mouse_pos(self)), point_y(mouse_pos(self)) );
  point_set( mouse_pos(self), xx, yy );
  point_set( mouse_pos_diff(self), 
    point_x(mouse_pos(self)) - point_x(mouse_pos_old(self)),
    point_y(mouse_pos(self)) - point_y(mouse_pos_old(self))
  );

  mouse_click_time(self) = high(mouse_click_time(self)+2,mouse_click_time_max());
}