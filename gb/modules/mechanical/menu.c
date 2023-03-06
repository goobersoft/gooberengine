
#define menu_max_label_length() 80

type() {

  // current cursor
  int cursor;
  // maximum cursor lines
  int maxlines;
  // selected action
  int selected;
  // labels (an array of string objects)
  local( string_t ** labels );
  // actions (an array of integer objects)
  local( integer_t ** actions );
  

} menu_t;

#define menu_cursor(self)     (self->cursor)
#define menu_maxlines(self)   (self->maxlines)
#define menu_selected(self)   (self->selected)
#define menu_labels(self)     (self->labels)
#define menu_actions(self)    (self->actions)

/////////
// new //
/////////

void init_menu( menu_t * self, int n ) {
  menu_cursor(self)     = 0;
  menu_maxlines(self)   = n;
  menu_selected(self)   = -1;
  menu_labels(self)     = allocv(string_t*,n);
  menu_actions(self)    = allocv(integer_t*,n);

  loop(i,menu_maxlines(self)) {
    menu_labels(self)[i]  = string(menu_max_label_length());
    menu_actions(self)[i] = integer(0);
  }
}

menu_t * menu( int n ) {
  menu_t * self         = alloc(menu_t);
  init_menu(self,n);
  return self;
}

void free_menu( menu_t * self ) {
  loop(i,menu_maxlines(self)) {
    free_string( menu_labels(self)[i] );
    free_integer( menu_actions(self)[i] );
  }
  free( menu_labels(self) );
  free( menu_actions(self) );
  free( self );
}

///////////////
// functions //
///////////////

void menu_set_cursor( menu_t * self, int n ) {
  menu_cursor(self) = clamp(n,0,menu_maxlines(self)-1);
}

void menu_set_label( menu_t * self, int n, char * c ) {
  string_copy(menu_labels(self)[n], c);
}

void menu_set_action( menu_t * self, int n ) {
  integer_set(menu_actions(self)[n], n);
}

void menu_cursor_up( menu_t * self ) {
  menu_cursor(self) = low(menu_cursor(self)-1,0);
}

void menu_cursor_down( menu_t * self ) {
  menu_cursor(self) = high(menu_cursor(self)+1,menu_maxlines(self)-1);
}

int menu_select( menu_t * self ) {
  menu_selected(self) = menu_cursor(self);
  return menu_selected(self);
}

void menu_unselect( menu_t * self ) {
  menu_selected(self) = -1;
}