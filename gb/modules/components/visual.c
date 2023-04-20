
////////////
// consts //
////////////

// 1.666 or 5:3
#define visual_aspect()           1666
#define visual_screen_width()      400
#define visual_screen_height()     240
#define visual_window_width_d()   1200
#define visual_window_height_d()   720
#define visual_window_scale_d()      3

////////////
// struct //
////////////

typedef struct {

  SDL_Window    * window;
  SDL_Renderer  * renderer;
  board_t       * screen;       // the main screen texture
  point_t         window_size;
  int             window_scale;
  bool_t          fullscreen;
  bool_t          fpsmode;
  int             fpstick;

} visual_t;

#define visual_window(self)          (self->window)
#define visual_renderer(self)        (self->renderer)
#define visual_screen(self)          (self->screen)
#define visual_window_size(self)     (&self->window_size)
#define visual_window_scale(self)    (self->window_scale)
#define visual_fullscreen(self)      (self->fullscreen)
#define visual_fpsmode(self)         (self->fpsmode)
#define visual_fpstick(self)         (self->fpstick)

/////////
// new //
/////////

void visual_init( visual_t * self ) {
  visual_window(self) = SDL_CreateWindow("gooberengine",
    SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
    visual_window_width_d(),visual_window_height_d(),0);
  visual_renderer(self) = SDL_CreateRenderer(visual_window(self),-1,
    //SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    // | SDL_RENDERER_PRESENTVSYNC
  visual_screen(self) = board(visual_renderer(self));
  point_set( visual_window_size(self), visual_window_width_d(), visual_window_height_d() );
  visual_window_scale(self) = visual_window_scale_d();
  visual_fullscreen(self)   = false();
  visual_fpsmode(self)      = 0;
}

visual_t * visual() {
  visual_t * r = alloc(visual_t);
  visual_init(r);
  return r;
}

///////////////
// functions //
///////////////

void visual_set_title( visual_t * self, char * t ) {
  SDL_SetWindowTitle( visual_window(self), t );
}

void visual_draw_scanlines( visual_t * self ) {
  SDL_Rect rr;
  
  SDL_SetRenderDrawBlendMode  ( visual_renderer(self), SDL_BLENDMODE_BLEND );
  for (int i = 2; i < point_y(visual_window_size(self)); i+=3) {
    rr.x = 0;
    rr.y = i;
    rr.w = 1200;
    rr.h = 1;
    SDL_SetRenderDrawColor      ( visual_renderer(self), 0, 0, 0, 96 );
    SDL_RenderFillRect( visual_renderer(self), ref(rr) );
    /*
    rr.y = i-2;
    SDL_SetRenderDrawColor      ( visual_renderer(self), 0, 0, 0, 48 );
    SDL_RenderFillRect( visual_renderer(self), ref(rr) );
    */
  }
}

// should the game run at 60 or 30 fps?
void visual_set_fpsmode( visual_t * self, bool_t b ) {
  visual_fpsmode(self) = bool(b);
  visual_fpstick(self) = 0;
}

////////////
// events //
////////////

void visual_update( visual_t * self ) {
  if (visual_fpsmode(self)) {
    visual_fpstick(self) = !visual_fpstick(self);
  }
}

void visual_draw_pre( visual_t * self ) {
  // clear the backbuffer for the renderer
  SDL_SetRenderDrawColor    ( visual_renderer(self), 0, 0, 0, 255 );
  SDL_RenderClear           ( visual_renderer(self) );
  // set the target back to the visual screen
  SDL_SetRenderDrawColor    ( visual_renderer(self), 255, 255, 255, 255 );
  
}


void visual_draw_post( visual_t * self ) {
  SDL_RenderCopy       ( visual_renderer(self), board_texture(visual_screen(self)), NULL, NULL);
  visual_draw_scanlines(self);
  SDL_RenderPresent    ( visual_renderer(self) );
}