
////////////
// consts //
////////////

// 1.666 or 5:3
#define visual_aspect()           1666
#define visual_screen_width()      400
#define visual_screen_height()     240
#define visual_window_width_d()   1200
#define visual_window_height_d()   720

////////////
// struct //
////////////

typedef struct {

  SDL_Window    * window;
  SDL_Renderer  * renderer;
  board_t       * screen;       // the main screen texture
  point_t         window_size;
  int             window_scale;

} visual_t;

#define visual_window(self)          (self->window)
#define visual_renderer(self)        (self->renderer)
#define visual_screen(self)          (self->screen)
#define visual_window_size(self)     (&self->window_size)
#define visual_window_scale(self)    (self->window_scale)

/////////
// new //
/////////

visual_t * visual() {
  visual_t * r = alloc(visual_t);
  
  visual_window(r) = SDL_CreateWindow("gooberengine",
    SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
    visual_window_width_d(),visual_window_height_d(),0);

  visual_renderer(r) = SDL_CreateRenderer(visual_window(r),-1,
    //SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

  visual_screen(r) = board(visual_renderer(r));

  point_set( visual_window_size(r), visual_window_width_d(), visual_window_height_d() );

  visual_window_scale(r) = 3;
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

////////////
// events //
////////////

void visual_draw_pre( visual_t * self ) {
  // clear the backbuffer for the renderer
  SDL_SetRenderDrawColor    ( visual_renderer(self), 0, 0, 0, 255 );
  SDL_RenderClear           ( visual_renderer(self) );
  // set the target back to the visual screen
  SDL_SetRenderDrawColor    ( visual_renderer(self), 255, 255, 255, 255 );
  
}

void visual_draw( visual_t * self ) {
  SDL_RenderCopy       ( visual_renderer(self), board_texture(visual_screen(self)), NULL, NULL);
  SDL_RenderPresent    ( visual_renderer(self) );
}