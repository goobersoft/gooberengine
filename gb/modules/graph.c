
//
//  graph module
//    this is the drawing interface.
//    the graph module gives all of the basic drawing commands
//    that gooberengine will need to draw things to the screen.

/////////////
// globals //
/////////////

byte_t _graph_transvals[] = {
  //bin(_0001)
  /*
  bin(_1111_1111_1111_1111),
  bin(_1110_1011_1110_1011),
  bin(_1010_0101_1010_0101),
  bin(_0001_0100_0001_0100),
  bin(_0000_0000_0000_0000)
  */
};

#define graph_mode_normal()   0
#define graph_mode_replace()  1
#define graph_mode_add()      2
#define graph_mode_sub()      3
#define graph_mode_high()     4
#define graph_mode_low()      5
#define graph_mode_avg()      6
#define graph_mode_depth()    7
#define graph_mode_stencil()  8

#define graph_width()        400
#define graph_height()       240
#define graph_area()         graph_width()*graph_height()
#define graph_depth_cls_d()  1000000

#define graph_max_layers()         10
#define graph_max_frame_dots() 100000

//////////
// type //
//////////

type() {

  // the reference to the visual + renderer
  visual_t      * visual;
  
  // the number of dots which were processed in the frame.
  uint_t        frame_dots;

  // the draw color
  color_t       color;
  // the cls color
  color_t       color_cls;
  // flip x/y
  bool_t        flip_x;
  bool_t        flip_y;
  // current depth
  int           depth;
  // depth enable flag
  bool_t        depth_enabled;
  // depth clear
  int           depth_cls;
  // current stencil
  bool_t        stencil;
  // 
  bool_t        stencil_enabled;
  // stencil cls
  bool_t        stencil_cls;
  // the drawing mode
  int           mode;

  point_t       clip_pos;
  point_t       clip_size;

  // A link to the font in Assets
  font_t      * font;
  // palette for rendering (color IDs)
  // this is a pointer to the default palette on init
  palette_t   * palette;
  // default palette
  palette_t   * palette_default;
  // transparency level
  byte_t        trans;

  // color data (colormap)
  colormap_t  * data;
  // depth data
  int         * data_depth;
  // stencil data
  bool_t      * data_stencil;
  // current layer
  int           layer;
  // extra colormaps
  colormap_t ** layers;

  // internal rectangles used for drawing
  SDL_Rect  rect_src;
  SDL_Rect  rect_dst;

} graph_t;

#define graph_visual(self)        (self->visual)
#define graph_renderer(self)      (self->renderer)

#define graph_flip_x(self)          (self->flip_x)
#define graph_flip_y(self)          (self->flip_y)

#define graph_frame_dots(self)      (self->frame_dots)

#define graph_color(self)           (self->color)
#define graph_color_cls(self)       (self->color_cls)

#define graph_depth(self)           (self->depth)
#define graph_depth_cls(self)       (self->depth_cls)
#define graph_depth_enabled(self)   (self->depth_enabled)

#define graph_stencil(self)         (self->stencil)
#define graph_stencil_cls(self)     (self->stencil_cls)
#define graph_stencil_enabled(self) (self->stencil_enabled)

#define graph_clip_pos(self)        (&self->clip_pos)
#define graph_clip_size(self)       (&self->clip_size)  
#define graph_clip_x(self)          (self->clip_pos.x)
#define graph_clip_y(self)          (self->clip_pos.y)
#define graph_clip_w(self)          (self->clip_size.x)
#define graph_clip_h(self)          (self->clip_size.y)

#define graph_mode(self)            (self->mode)
#define graph_font(self)            (self->font)

#define graph_palette(self)         (self->palette)
#define graph_palette_default(self) (self->palette_default)

#define graph_trans(self)           (self->trans)

#define graph_data(self)            (self->data)
#define graph_data_depth(self)      (self->data_depth)
#define graph_data_stencil(self)    (self->data_stencil)

