
#define menu_max_label_length() 80

type() {

  // current cursor
  int cursor;
  // maximum cursor lines
  int maxlines;
  // selected action
  int selection;
  // selection direction (0 for spacebar, -1 for left, 1 for right)
  int selection_dir;
  // labels (an array of string objects)
  local ( string_t ** labels );
  // actions (an array of integer objects)
  local ( int * actions );
  //
  local ( int * actions_left );
  //
  local ( int * actions_right );
  

} menu_t;

#define menu_cursor(self)         (self->cursor)
#define menu_maxlines(self)       (self->maxlines)
#define menu_selection(self)      (self->selection)
#define menu_selection_dir(self)  (self->selection_dir)
#define menu_labels(self)         (self->labels)
#define menu_actions(self)        (self->actions)
#define menu_actions_left(self)   (self->actions_left)
#define menu_actions_right(self)  (self->actions_right)

/////////
// new //
/////////

void init_menu( menu_t * self, int n ) {
  menu_cursor(self)         = 0;
  menu_maxlines(self)       = n;
  menu_selection(self)      = -1;
  menu_selection_dir(self)  = 0;
  menu_labels(self)         = allocv(string_t*,n);
  menu_actions(self)        = allocv(int,n);
  menu_actions_left(self)   = allocv(int,n);
  menu_actions_right(self)  = allocv(int,n);

  loop(i,menu_maxlines(self)) {
    menu_labels(self)[i]        = string(menu_max_label_length());
    menu_actions(self)[i]       = 0;
    menu_actions_left(self)[i]  = 0;
    menu_actions_right(self)[i] = 0;
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
  }
  free( menu_labels(self) );
  free( menu_actions(self) );
  free( menu_actions_left(self) );
  free( menu_actions_right(self) );
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

void menu_set_action( menu_t * self, int n, int m) {
  if inrange(n,0,menu_maxlines(self)) {
    menu_actions(self)[n] = m;
  }
}

void menu_set_action_left( menu_t * self, int n, int m ) {
  if inrange(n,0,menu_maxlines(self)) {
    menu_actions_left(self)[n] = m;
  }
}

void menu_set_action_right( menu_t * self, int n, int m ) {
  if inrange(n,0,menu_maxlines(self)) {
    menu_actions_right(self)[n] = m;
  }
}

void menu_cursor_up( menu_t * self ) {
  menu_cursor(self) = low(menu_cursor(self)-1,0);
}

void menu_cursor_down( menu_t * self ) {
  menu_cursor(self) = high(menu_cursor(self)+1,menu_maxlines(self)-1);
}

void menu_select( menu_t * self, int d ) {
  menu_selection(self)      = menu_cursor(self);
  menu_selection_dir(self)  = d;
}

void menu_cursor_left( menu_t * self ) {
  menu_select(self,-1);
}

void menu_cursor_right( menu_t * self ) {
  menu_select(self,0);
}



void menu_unselect( menu_t * self ) {
  menu_selection(self)      = -1;
  menu_selection_dir(self)  = 0;
}