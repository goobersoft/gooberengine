

// how big each cell is (width and height)
#define leddisplay_cellsize() 5
// position on screen
#define leddisplay_pos_x() 10
#define leddisplay_pos_y() 50
// defines how big the LED display is
#define leddisplay_cells_x()  14
#define leddisplay_cells_y()  28
// defines the source location of the colormap
#define leddisplay_spritesrc_x() 220
#define leddisplay_spritesrc_y() 0
//
#define leddisplay_coverart_pos_x()
#define leddisplay_coverart_pos_y()
#define leddisplay_coverart_spritesrc_x() 40
#define leddisplay_coverart_spritesrc_y() 40
#define leddisplay_coverart_size_x()
#define leddisplay_coverart_size_y()
// values for each led cell.
#define leddisplay_cell_off()  0
#define leddisplay_cell_low()  1
#define leddisplay_cell_mid()  2
#define leddisplay_cell_high() 3

//////////
// type //
//////////

type() {

  // draws if active
  bool_t active;

  // the amoutn of cells is static (14 by 28)
  int cells[leddisplay_cells_x()][leddisplay_cells_y()];

  // a reference to the bricker-0 colormap
  foreign( colormap_t * colormap );

} leddisplay_t;

#define leddisplay_active(self) (self->active)
#define leddisplay_cells(self) (self->cells)
#define leddisplay_colormap(self) (self->colormap)

/////////
// new //
/////////

void init_leddisplay( leddisplay_t * self ) {
  // off by default
  leddisplay_active(self) = false();
  // set all cells to 0
  loop(i,leddisplay_cells_x()) {
    loop(j,leddisplay_cells_y()) {
      leddisplay_cells(self)[i][j] = leddisplay_cell_off();
    }
  }
  // get the bricker-0 colormap from the gb assets object
  leddisplay_colormap(self) = gb_get_colormap("bricker-0");
}

leddisplay_t * leddisplay(void) {
  leddisplay_t * self = alloc(leddisplay_t);
  init_leddisplay(self);
  return self;
}

void free_leddisplay( leddisplay_t * self ) {
  // no special allocations made; object can be totally freed
  free(self);
}

///////////////
// functions //
///////////////

void leddisplay_activate( leddisplay_t * self, bool_t b ) {
  leddisplay_active(self) = bool(b);
}

void leddisplay_clear( leddisplay_t * self ) {
  loop(i,leddisplay_cells_x()) {
    loop(j,leddisplay_cells_y()) {
      leddisplay_cells(self)[i][j] = leddisplay_cell_off();
    }
  }
}

void leddisplay_set_cell( leddisplay_t * self, int x, int y, int v ) {
  if (inrange(x,0,leddisplay_cells_x())) {
    if (inrange(y,0,leddisplay_cells_y())) {
      leddisplay_cells(self)[x][y] = clamp(v,0,3);
    }
  }
}

void leddisplay_set_cells_random( leddisplay_t * self ) {
  loop(i,leddisplay_cells_x()) {
    loop(j,leddisplay_cells_y()) {
      leddisplay_cells(self)[i][j] = rnd(0,4);
    }
  }
}

// inverts the LED intensities (0->3, 1->2, 2->1, 3->0)
// useful for showing static images but also want some sort of animation.
void leddisplay_invert( leddisplay_t * self ) {
  loop(i,leddisplay_cells_x()) {
    loop(j,leddisplay_cells_y()) {
      leddisplay_cells(self)[i][j] = 3 - leddisplay_cells(self)[i][j];
    }
  }
}

// The LEDdisplay has a static size of 14 by 28. As such, you only need
// to supply the colormap reference and the position to copy the pixels.
// The display uses the following colors for its intensity:
// 0: (0,0,0) -- black
// 1: (1,0,0) -- dark red
// 2: (2,1,0) -- dark orange
// 3: (3,2,0) -- bright orange
void leddisplay_copy_from_colormap( leddisplay_t * self, colormap_t * c,
int x, int y ) {

}

////////////
// events //
////////////

void leddisplay_update( leddisplay_t * self ) {
}

void leddisplay_draw( leddisplay_t * self ) {
  // draw each cell with 200 intensity
  gb_set_intensity(200);

  if (leddisplay_active(self)) {
  // draw all cells
    loop(i,leddisplay_cells_x()) {
      loop(j,leddisplay_cells_y()) {
        gb_draw_colormap_sub(
          leddisplay_pos_x() + (i*leddisplay_cellsize()),
          leddisplay_pos_y() + (j*leddisplay_cellsize()),
          leddisplay_colormap(self),
          leddisplay_spritesrc_x() + 
            (leddisplay_cells(self)[i][j] * leddisplay_cellsize()),
          leddisplay_spritesrc_y(),
          leddisplay_cellsize(),
          leddisplay_cellsize()
        );
      }
    }
  }
  else {
    gb_draw_colormap_sub( leddisplay_pos_x(), leddisplay_pos_y(),
      leddisplay_colormap(self), 330, 100, 70, 140 );
  }
  // reset the drawing intensity
  gb_reset_intensity();
}