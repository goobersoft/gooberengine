
/////////
// pre //
/////////

// this is used for drawing operations.
SDL_Rect  _sprite_rect_src;
SDL_Rect  _sprite_rect_dst;

SDL_Rect _sprite_ready_rect_src( int x, int y, int w, int h ) {
  _sprite_rect_src.x = x;
  _sprite_rect_src.y = y;
  _sprite_rect_src.w = w;
  _sprite_rect_src.h = h;
  return _sprite_rect_src;
}

SDL_Rect _sprite_ready_rect_dst( int x, int y, int w, int h ) {
  _sprite_rect_dst.x = x;
  _sprite_rect_dst.y = y;
  _sprite_rect_dst.w = w;
  _sprite_rect_dst.h = h;
  return _sprite_rect_dst;
}

//////////
// type //
//////////

type() {

  // position relative to screen / render target
  point_t       pos;

  // offset to account for in terms of image
  point_t       handle;
  
  // flags to indicate if the image should be flipped.
  byte_t        flip_x;
  byte_t        flip_y;

  // pointer to the source texture (tileset)
  SDL_Texture * source;

  // parameters for when copying from the source image
  int           src_x;
  int           src_y;
  int           src_w;
  int           src_h;

} sprite_t;

#define sprite_pos(self)    (&self->pos)
#define sprite_handle(self) (&self->handle)
#define sprite_flip_x(self) (self->flip_x)
#define sprite_flip_y(self) (self->flip_y)
#define sprite_source(self) (self->source)
#define sprite_src_x(self)  (self->src_x)
#define sprite_src_y(self)  (self->src_y)
#define sprite_src_w(self)  (self->src_w)
#define sprite_src_h(self)  (self->src_h)

/////////
// new //
/////////

sprite_t * sprite( SDL_Texture * t ) {
  sprite_t * r     = alloc(sprite_t);
  point_init       (sprite_pos(r),0,0);
  point_init       (sprite_handle(r),0,0);
  sprite_flip_x(r) = false();
  sprite_flip_y(r) = false();
  sprite_source(r) = t;
  sprite_src_x(r)  = 0;
  sprite_src_y(r)  = 0;
  sprite_src_w(r)  = 0;
  sprite_src_h(r)  = 0;
  return r;
}

///////////
// funcs //
///////////

SDL_Rect sprite_get_rect(sprite_t * s) {
  return _sprite_ready_rect_src(sprite_src_x(s), sprite_src_y(s), sprite_src_w(s), sprite_src_h(s));
}

void sprite_set_rect( sprite_t * s, int x, int y, int w, int h ) {
  sprite_src_x(s) = x;
  sprite_src_y(s) = y;
  sprite_src_w(s) = w;
  sprite_src_h(s) = h;
}

void sprite_set_source( sprite_t * s, SDL_Texture * t ) {
  sprite_source(s) = t;
}