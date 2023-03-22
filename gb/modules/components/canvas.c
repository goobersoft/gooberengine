
type() {

  bool_t      active;
  bool_t      clsmode;

  color_t     color;
  color_t     color_cls;
  mint_t      rotation;
  mint_t      scale;
  SDL_Rect    rect_dst;
  SDL_Rect    rect_src;

  visual_t * visual;
  board_t  * board;

} canvas_t;

#define canvas_active(self)     (self->active)
#define canvas_clsmode(self)    (self->clsmode)
#define canvas_color(self)      (self->color)
#define canvas_color_cls(self)  (self->color_cls)
#define canvas_rotation(self)   (self->rotation)
#define canvas_scale(self)      (self->scale)
#define canvas_rect_dst(self)   (self->rect_dst)
#define canvas_rect_src(self)   (self->rect_src)
#define canvas_visual(self)     (self->visual)
#define canvas_board(self)      (self->board)

#define canvas_renderer(self)   visual_renderer(canvas_visual(self))
#define canvas_texture(self)    board_texture(canvas_board(self))

/////////
// new //
/////////


void init_canvas( canvas_t * self, visual_t * v ) {
  canvas_active(self)     = true();
  canvas_clsmode(self)    = true();
  canvas_color(self)      = make_color(3,3,3);
  canvas_color_cls(self)  = make_color(0,0,0);
  canvas_rotation(self)   = 0;
  canvas_scale(self)      = 1000;

  SDL_Rect r;
  canvas_rect_dst(self)   = r;
  canvas_rect_src(self)   = r;

  canvas_visual(self)     = v;
  canvas_board(self)      = board(visual_renderer(v));
}

canvas_t * canvas( visual_t * v ) {
  canvas_t * self = alloc(canvas_t);
  init_canvas(self,v);
  return self;
}

void free_canvas( canvas_t * self ) {
  free_board(canvas_board(self));
  free(self);
}

///////////
// funcs //
///////////

void canvas_activate( canvas_t * self, bool_t b ) {
  canvas_active(self) = bool(b);
}

void canvas_set_clsmode( canvas_t * self, bool_t b ) {
  canvas_clsmode(self) = bool(b);
}

void canvas_set_color( canvas_t * self, color_t c ) {
  canvas_color(self) = c;
}

void canvas_set_color_cls( canvas_t * self, color_t c ) {
  canvas_color_cls(self) = c;
}

void canvas_set_rect_dst( canvas_t * self, int x, int y, int w, int h ) {
  SDL_Rect s;
  s.x = x;
  s.y = y;
  s.w = w;
  s.h = h;
  canvas_rect_dst(self) = s;
}

void canvas_set_rect_src( canvas_t * self, int x, int y, int w, int h ) {
  SDL_Rect s;
  s.x = x;
  s.y = y;
  s.w = w;
  s.h = h;
  canvas_rect_src(self) = s;
}

void canvas_set_rotation( canvas_t * self, mint_t r ) {
  canvas_rotation(self) = wrap(r,0,1000);
}

void canvas_set_scale( canvas_t * self, mint_t s ) {
  canvas_scale(self) = s;
}

///////////////////////
// drawing functions //
///////////////////////

void canvas_draw_dot( canvas_t * self, int x, int y ) {
}

void canvas_draw_line( canvas_t * self, int x1, int y1, int x2, int y2 ) {
}

void canvas_draw_triangle( canvas_t * self, int x1, int y1, int x2, int y2, int x3, int y3 ) {
}

void canvas_draw_triangle_outline( canvas_t * self, int x1, int y1, int x2, int y2, int x3, int y3 ) {
}

void canvas_draw_rect( canvas_t * self, int x, int y, int w, int h ) {
}

void canvas_draw_rect_outline( canvas_t * self, int x, int y, int w, int h ) {
}

void canvas_draw_sprite( canvas_t * self, int x, int y, sprite_t * sp ) {
}

void canvas_draw_tilemap( canvas_t * self, int x, int y, tilemap_t * tl ) {
}

////////////
// events //
////////////

void canvas_update( canvas_t * self ) {

}

void canvas_draw_pre( canvas_t * self ) {
  SDL_SetRenderTarget( canvas_renderer(self), canvas_texture(self) );
  if (canvas_clsmode(self)) {
    SDL_SetRenderDrawColor( canvas_renderer(self),
      canvas_color_cls(self).r*85, canvas_color_cls(self).g*85, 
      canvas_color_cls(self).b*85, 255 );
    SDL_RenderClear( canvas_renderer(self) );
  }
  SDL_SetRenderTarget( canvas_renderer(self), null() );
}

void canvas_draw_post( canvas_t * self ) {
  /*
  SDL_SetRenderDrawColor( canvas_renderer(self),
    canvas_color(self).r*85, canvas_color(self).g*85, 
    canvas_color(self).b*85, 255 );
  SDL_RenderPresent( canvas_renderer(self) );
  */
}