/*
  Menu Entity
*/

#define ent_menu_tiles_x() 12

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

} ent_menu_t;

// getters
#define ent_menu_entity(self)       (self->entity)
#define ent_menu_menu(self)         (self->menu)
#define ent_menu_tilemap(self)      (self->tilemap)
#define ent_menu_window_pos(self)   (self->window_pos)
#define ent_menu_cursor_pos(self)   (self->cursor_pos)
#define ent_menu_title(self)        (self->title)
// helpers
#define ent_menu_pos(self)            entity_pos(ent_menu_entity(self))
#define ent_menu_pos_x(self)          entity_pos_x(ent_menu_entity(self))
#define ent_menu_pos_y(self)          entity_pos_y(ent_menu_entity(self))
#define ent_menu_labels(self)         menu_labels(ent_menu_menu(self))
#define ent_menu_actions(self)        menu_actions(ent_menu_menu(self))
#define ent_menu_maxlines(self)       menu_maxlines(ent_menu_menu(self))
#define ent_menu_tiles_y(self)        tilemap_size_y(ent_menu_tilemap(self))
#define ent_menu_colormap(self)       tilemap_colormap(ent_menu_tilemap(self))
#define ent_menu_cursor(self)         menu_cursor(ent_menu_menu(self))


/////////
// new //
/////////

void init_ent_menu( ent_menu_t * self, int n ) {
  colormap_t * c = gb_get_colormap("gb-0");

  // make a new entity.
  ent_menu_entity(self)  = entity(self);
  // make a new menu
  ent_menu_menu(self)    = menu(n);
  // make a new tilemap 12 by 20 tiles (10px)
  ent_menu_tilemap(self) = tilemap( ent_menu_tiles_x(), n+2, c );

  ent_menu_window_pos(self) = point(200,150);
  ent_menu_cursor_pos(self) = point(290,120);

  // gb-0 20,15
  tilemap_set_offset( ent_menu_tilemap(self), 
    point_x(ent_menu_window_pos(self)),
    point_y(ent_menu_window_pos(self))
  );

  ent_menu_title(self) = string(100);
  string_copy(ent_menu_title(self),"Untitled");
  
}

ent_menu_t * ent_menu( int n ) {
  ent_menu_t * self = alloc(ent_menu_t);
  init_ent_menu(self,n);
  return self;
}

void free_ent_menu( ent_menu_t * self ) {
  free_entity   ( ent_menu_entity(self) );
  free_menu     ( ent_menu_menu(self) );
  free_tilemap  ( ent_menu_tilemap(self) );
  free_point    ( ent_menu_window_pos(self) );
  free_point    ( ent_menu_cursor_pos(self) );
  free_string   ( ent_menu_title(self) );
  free          ( self );
}

///////////
// funcs //
///////////

void ent_menu_set_colormap( ent_menu_t * self, colormap_t * c ) {
  ent_menu_colormap(self) = c;
}

void ent_menu_set_label( ent_menu_t * self, int c, char * s ) {
  c = wrap(c,0, ent_menu_maxlines(self) );
  string_copy( ent_menu_labels(self)[c], s );
}

void ent_menu_set_title( ent_menu_t * self, char * s ) {
  string_copy( ent_menu_title(self), s );
}

void ent_menu_set_window_pos( ent_menu_t * self, int x, int y ) {
  point_set( ent_menu_window_pos(self), x, y );
  tilemap_set_offset( ent_menu_tilemap(self), 
    point_x(ent_menu_window_pos(self)),
    point_y(ent_menu_window_pos(self))
  );
}

void ent_menu_set_cursor_pos( ent_menu_t * self, int x, int y ) {
  point_set( ent_menu_cursor_pos(self), x, y );
}

void ent_menu_decorate_tilemap( ent_menu_t * self ) {
  
  loop(i,ent_menu_tiles_x()) {
    loop(j,ent_menu_tiles_y(self)) {
      if (i==0 and j==0) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 0, 0);
      }
      else if (i==0 and j==ent_menu_tiles_y(self)-1) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 0, 2);
      }
      else if (i==ent_menu_tiles_x()-1 and j==0) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 2, 0);
      }
      else if (i==ent_menu_tiles_x()-1 and j==ent_menu_tiles_y(self)-1) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 2, 2);
      }
      else if (i==0) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 0, 1);
      }
      else if (i==ent_menu_tiles_x()-1) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 2, 1);
      }
      else if (j==0) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 1, 0);
      }
      else if (j==ent_menu_tiles_y(self)-1) {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 1, 2);
      }
      else {
        tilemap_plot_data( ent_menu_tilemap(self), i, j, 1, 1);
      }
    }
  }
}

////////////
// events //
////////////

void ent_menu_update( ent_menu_t * self ) {
}

void ent_menu_draw( ent_menu_t * self ) {
  gb_draw_tilemap( ent_menu_pos_x(self), ent_menu_pos_y(self), 
    ent_menu_tilemap(self) );
  loop(j,ent_menu_maxlines(self)) {
    gb_draw_text( ent_menu_pos_x(self)+10, ent_menu_pos_y(self)+10+(10*j),
      string_value(ent_menu_labels(self)[j]) );
  }
  gb_draw_colormap_sub( ent_menu_pos_x(self)-5,
    ent_menu_pos_y(self) + 10 + (ent_menu_cursor(self)*10),
    ent_menu_colormap(self), point_x(ent_menu_cursor_pos(self)),
    point_y(ent_menu_cursor_pos(self)), 10, 10);
  gb_draw_text( ent_menu_pos_x(self)+5, ent_menu_pos_y(self)-5,
    string_data(ent_menu_title(self)) );
}