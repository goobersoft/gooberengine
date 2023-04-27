
#define board_width()   400
#define board_height()  240
#define board_area()    board_width()*board_height()

#define board_blend_mode_normal()   0
#define board_blend_mode_add()      1
#define board_blend_mode_multiply() 2
#define board_blend_mode_opaque()   3

type() {

  foreign ( SDL_Renderer * renderer );
  local   ( SDL_Texture * texture );
  foreign ( Uint32 * rawdata );
  bool_t         locked;
  int            rawpitch;

} board_t;

#define board_size(self)       (self->size)
#define board_renderer(self)   (self->renderer)
#define board_texture(self)    (self->texture)
#define board_locked(self)     (self->locked)
#define board_rawdata(self)    (self->rawdata)
#define board_rawpitch(self)   (self->rawpitch)

//////////////
// typedefs //
//////////////


type() {

  Uint32 data[96000];

} board_rawdata_t;


/////////
// new //
/////////


void board_init( board_t * self, SDL_Renderer * r ) {
  board_renderer(self)  = r;
  board_texture(self)   = SDL_CreateTexture( r,
    SDL_PIXELFORMAT_ABGR8888,
    SDL_TEXTUREACCESS_STREAMING,
    board_width(), board_height()
  );
  board_rawdata(self)   = allocv(Uint32*,board_width()*board_height());
  board_locked(self)    = false();
}

board_t * board( SDL_Renderer * r ) {
  board_t * b   = alloc(board_t);
  board_init( b, r );
  return b;
}

/////////
// del //
/////////

void free_board( board_t * self ) {
  SDL_DestroyTexture(board_texture(self));
  free(self);
}

///////////////
// functions //
///////////////

bool_t board_lock( board_t * self ) {
  if (board_locked(self) == false()) {
    SDL_LockTexture(board_texture(self), null(), (void**)ref(board_rawdata(self)), ref(board_rawpitch(self)));
    board_locked(self) = true();
    return true();
  }
  return false();
}

bool_t board_unlock( board_t * self ) {
  if (board_locked(self) == true()) {
    SDL_UnlockTexture(board_texture(self));
    board_locked(self)   = false();
    board_rawdata(self)  = null();
    board_rawpitch(self) = 0;
    return true();
  }
  return false();
}

///////////////////////
// drawing functions //
///////////////////////

/*
void board_set_color( board_t * self, byte_t r, byte_t g, byte_t b ) {
  SDL_SetRenderDrawColor( board_renderer(self), r, g, b, 255 );
}

void board_reset_color( board_t * self ) {
  board_set_color( self, 255, 255, 255 );
}

void board_draw_dot( board_t * self, int x, int y ) {
  SDL_RenderDrawPoint( board_renderer(self), x, y );
}

void board_draw_line( board_t * self, int x1, int y1, int x2, int y2 ) {
  SDL_RenderDrawLine( board_renderer(self), x1, y1, x2, y2 );
}

void board_draw_rect( board_t * self, int x, int y, int w, int h ) {
  SDL_Rect rct;
  rct.x = x;
  rct.y = y;
  rct.w = w;
  rct.h = h;
  SDL_RenderFillRect( board_renderer(self), ref(rct) );
}

void board_draw_rect_outline( board_t * self, int x, int y, int w, int h ) {
  SDL_Rect rct;
  rct.x = x;
  rct.y = y;
  rct.w = w;
  rct.h = h;
  SDL_RenderDrawRect( board_renderer(self), ref(rct) );
}

void board_draw_hline( board_t * self, int x, int y, int w ) {
  board_draw_rect( self, x, y, w, 1 );
}

void board_draw_vline( board_t * self, int x, int y, int h ) {
  board_draw_rect( self, x, y, 1, h );
}

void board_set_blend_mode( board_t * self, int b ) {
  if (b == board_blend_mode_normal()) {
    SDL_SetRenderDrawBlendMode( board_renderer(self), SDL_BLENDMODE_BLEND );
  }
  else if (b == board_blend_mode_add()) {
    SDL_SetRenderDrawBlendMode( board_renderer(self), SDL_BLENDMODE_ADD );
  }
  else if (b == board_blend_mode_multiply()) {
    SDL_SetRenderDrawBlendMode( board_renderer(self), SDL_BLENDMODE_MOD );
  }
  else if (b == board_blend_mode_opaque()) {
    SDL_SetRenderDrawBlendMode( board_renderer(self), SDL_BLENDMODE_NONE );
  }
}
*/