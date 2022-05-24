
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
  // A link to the font in Assets
  font_t     * font;
  // palette for rendering (color IDs)
  palette_t    palette;
  // transparency level
  byte_t       trans;

  // internal rectangles used for drawing
  SDL_Rect  rect_src;
  SDL_Rect  rect_dst;

} graph_t;

#define graph_visual(self)     (self->visual)
#define graph_renderer(self)   (self->renderer)
#define graph_color(self)      (self->color)
#define graph_color_cls(self)  (self->color_cls)
#define graph_font(self)       (self->font)
#define graph_palette(self)    (self->palette)
#define graph_trans(self)      (self->trans)
#define graph_rect_src(self)   (&self->rect_src)
#define graph_rect_dst(self)   (&self->rect_dst)

#define graph_texture_width()  400
#define graph_texture_height() 240


/////////
// new //
/////////

graph_t * graph( visual_t * v ) {
  graph_t * r = alloc(graph_t);
  
  graph_visual(r)    = v;
  graph_renderer(r)  = visual_renderer(v);

  graph_color(r)      = color(3,3,3);
  graph_color_cls(r)  = color(0,0,0);

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
  SDL_SetRenderDrawColor( graph_renderer(self),
    85*color_r(graph_color_cls(self)),
    85*color_g(graph_color_cls(self)),
    85*color_b(graph_color_cls(self)),
    255 );
  graph_set_dst_rect( self, 0, 0, visual_screen_width(), visual_screen_height() );
  SDL_RenderFillRect(graph_renderer(self),graph_rect_dst(self));
}

// this is primarily used for certain transparent rendering.
// since there's no alpha in GB's 64 color palette, dithering
// is applied instead.
void graph_draw_dot( graph_t * self, int x, int y ) {
  SDL_RenderDrawPoint(graph_renderer(self), x, y);
}

void graph_draw_dot_c( graph_t * self, int x, int y, color_t c ) {
  graph_set_color(self,c);
  graph_draw_dot(self,x,y);
}

void graph_draw_hl( graph_t * self, int x, int y, int w ) {
  SDL_RenderDrawLine(graph_renderer(self), x, y, x+w, y);
}

void graph_draw_vl( graph_t * self, int x, int y, int h ) {
  SDL_RenderDrawLine(graph_renderer(self), x, y, x, y+h);
}


void graph_draw_rect( graph_t * self, int x, int y, int w, int h ) {
  graph_set_dst_rect(self,x,y,w,h);
  SDL_RenderFillRect(graph_renderer(self),graph_rect_dst(self));
}

void graph_draw_rect_line( graph_t * self, int x, int y, int w, int h ) {
  graph_set_dst_rect(self,x,y,w,h);
  SDL_RenderDrawRect(graph_renderer(self),graph_rect_dst(self));
}


// this is a work in progress.
void graph_draw_circle( graph_t * self, int x, int y, int r ) {
  if (r<=0) return;
  // just draw a dot if the r is 1
  if (r == 1) {
    graph_draw_dot( self, x, y );
  }
  // draw a small diamind (5 pixels) if r is 2
  else if (r == 2) {
    graph_draw_dot( self, x, y-1 );
    graph_draw_hl ( self, x-1, y, 3);
    graph_draw_dot( self, x, y+1 );
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
  // draw a small diamind (5 pixels) if r is 2
  else if (r == 2) {
    graph_draw_dot( self, x, y-1 );
    graph_draw_hl ( self, x-1, y, 3);
    graph_draw_dot( self, x, y+1 );
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
  point_t * pp = mouse_pos(m);
  point_t * mp = mouse_image_pos(m);
  point_t * ms = mouse_image_size(m);
  graph_draw_image_sub( self, mouse_image(m),
    point_x(pp)/visual_scale(graph_visual(self)), 
    point_y(pp)/visual_scale(graph_visual(self)), point_x(mp), point_y(mp), point_x(ms), point_y(ms) ); 
}

void graph_draw_colormap( graph_t * self, colormap_t * c, int x, int y ) {
  loop(i,0,c->size->x) {
    loop(j,0,c->size->y) {
      graph_draw_dot_c(self,x+i,y+j,colormap_get_pixel(c,i,j));
    }
  }
}

void graph_draw_colormap_sub( graph_t * self, colormap_t * c, int dx, int dy, int sx, int sy, int sw, int sh ) {

}