#define graph_layer(self)           (self->layer)
#define graph_layers(self)          (self->layers)


/////////
// new //
/////////

graph_t * graph( visual_t * v ) {
  graph_t * r = alloc(graph_t);
  
  graph_visual(r)        = v;

  graph_flip_x(r)        = false();
  graph_flip_y(r)        = false();
  graph_color(r)         = color(3,3,3);
  graph_color_cls(r)     = color(0,0,0);
  graph_depth(r)         = 0;
  graph_depth_enabled(r) = true();
  graph_depth_cls(r)     = graph_depth_cls_d();
  graph_stencil(r)       = true();
  graph_stencil_cls(r)   = false();
  graph_mode(r)          = graph_mode_normal();

  graph_palette(r)         = palette();
  graph_palette_default(r) = graph_palette(r);

  point_set( graph_clip_pos(r),0,0);
  point_set( graph_clip_size(r),400,240); 

  graph_layer(r)  = 0;
  graph_layers(r) = allocv(colormap_t*,graph_max_layers());
  loop(i,0,graph_max_layers()) {
    graph_layers(r)[i] = colormap(graph_width(),graph_height());
  }
  // set the graph data to layer 0.
  graph_data(r) = graph_layers(r)[0];
  //graph_data(r)         = colormap( graph_width(), graph_height() );
  graph_data_stencil(r) = allocv( bool_t, graph_area() );
  graph_data_depth(r)   = allocv( int,    graph_area() );
  loop(i,0,graph_area()) {
    graph_data_depth(r)[i] = graph_depth_cls_d();
  }

  return r;
}

//////////
// free //
//////////

void free_graph(graph_t * self) {
  free(self);
}

///////////////
// functions //
///////////////

int _graph_calc_palette_index( color_t c ) {
  if (c.a) {
    return (c.b*16) + (c.g*4) + c.r;
  }
  return palette_index_trans();
}

void graph_set_flip( graph_t * self, bool_t x, bool_t y ) {
  graph_flip_x(self) = x;
  graph_flip_y(self) = y;
}

void graph_set_color( graph_t * self, color_t c ) {
  graph_color(self) = palette_colors(graph_palette(self))[_graph_calc_palette_index(c)];
}

void graph_set_cls_color( graph_t * self, color_t c ) {
  graph_color_cls(self) = palette_colors(graph_palette(self))[_graph_calc_palette_index(c)];
}

void graph_set_depth( graph_t * self, int d ) {
  graph_depth(self) = d;
}

void graph_enable_depth( graph_t * self, bool_t d ) {
  graph_depth_enabled(self) = bool(d);
}

void graph_set_cls_depth( graph_t * self, int d ) {
  graph_depth_cls(self) = d;
}

void graph_set_stencil( graph_t * self, bool_t s ) {
  graph_stencil(self) = bool(s);
}

void graph_enable_stencil( graph_t * self, bool_t s ) {
  graph_stencil_enabled(self) = bool(s);
}

void graph_set_cls_stencil( graph_t * self, bool_t s ) {
  graph_stencil_cls(self) = bool(s);
}

void graph_set_mode( graph_t * self, int m ) {
  graph_mode(self) = m;
}

void graph_set_clip( graph_t * self, int x, int y, int w, int h ) {
  point_set( graph_clip_pos(self), x, y );
  point_set( graph_clip_size(self), w, h );
}

void graph_reset_clip( graph_t * self ) {
  graph_set_clip(self,0,0,400,240);
}

void graph_set_trans( graph_t * self, int t ) {
  graph_trans(self) = clamp(t,0,4);
}

void graph_set_palette( graph_t * self, palette_t * p ) {
  if (p == null()) {
    graph_palette(self) = graph_palette_default(self);
  }
  else {
    graph_palette(self) = p;
  }
}

// setting n to -1 will be the base colormap
// setting 0 to 9 will be the extra colormaps
void graph_set_layer( graph_t * self, int n ) {
  n = wrap(n,0,graph_max_layers());
  graph_data(self) = graph_layers(self)[n];
}

