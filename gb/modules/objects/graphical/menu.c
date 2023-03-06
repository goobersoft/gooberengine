/*
  Menu Entity
*/

#define entity_menu_tiles_x() 12

type() {
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

} entity_menu_t;

// getters
#define entity_menu_entity(self)       (self->entity)

#define entity_menu_menu(self)         (self->menu)
#define entity_menu_tilemap(self)      (self->tilemap)
#define entity_menu_window_pos(self)   (self->window_pos)
#define entity_menu_cursor_pos(self)   (self->cursor_pos)
#define entity_menu_title(self)        (self->title)
// helpers
#define entity_menu_labels(self)       menu_labels(entity_menu_menu(self))
#define entity_menu_actions(self)      menu_actions(entity_menu_menu(self))
#define entity_menu_maxlines(self)     menu_maxlines(entity_menu_menu(self))
#define entity_menu_tiles_y(self)      tilemap_size_y(entity_menu_tilemap(self))
#define entity_menu_colormap(self)     tilemap_colormap(entity_menu_tilemap(self))
#define entity_menu_cursor(self)       menu_cursor(entity_menu_menu(self))

#define entity_menu_cursor_down(self)  menu_cursor_down(entity_menu_menu(self))
#define entity_menu_cursor_up(self)    menu_cursor_up(entity_menu_menu(self))
#define entity_menu_select(self)       menu_select(entity_menu_menu(self))


/////////
// new //
/////////

void init_entity_menu( entity_menu_t * self, int n ) {
  entity_menu_entity(self) = entity("menu",self);

  colormap_t * c        = gb_get_colormap("gb-0");
  // make a new menu
  entity_menu_menu(self)    = menu(n);
  // make a new tilemap 12 by 20 tiles (10px)
  entity_menu_tilemap(self) = tilemap( entity_menu_tiles_x(), n+2, c );

  entity_menu_window_pos(self) = point(200,150);
  entity_menu_cursor_pos(self) = point(290,120);

  // gb-0 20,15
  tilemap_set_offset( entity_menu_tilemap(self), 
    point_x(entity_menu_window_pos(self)),
    point_y(entity_menu_window_pos(self))
  );

  entity_menu_title(self) = string(100);
  string_copy(entity_menu_title(self),"Untitled");
}

entity_menu_t * entity_menu( int n ) {
  entity_menu_t * self = alloc(entity_menu_t);
  init_entity_menu(self,n);
  return self;
}

void free_entity_menu( entity_menu_t * self ) {

  free_entity   ( entity_menu_entity(self) );
  free_menu     ( entity_menu_menu(self) );
  free_tilemap  ( entity_menu_tilemap(self) );
  free_point    ( entity_menu_window_pos(self) );
  free_point    ( entity_menu_cursor_pos(self) );
  free_string   ( entity_menu_title(self) );
  free          ( self );
  
}

///////////
// funcs //
///////////

void entity_menu_set_colormap( entity_menu_t * self, colormap_t * c ) {
  entity_menu_colormap(self) = c;
}

void entity_menu_set_label( entity_menu_t * self, int c, char * s ) {
  c = wrap(c,0, entity_menu_maxlines(self) );
  string_copy( entity_menu_labels(self)[c], s );
}

void entity_menu_set_title( entity_menu_t * self, char * s ) {
  string_copy( entity_menu_title(self), s );
}

void entity_menu_set_window_pos( entity_menu_t * self, int x, int y ) {
  point_set( entity_menu_window_pos(self), x, y );
  tilemap_set_offset( entity_menu_tilemap(self), 
    point_x(entity_menu_window_pos(self)),
    point_y(entity_menu_window_pos(self))
  );
}

void entity_menu_set_cursor_pos( entity_menu_t * self, int x, int y ) {
  point_set( entity_menu_cursor_pos(self), x, y );
}

void entity_menu_decorate_tilemap( entity_menu_t * self ) {
  loop(i,entity_menu_tiles_x()) {
    loop(j,entity_menu_tiles_y(self)) {
      if (i==0 and j==0) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 0, 0);
      }
      else if (i==0 and j==entity_menu_tiles_y(self)-1) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 0, 2);
      }
      else if (i==entity_menu_tiles_x()-1 and j==0) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 2, 0);
      }
      else if (i==entity_menu_tiles_x()-1 and j==entity_menu_tiles_y(self)-1) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 2, 2);
      }
      else if (i==0) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 0, 1);
      }
      else if (i==entity_menu_tiles_x()-1) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 2, 1);
      }
      else if (j==0) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 1, 0);
      }
      else if (j==entity_menu_tiles_y(self)-1) {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 1, 2);
      }
      else {
        tilemap_plot_data( entity_menu_tilemap(self), i, j, 1, 1);
      }
    }
  }
}

////////////
// events //
////////////

void entity_menu_update( entity_menu_t * self ) {
}

void entity_menu_draw( entity_menu_t * self ) {
  entity_t * base = entity_menu_entity(self);
  // draw the window
  gb_draw_tilemap( entity_pos_x(base), entity_pos_y(base), 
    entity_menu_tilemap(self) );
  // draw the labels
  loop(j,entity_menu_maxlines(self)) {
    gb_draw_text( entity_pos_x(base)+10,
      entity_pos_y(base)+10+(10*j),
      string_value(entity_menu_labels(self)[j]) );
  }
  // draw the cursor
  gb_draw_colormap_sub( entity_pos_x(base)-10,
    entity_pos_y(base) + 10 + (entity_menu_cursor(self)*10),
    entity_menu_colormap(self), point_x(entity_menu_cursor_pos(self)),
    point_y(entity_menu_cursor_pos(self)), 10, 10);
  // draw the title
  gb_draw_text( entity_pos_x(base)+5, entity_pos_y(base)-5,
    string_data(entity_menu_title(self)) );
}