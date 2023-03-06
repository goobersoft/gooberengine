
type() {

  list_t * elements;

} ui_t;

#define ui_elements(self) (self->elements)

void init_ui( ui_t * self ) {
  ui_elements(self) = list();
}

ui_t * ui() {
  ui_t * self = alloc(ui_t);
  init_ui(self);
  return self;
}