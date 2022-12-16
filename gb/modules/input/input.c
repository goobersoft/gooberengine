
///////////
// input ///////////////////////////////////////////////////////////////////////
///////////

#define input_keyboard_max_keys() 256

#define input_key(a) SDL_SCANCODE_##a

#define input_mouse_button_left()      1
#define input_mouse_button_middle()    2
#define input_mouse_button_right()     3
#define input_mouse_button_back()      4
#define input_mouse_button_forward()   5
#define input_mouse_max_buttons()     10
#define input_mouse_click_time_max()  30 // frames

type() {

  struct {
    byte_t keys[input_keyboard_max_keys()];
  } keyboard;

  struct {
    bool_t    visible;
    point_t   pos;
    point_t   pos_old;
    point_t   pos_diff;
    point_t   click_pos;
    int       click_time;
    byte_t    buttons[input_mouse_max_buttons()];
  } mouse;

} input_t;

#define input_keyboard_keys(self) (self->keyboard.keys)

#define input_mouse_visible(self)    (self->mouse.visible)
#define input_mouse_pos(self)        ref(self->mouse.pos)
#define input_mouse_pos_old(self)    ref(self->mouse.pos_old)
#define input_mouse_pos_diff(self)   ref(self->mouse.pos_diff)
#define input_mouse_click_pos(self)  ref(self->mouse.click_pos)
#define input_mouse_click_time(self) (self->mouse.click_time)
#define input_mouse_buttons(self)    (self->mouse.buttons)

/////////
// new //
/////////

void free_input( input_t * self ) {
  free(self);
}

void init_input( input_t * self ) {
  input_mouse_visible(self) = false();
  SDL_ShowCursor(SDL_ENABLE);

  point_set(input_mouse_pos(self), 0,0);
  point_set(input_mouse_pos_old(self), 0,0);
  point_set(input_mouse_pos_diff(self), 0,0);
  
  point_set(input_mouse_click_pos(self), 0,0);
  input_mouse_click_time(self) = input_mouse_click_time_max();

  loop(i,input_mouse_max_buttons()) {
    input_mouse_buttons(self)[i] = 0;
  }
  loop(i,input_keyboard_max_keys()) {
    input_keyboard_keys(self)[i] = 0;
  }
}

input_t * input() {
  input_t * self = alloc(input_t);
  init_input(self);
  return self;
}


///////////////
// functions //
///////////////

// b is currently unused but will stand for the button which
// was clicked (constants above)
int input_mouse_click( input_t * self, int x, int y, int b ) {
  point_set(input_mouse_click_pos(self), x, y);
  input_mouse_click_time(self) = 0;
}

int input_get_key_state( input_t * self, int u ) {
  u = clamp(u,0,255);
  return input_keyboard_keys(self)[u];
}

int input_get_key_pressed( input_t * self, int u ) {
  u = clamp(u,0,255);
  return input_keyboard_keys(self)[u] == 2;
}

int input_get_key_held( input_t * self, int u ) {
  u = clamp(u,0,255);
  return input_keyboard_keys(self)[u] >= 0;
}

int input_get_mouse_button( input_t * self, int u ) {
  u = clamp(u,0,input_mouse_max_buttons()-1);
  return input_mouse_buttons(self)[u];
}

int input_get_mouse_button_pressed( input_t * self, int u ) {
  u = clamp(u,0,input_mouse_max_buttons()-1);
  return input_mouse_buttons(self)[u] == 2;
}

int input_get_mouse_button_held( input_t * self, int u ) {
  u = clamp(u,0,input_mouse_max_buttons()-1);
  return input_mouse_buttons(self)[u] >= 1;
}

////////////
// events //
////////////

void input_update_keyboard( input_t * self ) {
  // update the keyboard
  const Uint8 * b = SDL_GetKeyboardState( null() );
  loop(i,input_keyboard_max_keys()) {
    if (b[i] == 1) {
      if (input_keyboard_keys(self)[i] == 0) {
        input_keyboard_keys(self)[i] = 2;
      }
      else {
        input_keyboard_keys(self)[i] = 1;
      }
    }
    else {
      input_keyboard_keys(self)[i] = 0;
    }
  }
}

void input_update_mouse( input_t * self ) {
  int btn, xx, yy;
  btn = SDL_GetMouseState(ref(xx),ref(yy));

  loop(i,input_mouse_max_buttons()) {
    if ((btn & (1<<i)) != 0) {
      if (input_mouse_buttons(self)[i] == 0) {
        input_mouse_buttons(self)[i] = 2;
        input_mouse_click(self,xx,yy,i);
      }
      else if (input_mouse_buttons(self)[i] == 2) {
        input_mouse_buttons(self)[i] = 1;
      }
    }
    else {
      input_mouse_buttons(self)[i] = 0;
    }
    
  }

  point_set( input_mouse_pos_old(self), point_x(input_mouse_pos(self)), point_y(input_mouse_pos(self)) );
  point_set( input_mouse_pos(self), xx, yy );
  point_set( input_mouse_pos_diff(self), 
    point_x(input_mouse_pos(self)) - point_x(input_mouse_pos_old(self)),
    point_y(input_mouse_pos(self)) - point_y(input_mouse_pos_old(self))
  );

  input_mouse_click_time(self) = high(input_mouse_click_time(self)+2,input_mouse_click_time_max());
}

void input_update( input_t * self ) {
  input_update_keyboard(self);
  input_update_mouse(self);
}