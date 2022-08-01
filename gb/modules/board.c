
#define board_width()   400
#define board_height()  240

typedef struct {

  SDL_Renderer * renderer;
  SDL_Texture  * texture;
  SDL_Texture  * rawtexture;
  bool_t         locked;
  Uint32       * rawdata;
  int            rawpitch;

} board_t;

#define board_size(self)       (self->size)
#define board_renderer(self)   (self->renderer)
#define board_texture(self)    (self->texture)
#define board_locked(self)     (self->locked)
#define board_rawdata(self)    (self->rawdata)
#define board_rawpitch(self)   (self->rawpitch)

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
