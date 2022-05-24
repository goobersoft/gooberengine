
//////////
// type //
//////////

type() {

  SDL_Renderer * renderer;
  SDL_Surface  * surface;
  SDL_Texture  * texture;
  point_t        size;
  char         * path;
  color_t      * data;

} image_t;

#define image_renderer(self) (self->renderer)
#define image_surface(self)  (self->surface)
#define image_texture(self)  (self->texture)
#define image_size(self)     (&self->size)
#define image_path(self)     (self->path)
#define image_width(self)    point_x(image_size(self))
#define image_height(self)   point_y(image_size(self))
#define image_data(self)     (self->data)

/////////
// new //
/////////

// width and height must be included.
// SDL for this machine is using SDL_PIXELFORMAT_ABGR8888
image_t * image( char * f, SDL_Renderer * rr ) {
  
  // if no dimensions are given, the default pixel screen size
  // is used instead. This is the typical size for a tilesheet.
  // if (w==0) w = 400;
  // if (h==0) h = 240;

  image_t * r       = alloc(image_t);
  // use surface as temporary pixel data storage
  SDL_Surface * s   = IMG_Load(f);
  // init the object only if the file has loaded.
  if (s != null()) {
    // An image must have a relationship to the renderer to be able
    // to create a texture. Images should be created AFTER the
    // visual module, as the visual module includes a renderer.
    image_renderer(r) = rr;
    image_surface(r)  = s;
    image_texture(r)  = SDL_CreateTextureFromSurface(rr,s);
    image_width(r)    = s->w;
    image_height(r)   = s->h;
    image_path(r)     = f;

    //log("%s\n" , sdl_pixelformat_str(s->format->format));
    log("image '%s' loaded successfully." nl(),image_path(r));
    return r;
  }
  // no need to free the SDL_Surface if it failed - it is null.
  log("image '%s' failed." nl(),f);
  return null();
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