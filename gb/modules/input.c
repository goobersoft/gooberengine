


////////////
// joypad //
////////////

type() {

} input_joypad_t;

//////////////
// keyboard //
//////////////

#define input_keyboard_max_keys() 256

type() {

  byte_t keys[input_keyboard_max_keys()];

} input_keyboard_t;

///////////
// mouse //
///////////

#define input_mouse_max_buttons() 10

type() {

  byte_t buttons[input_mouse_max_buttons()];

} input_mouse_t;

///////////
// input //
///////////

type () {

  input_keyboard_t keyboard;
  input_joypad_t   joypad;
  input_mouse_t    mouse;

} input_t;