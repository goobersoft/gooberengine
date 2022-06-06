
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

  byte_t buttons[ controller_max_buttons() ];

} controller_t;

#define controller_buttons(self) (self->buttons)

/////////
// new //
/////////

controller_t * controller() {
  controller_t * r = alloc(controller_t);
  loop(i,0,controller_max_buttons()) {
    controller_buttons(r)[i] = controller_button_released();
  }
  return r;
}

///////////////
// functions //
///////////////

void controller_press_button( controller_t * self, int n ) {
  controller_buttons(self)[n] = controller_button_pressed();
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
  return controller_get_button(self,n) == controller_button_held();
}

int controller_is_button_released( controller_t * self, int n ) {
  return controller_get_button(self,n) == controller_button_released();
}

////////////
// events //
////////////

void controller_update( controller_t * self ) {
  loop(i,0,controller_max_buttons()) {
    if (controller_buttons(self)[i] == 2) {
      controller_buttons(self)[i] = 1;
    }
  }
}