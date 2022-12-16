
///////////
// input ///////////////////////////////////////////////////////////////////////
///////////

#define input_keyboard_max_keys() 256
#define input_mouse_max_buttons() 10
#define input_key(a) SDL_SCANCODE_##a

type() {

  struct {
    byte_t keys[input_keyboard_max_keys()];
  } keyboard;

  struct {
    byte_t buttons[input_mouse_max_buttons()];
  } mouse;

} input_t;

#define input_keyboard_keys(self) (self->keyboard.keys)
#define input_mouse_buttons(self) (self->mouse.buttons)

/////////
// new //
/////////

void free_input( input_t * self ) {
  free(self);
}

void init_input( input_t * self ) {
  log("%p",ref(self->keyboard.keys));
}

input_t * input() {
  input_t * self = alloc(input_t);
  init_input(self);
  return self;
}


///////////////
// functions //
///////////////

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

////////////
// events //
////////////

void input_update( input_t * self ) {
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