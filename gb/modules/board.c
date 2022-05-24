
typedef struct {

  local    point_t      * size;
  local    SDL_Texture  * texture;
  external SDL_Renderer * source;

} board_t;

#define board_size(self)       (self->size)
#define board_source(self)     (self->texture)
#define board_texture(self)    board_source(self)
#define board_renderer(self)   (self->source)

/////////
// new //
/////////

board_t * board(int x, int y, SDL_Renderer * r) {
  x = low(x,10);
  y = low(y,10);
  board_t * b   = alloc(board_t);
  b->size       = point(x,y);
  b->texture    = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_TARGET, x, y);
  b->source     = r;
  return b;
}

// the dunder will help differentiate it from functions.
// this is supposed to mean a new board with "full" resolution.
// (400x240)
board_t * board__full( SDL_Renderer * r ) {
  return board(400,240,r);
}

/////////
// del //
/////////

void free_board( board_t * self ) {
  SDL_DestroyTexture(board_source(self));
  free_point(board_size(self));
  free(self);
}

///////////////
// functions //
///////////////

