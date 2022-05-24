
////////////
// struct //
////////////

typedef struct {

  point_t    *  size;
  color_t    *  data;

} colormap_t;

#define colormap_size(self) (self->size)
#define colormap_data(self) (self->data)

/////////
// new //
/////////

colormap_t * colormap( int x, int y ) {
  x = low(x,10);
  y = low(y,10);
  colormap_t * r = alloc(colormap_t);

  colormap_size(r) = point(x,y);
  colormap_data(r) = array(x*y,0);
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
      ca = rounded(ca,85)/85;
      aa = (j*sx) + i;
      if (ca<3) {
        colormap_data(r)[aa] = -1;
      }
      else {
        colormap_data(r)[aa] = color( cr, cg, cb );
      }
      
    }
  }

  return r;
}

void free_colormap( colormap_t * self ) {
  free_array( colormap_data(self) );
  free_point( colormap_size(self) );
  free(self);
}

///////////////
// functions //
///////////////

// fill the colormap with a color index. You may specify -1
// to make it transparent
void colormap_fill( colormap_t * self, color_t c ) {
  loop(i,0,point_x(colormap_size(self))*point_y(colormap_size(self))) {
    colormap_data(self)[i] = c;
  }
}

color_t colormap_get_pixel( colormap_t * self, int x, int y ) {
  if (inrect(x,y,0,0,self->size->x,self->size->y)) {
    return colormap_data(self)[y*point_x(colormap_size(self)) + x];
  }
  return -1;
}