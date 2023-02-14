/*
  Menu Entity
*/

#define entmenu_tiles_x() 12

type() {
  // entity object
  local( entity_t * entity );
  // menu object
  local( menu_t * menu );
  // tilemap object
  local( tilemap_t * tilemap );
  // window graphic pos (colormap)
  local( point_t * window_pos );
  // cursor gfx pos (colormap)
  local( point_t * cursor_pos );
  // title of the menu
  local( string_t * title );

} entmenu_t;

// getters
#define entmenu_entity(self)       (self->entity)
#define entmenu_menu(self)         (self->menu)
#define entmenu_tilemap(self)      (self->tilemap)
#define entmenu_window_pos(self)   (self->window_pos)
#define entmenu_cursor_pos(self)   (self->cursor_pos)
#define entmenu_title(self)        (self->title)
// helpers
#define entmenu_pos(self)            entity_pos(entmenu_entity(self))
#define entmenu_pos_x(self)          entity_pos_x(entmenu_entity(self))
#define entmenu_pos_y(self)          entity_pos_y(entmenu_entity(self))
#define entmenu_labels(self)         menu_labels(entmenu_menu(self))
#define entmenu_actions(self)        menu_actions(entmenu_menu(self))
#define entmenu_maxlines(self)       menu_maxlines(entmenu_menu(self))
#define entmenu_tiles_y(self)        tilemap_size_y(entmenu_tilemap(self))
#define entmenu_colormap(self)       tilemap_colormap(entmenu_tilemap(self))
#define entmenu_cursor(self)         menu_cursor(entmenu_menu(self))

#define entmenu_cursor_down(self)    menu_cursor_down(entmenu_menu(self))
#define entmenu_cursor_up(self)      menu_cursor_up(entmenu_menu(self))
#define entmenu_select(self)         menu_select(entmenu_menu(self))


/////////
// new //
/////////

void init_entmenu( entmenu_t * self, int n ) {
  colormap_t * c = gb_get_colormap("gb-0");

  // make a new entity.
  entmenu_entity(self)  = entity(self);
  // make a new menu
  entmenu_menu(self)    = menu(n);
  // make a new tilemap 12 by 20 tiles (10px)
  entmenu_tilemap(self) = tilemap( entmenu_tiles_x(), n+2, c );

  entmenu_window_pos(self) = point(200,150);
  entmenu_cursor_pos(self) = point(290,120);

  // gb-0 20,15
  tilemap_set_offset( entmenu_tilemap(self), 
    point_x(entmenu_window_pos(self)),
    point_y(entmenu_window_pos(self))
  );

  entmenu_title(self) = string(100);
  string_copy(entmenu_title(self),"Untitled");
  
}

entmenu_t * entmenu( int n ) {
  entmenu_t * self = alloc(entmenu_t);
  init_entmenu(self,n);
  return self;
}

void free_entmenu( entmenu_t * self ) {
  free_entity   ( entmenu_entity(self) );
  free_menu     ( entmenu_menu(self) );
  free_tilemap  ( entmenu_tilemap(self) );
  free_point    ( entmenu_window_pos(self) );
  free_point    ( entmenu_cursor_pos(self) );
  free_string   ( entmenu_title(self) );
  free          ( self );
}

///////////
// funcs //
///////////

void entmenu_set_colormap( entmenu_t * self, colormap_t * c ) {
  entmenu_colormap(self) = c;
}

void entmenu_set_label( entmenu_t * self, int c, char * s ) {
  c = wrap(c,0, entmenu_maxlines(self) );
  string_copy( entmenu_labels(self)[c], s );
}

void entmenu_set_title( entmenu_t * self, char * s ) {
  string_copy( entmenu_title(self), s );
}

void entmenu_set_window_pos( entmenu_t * self, int x, int y ) {
  point_set( entmenu_window_pos(self), x, y );
  tilemap_set_offset( entmenu_tilemap(self), 
    point_x(entmenu_window_pos(self)),
    point_y(entmenu_window_pos(self))
  );
}

void entmenu_set_cursor_pos( entmenu_t * self, int x, int y ) {
  point_set( entmenu_cursor_pos(self), x, y );
}

void entmenu_decorate_tilemap( entmenu_t * self ) {
  
  loop(i,entmenu_tiles_x()) {
    loop(j,entmenu_tiles_y(self)) {
      if (i==0 and j==0) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 0, 0);
      }
      else if (i==0 and j==entmenu_tiles_y(self)-1) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 0, 2);
      }
      else if (i==entmenu_tiles_x()-1 and j==0) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 2, 0);
      }
      else if (i==entmenu_tiles_x()-1 and j==entmenu_tiles_y(self)-1) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 2, 2);
      }
      else if (i==0) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 0, 1);
      }
      else if (i==entmenu_tiles_x()-1) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 2, 1);
      }
      else if (j==0) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 1, 0);
      }
      else if (j==entmenu_tiles_y(self)-1) {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 1, 2);
      }
      else {
        tilemap_plot_data( entmenu_tilemap(self), i, j, 1, 1);
      }
    }
  }
}

////////////
// events //
////////////

void entmenu_update( entmenu_t * self ) {
}

void entmenu_draw( entmenu_t * self ) {
  // draw the window
  gb_draw_tilemap( entmenu_pos_x(self), entmenu_pos_y(self), 
    entmenu_tilemap(self) );
  // draw the labels
  loop(j,entmenu_maxlines(self)) {
    gb_draw_text( entmenu_pos_x(self)+10, entmenu_pos_y(self)+10+(10*j),
      string_value(entmenu_labels(self)[j]) );
  }
  // draw the cursor
  gb_draw_colormap_sub( entmenu_pos_x(self)-10,
    entmenu_pos_y(self) + 10 + (entmenu_cursor(self)*10),
    entmenu_colormap(self), point_x(entmenu_cursor_pos(self)),
    point_y(entmenu_cursor_pos(self)), 10, 10);
  // draw the title
  gb_draw_text( entmenu_pos_x(self)+5, entmenu_pos_y(self)-5,
    string_data(entmenu_title(self)) );
}