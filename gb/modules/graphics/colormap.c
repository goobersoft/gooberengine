
////////////
// struct //
////////////

typedef struct {

  point_t    *  size;
  color_t    *  data;
  int           area;
  point_t    *  draw_offset;

} colormap_t;

#define colormap_size(self)        (self->size)
#define colormap_width(self)       (self->size->x)
#define colormap_height(self)      (self->size->y)
#define colormap_data(self)        (self->data)
#define colormap_area(self)        (self->area)
#define colormap_draw_offset(self) (self->draw_offset)

//////////////
// typedefs //
//////////////

/////////
// new //
/////////

void colormap_init( colormap_t * self, int x, int y ) {
  colormap_size(self)          = point(x,y);
  colormap_draw_offset(self)   = point(0,0);
  colormap_data(self)          = allocv(color_t,x*y);
  colormap_area(self)          = x*y;
  loop(i,colormap_area(self)) {
    colormap_data(self)[i] = make_color_rgba(0,0,0,0);
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
  loop(i,sx) {
    loop(j,sy) {
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

colormap_t * colormap_from_base64( int w, int h, char * s ) {
  colormap_t * r   = alloc(colormap_t);
  colormap_size(r) = point(w,h);
  colormap_area(r) = w * h;
  colormap_data(r) = allocv(color_t,w*h);
  loop(i,colormap_area(r)) {
    colormap_data(r)[i] = make_color_index(int_from_base64(s,i));
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

void colormap_set_draw_offset( colormap_t * self, int x, int y ) {
  point_set( colormap_draw_offset(self),
    wrap(x,0,colormap_width(self)),
    wrap(y,0,colormap_height(self)) );
}

void colormap_plot( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    if (c.a) {
      int uu = y*point_x(colormap_size(self)) + x;
      colormap_data(self)[uu] = c;
    }
  }
}

void colormap_plot_replace( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = c;
  }
}

void colormap_plot_add( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_add(colormap_data(self)[uu],c);
  }
}

void colormap_plot_sub( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_sub(colormap_data(self)[uu],c);
  }
}

void colormap_plot_high( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_high(colormap_data(self)[uu],c);
  }
}

void colormap_plot_low( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_low(colormap_data(self)[uu],c);
  }
}

void colormap_plot_avg( colormap_t * self, int x, int y, color_t c ) {
  if inrect(x,y,0,0,colormap_width(self),colormap_height(self)) {
    int uu = y*point_x(colormap_size(self)) + x;
    colormap_data(self)[uu] = make_color_avg(colormap_data(self)[uu],c);
  }
}

// fill the colormap with a color index. You may specify -1
// to make it transparent
void colormap_clear( colormap_t * self, color_t c ) {
  loop(i,colormap_area(self)) {
    colormap_data(self)[i] = c;
  }
}

color_t colormap_get_pixel( colormap_t * self, int x, int y ) {
  if inrect(x,y,0,0,self->size->x,self->size->y) {
    return colormap_data(self)[y*point_x(colormap_size(self)) + x];
  }
  return make_color_trans();
}


string_t * colormap_to_base64( colormap_t * self ) {
  string_t * r = string( colormap_area(self) );


  loop(i,colormap_area(self)) {
    string_put(r,i,
      _base64str[ wrap( color_to_index( colormap_data(self)[i] ),0,65)]
    );
  }

  return r;
}