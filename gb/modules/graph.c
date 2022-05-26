
//
//  graph module
//    this is the drawing interface.
//    the graph module gives all of the basic drawing commands
//    that gooberengine will need to draw things to the screen.

/////////////
// globals //
/////////////

byte_t _graph_transvals[] = {
  bin(_1111_1111_1111_1111),
  bin(_1110_1011_1110_1011),
  bin(_1010_0101_1010_0101),
  bin(_0001_0100_0001_0100),
  bin(_0000_0000_0000_0000)
};

#define graph_mode_normal()  0
#define graph_mode_add()     1
#define graph_mode_sub()     2
#define graph_mode_mask()    3

//////////
// type //
//////////

type() {

  // the reference to the visual + renderer
  visual_t      * visual;
  SDL_Renderer  * renderer;
  
  // the draw color
  color_t      color;
  // the cls color
  color_t      color_cls;
  // the drawing mode
  int          mode;

  point_t      clip_pos;
  point_t      clip_size;

  // A link to the font in Assets
  font_t     * font;
  // palette for rendering (color IDs)
  palette_t    palette;
  // transparency level
  byte_t       trans;

  // color data (colormap)
  colormap_t * data;

  // internal rectangles used for drawing
  SDL_Rect  rect_src;
  SDL_Rect  rect_dst;

} graph_t;

#define graph_visual(self)     (self->visual)
#define graph_renderer(self)   (self->renderer)
#define graph_color(self)      (self->color)
#define graph_color_cls(self)  (self->color_cls)
#define graph_clip_pos(self)   (&self->clip_pos)
#define graph_clip_size(self)  (&self->clip_size)  
#define graph_clip_x(self)     (self->clip_pos.x)
#define graph_clip_y(self)     (self->clip_pos.y)
#define graph_clip_w(self)     (self->clip_size.x)
#define graph_clip_h(self)     (self->clip_size.y)
#define graph_mode(self)       (self->mode)
#define graph_font(self)       (self->font)
#define graph_palette(self)    (self->palette)
#define graph_trans(self)      (self->trans)
#define graph_rect_src(self)   (&self->rect_src)
#define graph_rect_dst(self)   (&self->rect_dst)
#define graph_data(self)       (self->data)

#define graph_width(self)  colormap_width(graph_data(self))
#define graph_height(self) colormap_height(graph_data(self))


/////////
// new //
/////////

