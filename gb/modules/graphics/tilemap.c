
type() {

  // a pointer to a colormap.
  // keep in mind that a tilemap can only one colormap.
  // in order to use graphics from a separate colormap,
  // you will need to swap the colormap after drawing a
  // portion of the tilemap (see the graph module).
  foreign( colormap_t * colormap );
  // the size in tiles
  local( point_t * size );
  // the size of the tile itself
  local( point_t * tile_size );
  // a pixel offset determining the topleft of the tilemap's
  // origin.
  local( point_t * offset );
  // data holds 
  local( point_t * data );

} tilemap_t;

// getters
#define tilemap_colormap(self)  (self->colormap)
#define tilemap_size(self)      (self->size)
#define tilemap_tile_size(self) (self->tile_size)
#define tilemap_offset(self)    (self->offset)
#define tilemap_data(self)      (self->data)
// helpers
#define tilemap_size_x(self)        point_x(tilemap_size(self))
#define tilemap_size_y(self)        point_y(tilemap_size(self))
#define tilemap_tile_size_x(self)   point_x(tilemap_tile_size(self))
#define tilemap_tile_size_y(self)   point_y(tilemap_tile_size(self))
#define tilemap_offset_x(self)      point_x(tilemap_offset(self))
#define tilemap_offset_y(self)      point_y(tilemap_offset(self))

/////////
// new //
/////////

void init_tilemap( tilemap_t * self, int w, int h, colormap_t * c ) {
  // c may be null()
  tilemap_colormap(self)  = c;

  // you cannot change the size of a tilemap once it is created
  tilemap_size(self)      = point(w,h);
  // you can however change the size of the tile after creation.
  tilemap_tile_size(self) = point(10,10);
  tilemap_offset(self)    = point(0,0);
  // the tilemap's data can be altered after creation.
  tilemap_data(self)      = allocv(point_t,w*h);

}

tilemap_t * tilemap( int w, int h, colormap_t * c ) {
  tilemap_t * self = alloc(tilemap_t);
  init_tilemap(self,w,h,c);
  return self;
}

void free_tilemap( tilemap_t * self ) {
  free_point  ( tilemap_size(self) );
  free_point  ( tilemap_tile_size(self) );
  free_point  ( tilemap_offset(self) );
  free        ( tilemap_data(self) );
  free        ( self );
}

///////////
// funcs //
///////////

void tilemap_set_colormap( tilemap_t * self, colormap_t * c ) {
  tilemap_colormap(self) = c;
}

void tilemap_set_offset( tilemap_t * self, int x, int y ) {
  point_set( tilemap_offset(self), x, y );
}

void tilemap_set_tile_size( tilemap_t * self, int w, int h ) {
  point_set( tilemap_tile_size(self), w, h );
}

// this function will return the tile x and y into point r.
void tilemap_get_tile( tilemap_t * self, int x, int y, point_t * r ) {
  int c = (y*point_x(tilemap_size(self))) + x;
  point_copy( r, ref(tilemap_data(self)[c]) ); 
}

////////////////////
// plotting funcs //
////////////////////

void tilemap_plot_data( tilemap_t * self, int tx, int ty, int nx, int ny ) {
  if inrect(tx,ty,0,0,point_x(tilemap_size(self)),point_y(tilemap_size(self))) {
    int c = (point_x(tilemap_size(self))*ty) + tx;
    point_set( ref(tilemap_data(self)[c]), nx, ny );
  }
}

void tilemap_rect_data( tilemap_t * self, int tx, int ty, int tw, int th, int nx, int ny ) {
  loop(i,tw) {
    loop(j,th) {
      tilemap_plot_data(self, tx+i, ty+j, nx, ny);
    }
  }
}

