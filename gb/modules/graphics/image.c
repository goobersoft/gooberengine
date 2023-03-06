
//////////
// type //
//////////

type() {

  local( SDL_Surface * surface );  
  local( point_t * size );
  // technically this would not need to be freed because it is a
  // string constant.
  foreign( char * path );

} image_t;

#define image_surface(self)  (self->surface)
#define image_size(self)     (self->size)
#define image_path(self)     (self->path)
#define image_width(self)    point_x(image_size(self))
#define image_height(self)   point_y(image_size(self))
#define image_data(self)     (self->data)


/////////
// new //
/////////

void free_image( image_t * self ) {
  if image_surface(self) {
    SDL_FreeSurface(image_surface(self));
  }

}

void init_image( image_t * self, char * f ) {
  image_size(self) = point(0,0);
  // use surface as temporary pixel data storage
  SDL_Surface * s   = IMG_Load(f);
  // init the object only if the file has loaded.
  if exists(s) {
    image_surface(self)  = s;
    image_width(self)    = s->w;
    image_height(self)   = s->h;
    image_path(self)     = f;
  }
}

// width and height must be included.
// SDL for this machine is using SDL_PIXELFORMAT_ABGR8888
image_t * image( char * f ) {

  image_t * r       = alloc(image_t);
  init_image(r,f);

  if image_surface(r) {
    return r;
  }
  else {
    free_image(r);
    return null();
  }
}

///////////
// funcs //
///////////

// SDL_PIXELFORMAT_ABGR8888
bool_t image_get_pixel( image_t * self, int x, int y, byte_t * rr, byte_t * rg, byte_t * rb, byte_t * ra ) {
  if (inrect(x,y,0,0,image_width(self),image_height(self))) {
    uint_t * pp = (uint_t*) get(image_surface(self),pixels);
    int a       = (image_width(self)*y) + x;
    uint_t p    = pp[a];
    val(rr) = (p)     & 255;
    val(rg) = (p>>8)  & 255;
    val(rb) = (p>>16) & 255;
    val(ra) = (p>>24) & 255;
    return true();
  }
  return false();
}