graph_t * graph( visual_t * v ) {
  graph_t * r = alloc(graph_t);
  
  graph_visual(r)    = v;
  graph_renderer(r)  = visual_renderer(v);

  graph_color(r)      = color(3,3,3);
  graph_color_cls(r)  = color(0,0,0);
  graph_mode(r)       = graph_mode_normal();

  point_set( graph_clip_pos(r),0,0);
  point_set( graph_clip_size(r),400,240); 

  graph_data(r)       = colormap( visual_screen_width(), visual_screen_height() );

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

void graph_set_color( graph_t * self, color_t c ) {
  graph_color(self) = c;
  int rr, gg, bb;
  rr = color_r(c)*85;
  gg = color_g(c)*85;
  bb = color_b(c)*85;
  //log("%d %d %d" nl(), rr, gg, bb);
  //SDL_Delay(250);
  SDL_SetRenderDrawColor( graph_renderer(self), rr, gg, bb, 255 );
}

void graph_set_cls_color( graph_t * self, color_t c ) {
  graph_color_cls(self) = c;
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

void graph_set_src_rect( graph_t * self, int x, int y, int w, int h ) {
  graph_rect_src(self)->x = x;
  graph_rect_src(self)->y = y;
  graph_rect_src(self)->w = w;
  graph_rect_src(self)->h = h;
}

void graph_set_dst_rect( graph_t * self, int x, int y, int w, int h ) {
  graph_rect_dst(self)->x = x;
  graph_rect_dst(self)->y = y;
  graph_rect_dst(self)->w = w;
  graph_rect_dst(self)->h = h;
}

void graph_set_trans( graph_t * self, int t ) {
  graph_trans(self) = clamp(t,0,4);
}

///////////////////////
// drawing functions //
///////////////////////

void graph_cls( graph_t * self ) {
  colormap_fill(graph_data(self),graph_color_cls(self));
  /*
  SDL_SetRenderDrawColor( graph_renderer(self),
    85*color_r(graph_color_cls(self)),
    85*color_g(graph_color_cls(self)),
    85*color_b(graph_color_cls(self)),
    255 );
  
  graph_set_dst_rect( self, 0, 0, visual_screen_width(), visual_screen_height() );
  SDL_RenderFillRect(graph_renderer(self),graph_rect_dst(self));
  */
}

color_t graph_get_pixel( graph_t * self, int x, int y ) {
  colormap_get_pixel(graph_data(self), x, y);
}

// this is primarily used for certain transparent rendering.
// since there's no alpha in GB's 64 color palette, dithering
// is applied instead.
void graph_draw_dot( graph_t * self, int x, int y ) {
  if (inrect(x,y,graph_clip_x(self),graph_clip_y(self),graph_clip_w(self),graph_clip_h(self))) {
    switch(graph_mode(self)) {
      case graph_mode_normal(): colormap_plot( graph_data(self), x, y, graph_color(self) );      break;
      case graph_mode_add():    colormap_plot_add( graph_data(self), x, y, graph_color(self) );  break;
      case graph_mode_sub():    colormap_plot_sub( graph_data(self), x, y, graph_color(self) );  break;
      case graph_mode_mask():   colormap_plot_mask( graph_data(self), x, y, graph_color(self) ); break;
    }
  }
  //SDL_RenderDrawPoint(graph_renderer(self), x, y);
}



void graph_draw_dot_c( graph_t * self, int x, int y, color_t c ) {
  if (c>=0) {
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
      rx = x + frac(r,1000,rounded(sine(rt + frac(i,ct,1000)),10));
      ry = y + frac(r,1000,rounded(cosine(rt + frac(i,ct,1000)),10));
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
      if (u==rs) {
        graph_draw_dot_c(self,x+i,y+j,rd);
      }
    }
  }
}

void graph_draw_line( graph_t * self, int x1, int y1, int x2, int y2 ) {
  int dx = x2-x1;
  int dy = y2-y1;
  int uu = sqroot( sqr(dx) + sqr(dy) );
  
  int vv = 0;
  int rx = 0;
  int ry = 0;
  loop(i,0,uu) {
    vv = rounded(frac(i,uu,1000),1000);
    rx = x1 + (dx*i/uu);
    ry = y1 + (dy*i/uu);
    graph_draw_dot( self, rx, ry );
  }
}

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

void graph_draw_text( graph_t * self, int x, int y, char * t ) {
  
}



///////////////////////////////////
// specific module drawing stuff //
///////////////////////////////////

void graph_draw_mouse( graph_t * self, mouse_t * m ) {
  if (mouse_visible(m)) {
    point_t * pp = mouse_pos(m);
    point_t * mp = mouse_image_pos(m);
    point_t * ms = mouse_image_size(m);
    graph_draw_image_sub( self, mouse_image(m),
      point_x(pp)/visual_scale(graph_visual(self)), 
      point_y(pp)/visual_scale(graph_visual(self)), point_x(mp), point_y(mp), point_x(ms), point_y(ms) ); 
  }
}

void graph_draw_colormap( graph_t * self, int x, int y, colormap_t * c ) {
  loop(i,0,c->size->x) {
    loop(j,0,c->size->y) {
      graph_draw_dot_c(self,x+i,y+j,colormap_get_pixel(c,i,j));
    }
  }
}

void graph_draw_colormap_sub( graph_t * self, colormap_t * c, int dx, int dy, int sx, int sy, int sw, int sh ) {
  loop(i,0,sw) {
    loop(j,0,sh) {
      graph_draw_dot_c( self, dx+i, dy+j, colormap_get_pixel(c,sx+i,sy+j) );
    }
  }
}

////////////
// events //
////////////

void graph_present( graph_t * self ) {
  color_t c;
  loop(i,0,graph_width(self)) {
    loop(j,0,graph_height(self)) {
      c = colormap_get_pixel(graph_data(self),i,j);
      SDL_SetRenderDrawColor(graph_renderer(self),color_r(c)*85, color_g(c)*85, color_b(c)*85, 255);
      SDL_RenderDrawPoint(graph_renderer(self),i,j);
    }
  }
}