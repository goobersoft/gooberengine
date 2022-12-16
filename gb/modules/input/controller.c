
#define controller_max_buttons() 12

#define controller_button_released() 0
#define controller_button_pressed()  2
#define controller_button_held()     1

#define controller_button_up()       0
#define controller_button_right()    1
#define controller_button_down()     2
#define controller_button_left()     3
#define controller_button_a()        4
#define controller_button_b()        5
#define controller_button_c()        6
#define controller_button_d()        7
#define controller_button_l()        8
#define controller_button_r()        9
#define controller_button_start()   10
#define controller_button_select()  11

type() {

  input_t * input;
  byte_t buttons[ controller_max_buttons() ];

} controller_t;

#define controller_input(self)   (self->input)
#define controller_buttons(self) (self->buttons)

/////////
// new //
/////////

void controller_init( controller_t * self, input_t * u ) {
  loop(i,controller_max_buttons()) {
    controller_buttons(self)[i] = controller_button_released();
  }
  controller_input(self) = u;
}

controller_t * controller( input_t * u ) {
  controller_t * self = alloc(controller_t);
  controller_init(self,u);
  return self;
}

///////////////
// functions //
///////////////

void controller_press_button( controller_t * self, int n ) {
  if (controller_buttons(self)[n] == 0) {
    controller_buttons(self)[n] = controller_button_pressed();
  }
}

void controller_release_button( controller_t * self, int n ) {
  controller_buttons(self)[n] = controller_button_released();
}

int controller_get_button( controller_t * self, int n ) {
  return controller_buttons(self)[n];
}

int controller_is_button_pressed( controller_t * self, int n ) {
  return controller_get_button(self,n) == controller_button_pressed();
}

int controller_is_button_held( controller_t * self, int n ) {
  return controller_get_button(self,n) > 0;
}

int controller_is_button_released( controller_t * self, int n ) {
  return controller_get_button(self,n) == 0;
}

////////////
// events //
////////////


void controller_update( controller_t * self ) {
  loop(i,controller_max_buttons()) {
    if (controller_buttons(self)[i] == controller_button_pressed()) {
      controller_buttons(self)[i] = controller_button_held();
    }
  }

  byte_t * b = input_keyboard_keys(controller_input(self));
  if (b[SDL_SCANCODE_UP])      controller_press_button(self,controller_button_up());
  else                         controller_release_button(self,controller_button_up());

  if (b[SDL_SCANCODE_RIGHT])   controller_press_button(self,controller_button_right());
  else                         controller_release_button(self,controller_button_right());

  if (b[SDL_SCANCODE_DOWN])    controller_press_button(self,controller_button_down());
  else                         controller_release_button(self,controller_button_down());

  if (b[SDL_SCANCODE_LEFT])    controller_press_button(self,controller_button_left());
  else                         controller_release_button(self,controller_button_left());

  if (b[SDL_SCANCODE_Z])       controller_press_button(self,controller_button_a());
  else                         controller_release_button(self,controller_button_a());

  if (b[SDL_SCANCODE_X])       controller_press_button(self,controller_button_b());
  else                         controller_release_button(self,controller_button_b());

  if (b[SDL_SCANCODE_C])       controller_press_button(self,controller_button_c());
  else                         controller_release_button(self,controller_button_c());

  if (b[SDL_SCANCODE_V])       controller_press_button(self,controller_button_d());
  else                         controller_release_button(self,controller_button_d());

  if (b[SDL_SCANCODE_SLASH])   controller_press_button(self,controller_button_l());
  else                         controller_release_button(self,controller_button_l());

  if (b[SDL_SCANCODE_RSHIFT])  controller_press_button(self,controller_button_r());
  else                         controller_release_button(self,controller_button_r());

  if (b[SDL_SCANCODE_RETURN])  controller_press_button(self,controller_button_start());
  else                         controller_release_button(self,controller_button_start());

  if (b[SDL_SCANCODE_SPACE])   controller_press_button(self,controller_button_select());
  else                         controller_release_button(self,controller_button_select());
}