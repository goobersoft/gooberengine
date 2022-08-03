
////////////
// struct //
////////////

typedef struct {

  point_t    *  size;
  color_t    *  data;
  int           area;

} colormap_t;

#define colormap_size(self)   (self->size)
#define colormap_width(self)  (self->size->x)
#define colormap_height(self) (self->size->y)
#define colormap_data(self)   (self->data)
#define colormap_area(self)   (self->area)

/////////
// new //
/////////

void colormap_init( colormap_t * self, int x, int y ) {
  colormap_size(self) = point(x,y);
  colormap_data(self) = allocv(color_t,x*y);
  colormap_area(self) = x*y;
  loop(i,0,colormap_area(self)) {
    colormap_data(self)[i] = make_color(0,0,0);
  }
}

colormap_t * colormap( int x, int y ) {
  x = low(x,10);
  y = low(y,10);
  colormap_t * r = alloc(colormap_t);
  colormap_init( r, x, y );
  return r;
}

colormap_t * colormap_from_image( image_t * u ) {
  colormap_t * r = colormap( point_x(image_size(u)), point_y(image_size(u)) );

  int sx = point_x(colormap_size(r));
  int sy = point_y(colormap_size(r));
  
  color_t cc;
  byte_t  cr;
  byte_t  cg;
  byte_t  cb;
  byte_t  ca;
  int     aa;
  loop(i,0,sx) {
    loop(j,0,sy) {
      image_get_pixel( u, i, j, ref(cr), ref(cg), ref(cb), ref(ca) );
      cr = rounded(cr,85)/85;
      cg = rounded(cg,85)/85;
      cb = rounded(cb,85)/85;
      ca = (ca >= 128);

      aa = (j*sx) + i;
      if (ca==0) {
        colormap_data(r)[aa] = make_color_trans();
      }
      else {
        colormap_data(r)[aa] = make_color( cr, cg, cb );
      }
      
    }
  }

  return r;
}

void free_colormap( colormap_t * self ) {
  free( colormap_data(self) );
  free( colormap_size(self) );
  free( self );
}

///////////////
// functions //
///////////////

void colormap_plot( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    if (c.a) {
      int uu = y*point_x(colormap_size(self)) + x;
      colormap_data(self)[uu] = c;
    }
  }
}

void colormap_plot_replace( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = c;
  }
}

void colormap_plot_add( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_add(colormap_data(self)[uu],c);
  }
}

void colormap_plot_sub( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_sub(colormap_data(self)[uu],c);
  }
}

void colormap_plot_high( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_high(colormap_data(self)[uu],c);
  }
}

void colormap_plot_low( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_low(colormap_data(self)[uu],c);
  }
}

void colormap_plot_avg( colormap_t * self, int x, int y, color_t c ) {
  if (inrect(x,y,0,0,colormap_width(self),colormap_height(self))) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_avg(colormap_data(self)[uu],c);
  }
}

// fill the colormap with a color index. You may specify -1
// to make it transparent
void colormap_clear( colormap_t * self, color_t c ) {
  loop(i,0,colormap_area(self)) {
    colormap_data(self)[i] = c;
  }
}

color_t colormap_get_pixel( colormap_t * self, int x, int y ) {
  if (inrect(x,y,0,0,self->size->x,self->size->y)) {
    return colormap_data(self)[y*point_x(colormap_size(self)) + x];
  }
  return make_color_trans();
}