void graph_reset_layer( graph_t * self ) {
  graph_set_layer(self,0);
}

///////////////////////
// drawing functions //
///////////////////////

void graph_cls( graph_t * self ) {
  static int _i = false();
  // clear the color data
  colormap_clear(graph_data(self),graph_color_cls(self));
  loop(i,0,graph_area()) {
    // clear the depth buffer
    graph_data_depth(self)[i]    = graph_depth_cls(self);
    // clear the stencil buffer
    graph_data_stencil(self)[i]  = graph_stencil_cls(self);
  }
}

color_t graph_get_pixel( graph_t * self, int x, int y ) {
  colormap_get_pixel(graph_data(self), x, y);
}

void graph_plot_depth( graph_t * self, int x, int y, int d ) {
  if (inrect(x,y,0,0,graph_width(),graph_height())) {
    graph_data_depth(self)[y*graph_width()+x] = d;
  }
}

void graph_plot_stencil( graph_t * self, int x, int y, bool_t s ) { 
  if (inrect(x,y,0,0,graph_width(),graph_height())) {
    graph_data_stencil(self)[y*graph_width()+x] = bool(s);
  }
}

// true if opaque (cannot draw), false if transparent (can draw)
bool_t graph_get_pixel_stencil( graph_t * self, int x, int y ) {
  if (inrect(x,y,0,0,graph_width(),graph_height())) {
    return graph_data_stencil(self)[(graph_width()*y)+x];
  }
}

int graph_get_pixel_depth( graph_t * self, int x, int y ) {
  if (inrect(x,y,0,0,graph_width(),graph_height())) {
    return graph_data_depth(self)[(graph_width()*y)+x];
  }
}

