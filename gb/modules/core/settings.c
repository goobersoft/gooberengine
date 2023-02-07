
type() {

  // scale of the window
  int     window_scale;
  // scanlines on screen enabled?
  bool_t  scanlines;
  // fullscreen enabled?
  bool_t  fullscreen;

} settings_t;

#define settings_window_scale(self) (self->window_scale)
#define settings_scanlines(self)    (self->scanlines)
#define settings_fullscreen(self)   (self->fullscreen)

/////////
// new //
/////////

void init_settings(settings_t * self) {
  settings_window_scale(self)   = 3;
  settings_scanlines(self)      = true();
  settings_fullscreen(self)     = false();
}

settings_t * settings() {
  settings_t * self = alloc(settings_t);
  init_settings(self);
  return self;
}

void free_settings(settings_t * self) {
  free(self);
}

///////////////
// functions //
///////////////

// utilizes GBML-styled string format
string_t * settings_to_string(settings_t * self) {
  // allocate a new string
  string_t * s = string(100);
  // make a cursor
  int u = 0;
  // ---------------
  u = string_copy_at(s,u,"window-scale=");
  u = string_copy_number_at(s,u,settings_window_scale(self));
  u = string_copy_at(s,u,";scanlines=");
  u = string_copy_number_at(s,u,settings_scanlines(self));
  u = string_copy_at(s,u,";fullscreen=");
  u = string_copy_number_at(s,u,settings_fullscreen(self));
  u = string_copy_at(s,u,";");
  u = string_end(s,u);
  // ---------------
  // crop the string to the necessary length
  string_crop(s);
  // return the string
  return s;

}