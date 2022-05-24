
////////////
// consts //
////////////

// 1.666 or 5:3
#define visual_aspect()          1666
#define visual_screen_width()     400
#define visual_screen_height()    240
#define visual_width_d()         1200
#define visual_height_d()         720

////////////
// struct //
////////////

typedef struct {

  SDL_Window    * window;
  SDL_Renderer  * renderer;
  board_t       * screen;
  board_t       * target;
  point_t         size;
  int             scale;
  SDL_Rect        rect;

} visual_t;

#define visual_window(self)   (self->window)
#define visual_renderer(self) (self->renderer)
#define visual_target(self)   (self->target)
#define visual_screen(self)   (self->screen)
#define visual_size(self)     (&self->size)
#define visual_width(self)    (self->size.w)
#define visual_height(self)   (self->size.h)
#define visual_scale(self)    (self->scale)
#define visual_rect(self)     (&self->rect)

/////////
// new //
/////////

visual_t * visual() {
  visual_t * r = alloc(visual_t);
  
  visual_window(r) = SDL_CreateWindow("gooberengine",
    SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
    1200,720,0);

  visual_renderer(r) = SDL_CreateRenderer(visual_window(r),-1,
    //SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

  visual_screen(r) = board(visual_screen_width(), visual_screen_height(), visual_renderer(r));

  point_set( visual_size(r), visual_width_d(), visual_height_d() );

  visual_scale(r) = 3;
  return r;
}

/////////
// del //
/////////

void free_visual(visual_t * v) {
  //free_point            (get(v,size));
  SDL_DestroyRenderer   (visual_renderer(v));
  SDL_DestroyWindow     (visual_window(v));
  free(v);
}

///////////
// funcs //
///////////

void visual_set_target( visual_t * self, board_t * b ) {
  if (b == NULL) {
    SDL_SetRenderTarget( visual_renderer(self), board_texture(visual_screen(self)) );
  }
  else {
    SDL_SetRenderTarget( visual_renderer(self), board_source(b) );
    board_renderer(b) = visual_renderer(self);
  }
}

////////////
// events //
////////////

void visual_draw_pre( visual_t * self ) {
  // clear the backbuffer for the renderer
  SDL_SetRenderTarget       ( visual_renderer(self), NULL);
  SDL_SetRenderDrawColor    ( visual_renderer(self), 0, 0, 0, 255 );
  SDL_RenderClear           ( visual_renderer(self) );
  // set the target back to the visual screen
  SDL_SetRenderTarget       ( visual_renderer(self), board_texture(visual_screen(self)));
  SDL_SetRenderDrawColor    ( visual_renderer(self), 255, 255, 255, 255 );
  
}

void visual_draw( visual_t * self ) {
  SDL_SetRenderTarget  ( visual_renderer(self), NULL );
  SDL_RenderCopy       ( visual_renderer(self), board_texture(visual_screen(self)), NULL, NULL);
  SDL_RenderPresent    ( visual_renderer(self) );
}