// this is primarily used for certain transparent rendering.
// since there's no alpha in GB's 64 color palette, dithering
// is applied instead.
void graph_draw_dot( graph_t * self, int x, int y ) {
  if (graph_frame_dots(self) < graph_max_frame_dots()) {
    if (inrect(x,y,graph_clip_x(self),graph_clip_y(self),graph_clip_w(self),graph_clip_h(self))) {
      graph_frame_dots(self) += 1;

      int gm = graph_mode(self);

      if (gm==graph_mode_normal()) {
        colormap_plot( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }
      else if (gm==graph_mode_replace()) {
        colormap_plot_replace( graph_data(self), x, y, graph_color(self) );
      }
      else if (gm==graph_mode_add()) {
        colormap_plot_add( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot_add( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }
      else if (gm==graph_mode_sub()) {
        colormap_plot_sub( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot_sub( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }
      else if (gm==graph_mode_high()) {
        colormap_plot_high( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot_high( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }
      else if (gm==graph_mode_low()) {
        colormap_plot_low( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot_low( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }
      else if (gm==graph_mode_avg()) {
        colormap_plot_avg( graph_data(self), x, y, graph_color(self) );
        /*
        if (graph_get_pixel_stencil(self,x,y)==false()) {
          if (graph_depth(self) <= graph_get_pixel_depth(self,x,y)) {
            colormap_plot_avg( graph_data(self), x, y, graph_color(self) );
            if (graph_depth_enabled(self)==true()) {
              graph_plot_depth(self,x,y,graph_depth(self));
            }
          }
        }
        */
      }

      else if (gm==graph_mode_depth()) {
        if (graph_depth(self) < graph_get_pixel_depth(self,x,y)) {
          graph_plot_depth(self,x,y,graph_depth(self));
        }
      }

      else if (gm==graph_mode_stencil()) {
        graph_plot_stencil(self,x,y,graph_stencil(self));
      }
    }
  //SDL_RenderDrawPoint(graph_renderer(self), x, y);
  }
}





void graph_draw_dot_c( graph_t * self, int x, int y, color_t c ) {
  if (color_a(c)==1) {
    graph_set_color(self,c);
    graph_draw_dot(self,x,y);
  }
}

void graph_draw_hl( graph_t * self, int x, int y, int w ) {
  loop(i,0,w) {
    graph_draw_dot(self,x+i,y);
  }
  //SDL_RenderDrawLine(graph_renderer(self), x, y, x+w, y);
}

void graph_draw_vl( graph_t * self, int x, int y, int h ) {
  loop(j,0,h) {
    graph_draw_dot(self,x,y+j);
  }
  //SDL_RenderDrawLine(graph_renderer(self), x, y, x, y+h);
}


void graph_draw_rect( graph_t * self, int x, int y, int w, int h ) {
  loop(i,0,w) {
    loop(j,0,h) {
      graph_draw_dot(self,x+i,y+j);
    }
  }
  //graph_set_dst_rect(self,x,y,w,h);
  //SDL_RenderFillRect(graph_renderer(self),graph_rect_dst(self));
}

void graph_draw_rect_line( graph_t * self, int x, int y, int w, int h ) {
  loop(i,0,w) {
    loop(j,0,h) {
      if ((i==0) or (j==0) or (i==w-1) or (j==h-1)) {
        graph_draw_dot(self,x+i,y+j);
      }
    }
  }
  
  //graph_set_dst_rect(self,x,y,w,h);
  //SDL_RenderDrawRect(graph_renderer(self),graph_rect_dst(self));
}


// this is a work in progress.
void graph_draw_circle( graph_t * self, int x, int y, int r ) {
  if (r<=0) return;
  // just draw a dot if the r is 1
  if (r == 1) {
    graph_draw_dot( self, x, y );
  }
  // otherwise...
  else {
    loop(i,-r,r) {
      loop(j,-r,r) {
        if (sqroot(sqr(i)+sqr(j)) < r) {
          graph_draw_dot(self, x+i, y+j);
        }
      }
    }
  }
}

void graph_draw_circle_line( graph_t * self, int x, int y, int r ) {
  if (r<=0) return;
  // just draw a dot if the r is 1
  if (r == 1) {
    graph_draw_dot( self, x, y );
  }
  // otherwise...
  else {
    int uu = 0;
    loop(i,-r,r) {
      loop(j,-r,r) {
        uu = sqroot(sqr(i)+sqr(j)); 
        if ((uu < r) and (uu >= r-1)) {
          graph_draw_dot(self, x+i, y+j);
        }
      }
    }
  }
}

void graph_draw_circle_dots( graph_t * self, int x, int y, int r, int ct, int rt ) {
  if (r<=0) return;
  if (r==1) {
    graph_draw_dot( self, x, y );
  }
  else {
    int rx = 0;
    int ry = 0;
    loop(i,0,ct) {
      rx = x + frac(r,rounded(sine(rt + frac(i,1000,ct)),10),1000);
      ry = y + frac(r,rounded(cosine(rt + frac(i,1000,ct)),10),1000);
      graph_draw_dot(self,rx,ry);
    } 
  }
}

void graph_draw_circle_spray( graph_t * self, int x, int y, int r, int p ) {
  if (r<=0) return;
  // just draw a dot if the r is 1
  if (r == 1) {
    graph_draw_dot( self, x, y );
  }
  // otherwise...
  else {
    loop(i,-r,r) {
      loop(j,-r,r) {
        if (sqroot(sqr(i)+sqr(j)) < r) {
          if prob(p) graph_draw_dot(self, x+i, y+j);
        }
      }
    }
  }
}

void graph_draw_rect_spray( graph_t * self, int x, int y, int w, int h, int p ) {
  loop(i,0,w) {
    loop(j,0,h) {
      if (prob(p)) {
        graph_draw_dot( self, x+i, y+j );
      }
    }
  }
}

void graph_draw_replace( graph_t * self, int x, int y, int w, int h, color_t rs, color_t rd ) {
  loop(i,0,w) {
    loop(j,0,h) {
      color_t u = graph_get_pixel( self, x+i, y+j );
      if (color_eq(u,rs)) {
        graph_draw_dot_c(self,x+i,y+j,rd);
      }
    }
  }
}

void graph_draw_line( graph_t * self, int x1, int y1, int x2, int y2 ) {
  int dx = x2-x1;
  int dy = y2-y1;
  int uu = sqroot( sqr(dx) + sqr(dy) );
  int rx = 0;
  int ry = 0;
  loop(i,0,uu) {
    rx = x1 + (dx*i/uu);
    ry = y1 + (dy*i/uu);
    graph_draw_dot( self, rx, ry );
  }
}

/*
void graph_draw_image( graph_t * self, image_t * u, int x, int y ) {
  graph_set_src_rect(self, 0, 0, image_width(u), image_height(u));
  graph_set_dst_rect(self, x, y, image_width(u), image_height(u));
  SDL_RenderCopy( graph_renderer(self), image_texture(u), graph_rect_src(self), graph_rect_dst(self) );
}

void graph_draw_image_sub( graph_t * self, image_t * u, int dx, int dy, int sx, int sy, int sw, int sh ) {
  graph_set_src_rect(self, sx, sy, sw, sh);
  graph_set_dst_rect(self, dx, dy, sw, sh);
  SDL_RenderCopy( graph_renderer(self), image_texture(u), graph_rect_src(self), graph_rect_dst(self) );
}
*/

void graph_draw_text( graph_t * self, int x, int y, char * t ) {
  
}

void graph_draw_rect_dither_even( graph_t * self, int x, int y, int w, int h) {
  loop(j,0,h) {
    loop(i,0,w) {
      if (((i+j)&1)==0) {
        graph_draw_dot(self,x+i,y+j);
      }
    }
  }
}

void graph_draw_rect_dither_odd( graph_t * self, int x, int y, int w, int h ) {
  loop(j,0,h) {
    loop(i,0,w) {
      if (((i+j)&1)==1) {
        graph_draw_dot(self,x+i,y+j);
      }
    }
  }
}

void graph_draw_rect_dither( graph_t * self, int x, int y, int w, int h, bool_t o ) {
  if (o>0) {
    graph_draw_rect_dither_odd(self,x,y,w,h);
  }
  else {
    graph_draw_rect_dither_even(self,x,y,w,h);
  }
}

void graph_draw_triangle( graph_t * self, int x1, int y1, int x2, int y2, int x3, int y3 ) {
  
  // I need to fix this
  int dx = x2-x1;
  int dy = y2-y1;
  int ds = sqroot(sqr(dx+1)+sqr(dy+1));
  //log("%d",ds);
  int ddx;
  int ddy;
  loop(i,0,ds) {
    ddx = x1 + (dx*i/ds);
    ddy = y1 + (dy*i/ds);
    graph_draw_line( self, ddx, ddy, x3, y3 );
  }
}

void graph_draw_triangle_line( graph_t * self, int x1, int y1, int x2, int y2, int x3, int y3 ) {
  graph_draw_line( self, x1, y1, x2, y2 );
  graph_draw_line( self, x2, y2, x3, y3 );
  graph_draw_line( self, x3, y3, x1, y1 );
}


///////////////////////////////////
// specific module drawing stuff //
///////////////////////////////////

// TODO: flipping code.


void graph_draw_colormap( graph_t * self, int x, int y, colormap_t * c ) {
  int sx = graph_flip_x(self)==true() ? c->size->x-1 : 0;
  int sy = graph_flip_y(self)==true() ? c->size->y-1 : 0;
  int dx = graph_flip_x(self)==true() ? -1 : 1;
  int dy = graph_flip_y(self)==true() ? -1 : 1;
  loop(i,0,c->size->x) {
    loop(j,0,c->size->y) {
      graph_draw_dot_c(self,sx+(i*dx),sy+(j*dy),colormap_get_pixel(c,i,j));
    }
  }
}

void graph_draw_colormap_sub( graph_t * self, int dx, int dy, colormap_t * c, int sx, int sy, int sw, int sh ) {
  int ssx = graph_flip_x(self)==true() ? sw-1 : 0;
  int ssy = graph_flip_y(self)==true() ? sh-1 : 0;
  int ddx = graph_flip_x(self)==true() ? -1 : 1;
  int ddy = graph_flip_y(self)==true() ? -1 : 1;
  loop(i,0,sw) {
    loop(j,0,sh) {
      graph_draw_dot_c( self, ssx + dx + (i*ddx), ssy + dy + (j*ddy), colormap_get_pixel(c,sx+i,sy+j) );
    }
  }
}

void graph_draw_colormap_sub_ex( graph_t * self, int dx, int dy, int dw, int dh, colormap_t * c, int sx, int sy, int sw, int sh ) {
  int sssx = graph_flip_x(self)==true() ? sw-1 : 0;
  int sssy = graph_flip_y(self)==true() ? sh-1 : 0;
  int dddx = graph_flip_x(self)==true() ? -1 : 1;
  int dddy = graph_flip_y(self)==true() ? -1 : 1;
  int ddx;
  int ddy;
  loop(i,0,dw) {
    loop(j,0,dh) {
      ddx = i*sw/dw;
      ddy = j*sw/dh; 
      graph_draw_dot_c( self, sssx + dx + (i*dddx), sssy + dy + (j*dddy), colormap_get_pixel(c,sx+ddx,sy+ddy) );
    }
  }
}

// points 1, 2, 3, 4 are the points of a quad in clockwise motion.
// you may move the points in any order you like to get rotation,
// flipping, scaling, etc.
void graph_draw_colormap_sub_quad( graph_t * self,
  int dx1, int dy1, int dx2, int dy2, int dx3, int dy3, int dx4, int dy4,
  colormap_t * c, int sx, int sy, int sw, int sh) {


}


void graph_draw_layer( graph_t * self, int d ) {
  d = wrap(d,0,graph_max_layers());
  if (d!=0) {
    int u = graph_layer(self);
    graph_set_layer(self,0);
    graph_draw_colormap( self, 0, 0, graph_layers(self)[d] );
    graph_set_layer(self,u);
  }
}

void graph_draw_mouse( graph_t * self, mouse_t * m ) {
  /*
  if (mouse_visible(m)==true()) {
    graph_set_layer(self,-1);
    graph_set_mode(self,graph_mode_replace());
    graph_set_flip(self,false(),false());
    graph_set_color(self,color_trans());
    graph_draw_rect_spray(self,0,0,400,240,10);
    graph_draw_colormap_sub( self, mouse_x(m)/3, mouse_y(m)/3, mouse_colormap(m), 40, 200, 10, 10 );
    graph_draw_layer(self,-1);
    graph_reset_layer(self);
  }
  */
}

////////////
// events //
////////////

void graph_present( graph_t * self ) {
  // pixel format = ABGR8888
  visual_t  * vv  = graph_visual(self);
  board_t   * bb  = visual_screen(vv);
  
  board_lock( bb );
  int         bp  = board_rawpitch(bb);
  Uint32    * px  = board_rawdata(bb);

  colormap_t * cm = graph_data(self);

  int xx  = 0;
  int yy  = 0;
  int ci  = 0;
  color_t cc;

  // who uses do/while loops, right?
  // while     = check first, do after
  // do-while  = do first, check after
  do {
    //log("%d %d",xx,yy);
    cc = colormap_data(cm)[ci];
    px[ci] = abgr(255,color_b(cc)*85,color_g(cc)*85,color_r(cc)*85);
    ci++;
    xx += 1;
    if (xx==400) {
      xx  = 0;
      yy += 1;
    }
  } while (xx<400 && yy<240);
  board_unlock( bb );
  graph_frame_dots(self) = 0;
}