
//
//  graph module
//    this is the drawing interface.
//    the graph module gives all of the basic drawing commands
//    that gooberengine will need to draw things to the screen.

/////////////
// globals //
/////////////

#define graph_mode_normal()   0
#define graph_mode_replace()  1
#define graph_mode_add()      2
#define graph_mode_sub()      3
#define graph_mode_rsub()     4
#define graph_mode_high()     5
#define graph_mode_rhigh()    6
#define graph_mode_low()      7
#define graph_mode_rlow()     8
#define graph_mode_avg()      9
#define graph_mode_color()    10
#define graph_mode_depth()    11
#define graph_mode_stencil()  12

#define graph_width()        400
#define graph_height()       240
#define graph_area()         (graph_width()*graph_height())
#define graph_max_depth()    1000000
#define graph_min_depth()   -1000000

#define graph_layer_base()          0
#define graph_layer_top()           9
#define graph_max_layers()         10
#define graph_max_frame_dots() 100000

#define graph_max_intensity()    1000

//////////
// type //
//////////

type() {

  // the reference to the visual + renderer
  foreign( visual_t * visual );
  
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
  // spray intensity [0-1000]
  int           intensity;
  int           intensity_old;

  // the drawing mode
  int           mode;

  //local( rect_t * clip );

  // TODO: change these into a rect object
  point_t       clip_pos;
  point_t       clip_size;

  // A link to the font in Assets
  foreign( font_t * font );
  // palette for rendering (color IDs)
  // this is a pointer to the default palette on init
  foreign( palette_t * palette );
  // default palette
  foreign( palette_t * palette_default );
  // transparency level
  byte_t        trans;

  // color data (colormap)
  foreign( colormap_t * data );
  // depth data
  local( int * data_depth );
  // current layer
  int           layer;
  // extra colormaps
  local( colormap_t ** layers );

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

#define graph_intensity(self)       (self->intensity)
#define graph_intensity_old(self)   (self->intensity_old)

//#define graph_clip(self)          (self->graph_clip)
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

#define graph_layer(self)           (self->layer)
#define graph_layers(self)          (self->layers)


/////////
// new //
/////////

void init_graph( graph_t * self, visual_t * v ) {
  graph_visual(self)          = v;

  graph_flip_x(self)          = false();
  graph_flip_y(self)          = false();
  graph_color(self)           = make_color(3,3,3);
  graph_color_cls(self)       = make_color_rgba(0,0,0,0);
  graph_depth(self)           = 0;
  graph_depth_cls(self)       = graph_max_depth();
  graph_depth_enabled(self)   = false();

  graph_intensity(self)       = 200;
  graph_intensity_old(self)   = 200;

  graph_mode(self)            = graph_mode_normal();

  graph_palette(self)         = palette();
  graph_palette_default(self) = graph_palette(self);

  point_set( graph_clip_pos(self),0,0 );
  point_set( graph_clip_size(self),400,240 );

  graph_layer(self)  = 0;
  graph_layers(self) = allocv(colormap_t*,graph_max_layers());
  loop(i,graph_max_layers()) {
    graph_layers(self)[i] = colormap(graph_width(),graph_height());
  }
  // set the graph data to layer 0.
  graph_data(self) = graph_layers(self)[0];
  //graph_data(r)         = colormap( graph_width(), graph_height() );
  //graph_data_stencil(self) = allocv( bool_t, graph_area() );
  graph_data_depth(self)   = allocv( int, graph_area() );
  loop(i,graph_area()) {
    graph_data_depth(self)[i] = graph_max_depth();
  }
}

graph_t * graph( visual_t * v ) {
  graph_t * r = alloc(graph_t);
  init_graph(r,v);
  return r;
}

void free_graph( graph_t * self ) {
  // free the clipping rect
  //free_rect( graph_clip(self) );
  // free the depth buffer
  free( graph_data_depth(self) );
  // free all of the colormaps contained in the layers
  loop(i,graph_max_layers()) {
    free_colormap(graph_layers(self)[i]);
  }
  // clear the array that held the layers
  free(graph_layers(self));
  // finally, free self
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


color_t graph_set_color( graph_t * self, color_t c ) {
  color_t r = graph_color(self);
  graph_color(self) = palette_colors(graph_palette(self))[_graph_calc_palette_index(c)];
  return r;
}

color_t graph_set_cls_color( graph_t * self, color_t c ) {
  color_t r = graph_color_cls(self);
  graph_color_cls(self) = palette_colors(graph_palette(self))[_graph_calc_palette_index(c)];
  return r;
}

int graph_set_depth( graph_t * self, int d ) {
  int r = graph_depth(self);
  graph_depth(self) = d;
  return r;
}

bool_t graph_enable_depth( graph_t * self, bool_t d ) {
  bool_t r = graph_depth_enabled(self);
  graph_depth_enabled(self) = bool(d);
  return r;
}

int graph_set_cls_depth( graph_t * self, int d ) {
  int r = graph_depth_cls(self);
  graph_depth_cls(self) = d;
  return r;
}

int graph_set_mode( graph_t * self, int m ) {
  int r = graph_mode(self);
  graph_mode(self) = m;
  return r;
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
  if eq(p,null()) {
    graph_palette(self) = graph_palette_default(self);
  }
  else {
    graph_palette(self) = p;
  }
}

// setting n to 0 is the base layer.
// 1-9 are extra layers.
void graph_set_layer( graph_t * self, int n ) {
  n = wrap(n,0,graph_max_layers());
  graph_data(self) = graph_layers(self)[n];
}

// sets the layer to the bottommost (0)
void graph_set_layer_base( graph_t * self ) {
  graph_set_layer(self,0);
}

void graph_set_font( graph_t * self, font_t * f ) {
  graph_font(self) = f;
}

int graph_set_intensity( graph_t * self, int n ) {
  int u = graph_intensity(self);
  // keep track of the old value
  graph_intensity_old(self) = graph_intensity(self);
  graph_intensity(self) = clamp(n,0,graph_max_intensity());
  return u;
}

int graph_set_intensity_max( graph_t * self ) {
  return graph_set_intensity( self, graph_max_intensity() );
}

int graph_reset_intensity( graph_t * self ) {
  // set the value to the old one.
  graph_intensity(self) = graph_intensity_old(self);
}

///////////////////////
// drawing functions //
///////////////////////

void graph_cls_color( graph_t * self ) {
  colormap_clear(graph_data(self),graph_color_cls(self));
}

void graph_cls_depth( graph_t * self ) {
  loop(i,graph_area()) { 
    graph_data_depth(self)[i] = graph_depth_cls(self);
  }
}

void graph_cls( graph_t * self ) {
  graph_cls_color(self);
  graph_cls_depth(self);
}

void graph_cls_all( graph_t * self ) {
  loop(i,graph_max_layers()) {
    graph_set_layer(self,i);
    graph_cls_color(self);
  }
  // set back to layer 0 when finished.
  graph_set_layer(self,0);
}

color_t graph_get_pixel( graph_t * self, int x, int y ) {
  colormap_get_pixel(graph_data(self), x, y);
}

void graph_plot_depth( graph_t * self, int x, int y, int d ) {
  if inrect(x,y,0,0,graph_width(),graph_height()) {
    graph_data_depth(self)[y*graph_width()+x] = d;
  }
}

int graph_get_pixel_depth( graph_t * self, int x, int y ) {
  if inrect(x,y,0,0,graph_width(),graph_height()) {
    return graph_data_depth(self)[(graph_width()*y)+x];
  }
}

// this is primarily used for certain transparent rendering.
// since there's no alpha in GB's 64 color palette, dithering
// is applied instead.
void graph_draw_dot( graph_t * self, int x, int y ) {

  if ( graph_frame_dots(self) < graph_max_frame_dots() ) {
    if inrect(x,y,graph_clip_x(self),graph_clip_y(self),graph_clip_w(self),graph_clip_h(self)) {
      // depth mode is not affected by spray intensity
      if eq(graph_mode(self), graph_mode_depth()) {
        graph_frame_dots(self) += 1;
        graph_plot_depth(self,x,y,graph_depth(self));
      }
      else if prob(graph_intensity(self)) {
        bool_t fb = true();
        if graph_depth_enabled(self) {
          fb = fb && (graph_depth(self) <= graph_get_pixel_depth(self,x,y));
          if (fb == true()) {
            graph_frame_dots(self) += 1;
            graph_plot_depth(self,x,y,graph_depth(self));
          }
        }
        if (fb == true()) {
          if ( graph_mode(self) == graph_mode_normal() ) {
            graph_frame_dots(self) += 1;
            colormap_plot( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_replace() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_replace( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_add() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_add( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_sub() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_sub( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_high() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_high( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_low() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_low( graph_data(self), x, y, graph_color(self) );
          }
          else if ( graph_mode(self) == graph_mode_avg() ) {
            graph_frame_dots(self) += 1;
            colormap_plot_avg( graph_data(self), x, y, graph_color(self) );
          }
        }
      }
    }
  }
}


void graph_draw_dot_c( graph_t * self, int x, int y, color_t c ) {
  graph_set_color(self,c);
  graph_draw_dot(self,x,y);
}

void graph_draw_hl( graph_t * self, int x, int y, int w ) {
  loop(i,w) {
    graph_draw_dot(self,x+i,y);
  }
}

void graph_draw_vl( graph_t * self, int x, int y, int h ) {
  loop(j,h) {
    graph_draw_dot(self,x,y+j);
  }
}


void graph_draw_rect( graph_t * self, int x, int y, int w, int h ) {
  loop(i,w) {
    loop(j,h) {
      graph_draw_dot(self,x+i,y+j);
    }
  }
}

void graph_draw_rect_line( graph_t * self, int x, int y, int w, int h ) {
  loop(i,w) {
    loop(j,h) {
      if ((i==0) or (j==0) or (i==w-1) or (j==h-1)) {
        graph_draw_dot(self,x+i,y+j);
      }
    }
  }
}


// this is a work in progress.
void graph_draw_circle( graph_t * self, int x, int y, int r ) {
  if (r<=0) return;
  // just draw a dot if the r is 1
  if (r==1) {
    graph_draw_dot( self, x, y );
  }
  // otherwise...
  else {
    loop2(i,-r,r) {
      loop2(j,-r,r) {
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
    loop2(i,-r,r) {
      loop2(j,-r,r) {
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
    loop(i,ct) {
      rx = x + frac(r,rounded(sine(rt + frac(i,1000,ct)),10),1000);
      ry = y + frac(r,rounded(cosine(rt + frac(i,1000,ct)),10),1000);
      graph_draw_dot(self,rx,ry);
    } 
  }
}

void graph_draw_replace( graph_t * self, int x, int y, int w, int h, color_t rs, color_t rd ) {
  loop(i,w) {
    loop(j,h) {
      color_t u = graph_get_pixel( self, x+i, y+j );
      if (check_color_eq(u,rs)) {
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
  loop(i,uu) {
    rx = x1 + (dx*i/uu);
    ry = y1 + (dy*i/uu);
    graph_draw_dot( self, rx, ry );
  }
}

void graph_draw_rect_dither_even( graph_t * self, int x, int y, int w, int h) {
  loop(j,h) {
    loop(i,w) {
      if (((i+j)&1)==0) {
        graph_draw_dot(self,x+i,y+j);
      }
    }
  }
}

void graph_draw_rect_dither_odd( graph_t * self, int x, int y, int w, int h ) {
  loop(j,h) {
    loop(i,w) {
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
  loop(i,ds) {
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
  loop(i,c->size->x) {
    loop(j,c->size->y) {
      graph_draw_dot_c(self,sx+(i*dx),sy+(j*dy),colormap_get_pixel(c,i,j));
    }
  }
}

void graph_draw_colormap_sub( graph_t * self, int dx, int dy, colormap_t * c, int sx, int sy, int sw, int sh ) {
  int ssx = graph_flip_x(self)==true() ? sw-1 : 0;
  int ssy = graph_flip_y(self)==true() ? sh-1 : 0;
  int ddx = graph_flip_x(self)==true() ? -1 : 1;
  int ddy = graph_flip_y(self)==true() ? -1 : 1;
  loop(i,sw) {
    loop(j,sh) {
      graph_draw_dot_c( self, ssx + dx + (i*ddx), ssy + dy + (j*ddy), colormap_get_pixel(c,sx+i,sy+j) );
    }
  }
}

void graph_draw_colormap_sub_ex( 
  graph_t * self, 
  int dx, int dy, int dw, int dh, 
  colormap_t * c, int sx, int sy, int sw, int sh ) 
{
  int sssx = graph_flip_x(self)==true() ? sw-1 : 0;
  int sssy = graph_flip_y(self)==true() ? sh-1 : 0;
  int dddx = graph_flip_x(self)==true() ? -1 : 1;
  int dddy = graph_flip_y(self)==true() ? -1 : 1;
  int ddx;
  int ddy;
  loop(i,dw) {
    loop(j,dh) {
      ddx = i*sw/dw;
      ddy = j*sh/dh; 
      graph_draw_dot_c( self, sssx + dx + (i*dddx), sssy + dy + (j*dddy), colormap_get_pixel(c,sx+ddx,sy+ddy) );
    }
  }
}

// points 1, 2, 3, 4 are the points of a quad in clockwise motion.
// you may move the points in any order you like to get rotation,
// flipping, scaling, etc.
void graph_draw_colormap_sub_quad (
  graph_t * self,
  int dx1, int dy1, int dx2, int dy2, int dx3, int dy3, int dx4, int dy4,
  colormap_t * c, int sx, int sy, int sw, int sh )
{
  // TODO
}

void graph_draw_text( graph_t * self, int x, int y, char * t ) {
  if graph_font(self) {
    font_t * f = graph_font(self);
    int xx = font_tile_width(graph_font(self));
    int yy = font_tile_height(graph_font(self));
    int px = font_x(graph_font(self));
    int py = font_y(graph_font(self));
    int sx;
    int sy;
    loop(i,strlen(t)) {
      font_get_pos_at(f, t[i], ref(sx), ref(sy));
      graph_draw_colormap_sub( self, x + (i*xx), y, font_colormap(f), sx, sy, xx, yy );
    }
  }
}

// this function is used if the number is already in string format
// and just want to use the number formatting stuff.
void graph_draw_number_s( graph_t * self, int x, int y, char * n,
  colormap_t * c, int cx, int cy, int tw, int th ) {
  // current character
  int u = 0;
  int l = strlen(n);
  int nn = 0;

  while (u < l) {
    if (n[u] == '-') {
      nn = 10;
    }
    else if (n[u] == ' ') {
      nn = 0;
    }
    else {
      nn = n[u] - 47;
    }
    graph_draw_colormap_sub( self, x+(tw*u), y, c, cx+(tw*nn), cy, tw, th );
    u++;
  }
}

// this function expects the number graphical data to be in the following
// manner: ' 0123456789-'
void graph_draw_number( graph_t * self, int x, int y, int n, colormap_t * c,
  int cx, int cy, int tw, int th ) {

  graph_draw_number_s( self, x, y, str(n), c, cx, cy, tw, th );
}



// sm: sine magnitude - how high/low does the text characters go?
// sv: sine velocity - how much is added to sine() after each character?
// so: sine origin - starting sine position [0-999]
/*
void graph_draw_text_sine( graph_t * self, int x, int y, char * t, int sm, int sv, int so ) {
  char u[2];
  int ux;
  int uy;
  loop(i,strlen(t)) {

    // draw the character
    graph_draw_text( self, x, y, u );
  }
}
*/

void graph_draw_layer( graph_t * self, int d ) {
  d = wrap(d,0,graph_max_layers());
  // will only operate if the current layer is higher than 0
  if (d!=0) {
    // get current layer
    int u = graph_layer(self);
    // set current layer to base (0)
    graph_set_layer(self,0);
    // draw the layer info
    graph_draw_colormap( self, 0, 0, graph_layers(self)[d] );
    // set back to current layer
    graph_set_layer(self,u);
  }
}

void graph_merge_layer( graph_t * self, int l2 ) {
  l2 = wrap(l2,0,graph_max_layers());
  // only works if the layer being merged is > 0
  if (l2 != 0) {
    // automatically set the layer to 0 when merging.
    graph_set_layer(self,0);
    // get the colormap of the layer we want
    colormap_t * col = graph_layers(self)[l2];
    // transient color variable
    color_t      c;
    // loop [0-96000)
    loop(i,graph_area()) {
      // set c to the data of the colormap of [l2]
      c = colormap_data(col)[i];
      // if a is not 0 (i.e. opaque) set the buffer to the new color.
      if (c.a == 1) colormap_data(graph_data(self))[i] = c;
    }
  }
}

// this does not allow movement of the layer when drawing.
// draw the dots in their prospective location before drawing the layer.
void graph_draw_layer_sub( graph_t * self, int d, int sx, int sy, int sw, int sh ) {
  // TODO
}

void graph_draw_mouse( graph_t * self, mouse_t * m ) {
  if mouse_visible(m) {
    int u = graph_intensity(self);
    graph_set_intensity(self,1000);

    graph_draw_colormap_sub( self, 
      (point_x(mouse_click_pos(m))/visual_window_scale(graph_visual(self)))-5, 
      (point_y(mouse_click_pos(m))/visual_window_scale(graph_visual(self)))-5, 
      mouse_colormap(m), 50 + (10*(mouse_click_time(m)/7)), 200, 10, 10 ); 

    graph_draw_colormap_sub( self, mouse_x(m)/visual_window_scale(graph_visual(self)), 
      mouse_y(m)/visual_window_scale(graph_visual(self)), mouse_colormap(m), 40, 200, 10, 10 );
    
    graph_set_intensity(self,u);
  }
}

void graph_draw_tilemap( graph_t * self, int x, int y, tilemap_t * t ) {
  point_t pt;
  loop(i,point_x(tilemap_size(t))) {
    loop(j,point_y(tilemap_size(t))) {
      tilemap_get_tile(t,i,j,ref(pt));
      graph_draw_colormap_sub( self, 
        x + (point_x(tilemap_tile_size(t))*i),
        y + (point_y(tilemap_tile_size(t))*j),
        tilemap_colormap(t), 
        point_x(tilemap_offset(t)) + (pt.x*point_x(tilemap_tile_size(t))), 
        point_y(tilemap_offset(t)) + (pt.y*point_y(tilemap_tile_size(t))),
        point_x(tilemap_tile_size(t)),
        point_y(tilemap_tile_size(t)));
    }
  }
}

void graph_draw_tilemap_sub( graph_t * self, int x, int y, int w, int h, tilemap_t * t ) {
  // TODO
}

// draws the whole screen with the cls color
// useful if you want to use the spray functionality to clear the screen instead of
// simply wiping it (but this method is slower).
void graph_draw_cls( graph_t * self ) {
  color_t u  = graph_set_color( self, graph_color_cls(self) );
  int v      = graph_set_mode( self, graph_mode_replace() );
  graph_draw_rect( self, 0, 0, 400, 240 );
  graph_set_color( self, u );
  graph_set_mode( self, v);
}

///////////////////////////////////
// functions for drawing modules // 
///////////////////////////////////

void graph_draw_sprite( graph_t * self, int x, int y, sprite_t * s ) {
  graph_set_flip( self, sprite_flip_x(s), sprite_flip_y(s) );
  graph_draw_colormap_sub( self, x-point_x(sprite_offset(s)), y-point_y(sprite_offset(s)), sprite_colormap(s), 
    rect_x(sprite_rect(s)), rect_y(sprite_rect(s)), rect_w(sprite_rect(s)), rect_h(sprite_rect(s)));
  graph_set_flip( self, false(), false() );
}

void graph_draw_sprite_tiled( graph_t * self, int x, int y, sprite_t * s, int w, int h ) {
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      graph_draw_sprite(self,x + (sprite_width(s)*i),y + (sprite_height(s)*j),s);
    }
  }
}




////////////
// events //
////////////

void graph_present( graph_t * self ) {
  // pixel format = ABGR8888
  visual_t  * vv  = graph_visual(self);
  board_t   * bb  = visual_screen(vv);
  
  board_lock( bb );
  Uint32    * px  = board_rawdata(bb);

  colormap_t * cm = graph_data(self);

  int ci  = 0;
  color_t cc;

  // who uses do/while loops, right?
  // while     = check first, do after
  // do-while  = do first, check after
  do {
    //log("%d %d",xx,yy);
    cc = colormap_data(cm)[ci];
    px[ci++] = abgr(255,get_color_b(cc)*85,get_color_g(cc)*85,get_color_r(cc)*85);
    //ci++;
  } while (ci<graph_area());
  
  board_unlock( bb );
  graph_frame_dots(self) = 0;
}