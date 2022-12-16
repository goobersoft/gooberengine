
type() {

  // reference image
  colormap_t * colormap;

  // position on image
  uint_t x;
  uint_t y;
  // amount of tiles (x and y)
  uint_t tiles_x;
  uint_t tiles_y;
  // size of tiles
  uint_t tile_w;
  uint_t tile_h;

} font_t;

#define font_colormap(self)    (self->colormap)
#define font_x(self)           (self->x)
#define font_y(self)           (self->y)
#define font_tiles_x(self)     (self->tiles_x)
#define font_tiles_y(self)     (self->tiles_y)
#define font_tile_width(self)  (self->tile_w)
#define font_tile_height(self) (self->tile_h)

/////////
// new //
/////////

void font_init( font_t * self, colormap_t * u ) {
  font_colormap(self)    = u;
  font_x(self)           = 0;
  font_y(self)           = 0;
  font_tiles_x(self)     = 0;
  font_tiles_y(self)     = 0;
  font_tile_width(self)  = 0;
  font_tile_height(self) = 0;
}

font_t * font( colormap_t * u ) {
  font_t * r = alloc(font_t);
  font_init(r,u);
  return r;
}

// del
#define font_free(self) free(self)

///////////
// funcs //
///////////

void font_set_colormap( font_t * self, colormap_t * c ) {
  font_colormap(self) = c;
}

void font_set_pos( font_t * self, int x, int y ) {
  font_x(self) = x;
  font_y(self) = y;
}

// set the size of the tile itself (pixels)
void font_set_tile_size( font_t * self, int w, int h ) {
  font_tile_width(self)  = w;
  font_tile_height(self) = h;
}

// not the same as "tile_size"
// this means "how many tiles in the x and y dimension?"
void font_set_tiles_size( font_t * self, int x, int y ) {
  font_tiles_x(self) = x;
  font_tiles_y(self) = y;
}

// returns the position of the source rectangle where the character
// is at (rx, ry).
void font_get_pos_at( font_t * self, char c, int * rx, int * ry ) {
  c=c-32;

  int ty = c / font_tiles_x(self);
  int tx = c % font_tiles_x(self);

  val(rx) = font_x(self) + (font_tile_width(self)*tx);
  val(ry) = font_y(self) + (font_tile_height(self)*ty);
}

void font_set( font_t * self, colormap_t * c, int px, int py, int sx, int sy, 
  int tx, int ty ) {
  font_set_colormap(self,c);
  font_set_pos(self,px,py);
  font_set_tile_size(self,sx,sy);
  font_set_tiles_size(self,tx,ty);
}