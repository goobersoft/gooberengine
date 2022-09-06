

// default maximum values
/*
#define assets_num_images_d()     1000
#define assets_num_colormaps_d()  1000
#define assets_num_sounds_d()     1000
#define assets_num_fonts_d()      100
*/

type() {

  /*
  int num_sounds;
  int num_images;
  int num_colormaps;
  int num_fonts;
  
  // array of images
  image_t ** images;
  // array of colormaps
  colormap_t ** colormaps;
  // array of sounds
  sound_t ** sounds;
  // array of fonts
  font_t  ** fonts;
  */

  // moving to dictionaries so that sounds/images/fonts
  // can be recalled with keys, not indices.
  dict_t * images;
  dict_t * colormaps;
  dict_t * sounds;
  dict_t * fonts;

  // reference to visual object
  visual_t * visual;

} assets_t;

/*
#define assets_num_images(self)    (self->num_images)
#define assets_num_colormaps(self) (self->num_colormaps)
#define assets_num_sounds(self)    (self->num_sounds)
#define assets_num_fonts(self)     (self->num_fonts)
*/
#define assets_images(self)        (self->images)
#define assets_colormaps(self)     (self->colormaps)
#define assets_sounds(self)        (self->sounds)
#define assets_fonts(self)         (self->fonts)
#define assets_visual(self)        (self->visual)

/////////
// new //
/////////

void assets_init( assets_t * self, visual_t * v ) {
  /*
  assets_num_images(self)    = assets_num_images_d();
  assets_num_colormaps(self) = assets_num_colormaps_d();
  assets_num_sounds(self)    = assets_num_sounds_d();
  assets_num_fonts(self)     = assets_num_fonts_d();
  assets_images(self)        = allocv(image_t*,    assets_num_images_d());
  assets_colormaps(self)     = allocv(colormap_t*, assets_num_colormaps_d());
  assets_sounds(self)        = allocv(sound_t*,    assets_num_sounds_d());
  assets_fonts(self)         = allocv(font_t*,     assets_num_fonts_d());
  */
  assets_images(self)        = dict();
  assets_colormaps(self)     = dict();
  assets_sounds(self)        = dict();
  assets_fonts(self)         = dict();
  assets_visual(self)        = v;
}

assets_t * assets( visual_t * v ) {
  assets_t * r            = alloc(assets_t);
  assets_init(r,v);
  return r;
}

/*
assets_t * assets__custom_sizes( size_t i, size_t c, size_t s, size_t f ) {
  i = clamp(i,0,100000);
  s = clamp(s,0,100000);
  f = clamp(f,0,100000);
  assets_t * r         = alloc(assets_t);
  assets_num_images(r)    = i;
  assets_num_colormaps(r) = c;
  assets_num_sounds(r)    = s;
  assets_num_fonts(r)     = f;
  assets_images(r)        = allocv(image_t*,    i);
  assets_colormaps(r)     = allocv(colormap_t*, c);
  assets_sounds(r)        = allocv(sound_t*,    s);
  assets_fonts(r)         = allocv(font_t*,     f);
  return r;
}
*/

///////////
// funcs //
///////////

// -- setters -- //

bool_t assets_set_image( assets_t * self, char * k, image_t * a ) {
  return dset(assets_images(self),k,a);
  //return dict_set( assets_images(self), k, a );
}

bool_t assets_set_colormap( assets_t * self, char * k, colormap_t * c ) {
  return dset(assets_colormaps(self),k,c);
  //return dict_set( assets_colormaps(self), k, c);
}

bool_t assets_set_sound( assets_t * self, char * k, sound_t * s ) {
  return dset(assets_sounds(self),k,s);
  //return dict_set( assets_sounds(self), k, s);
}

bool_t assets_set_font( assets_t * self, char * k, font_t * f ) {
  return dset(assets_fonts(self),k,f);
  //return dict_set( assets_fonts(self), k, f);
}

// -- getters -- //

image_t * assets_get_image( assets_t * self, char * k ) {
  return dget(assets_images(self),k,image_t*);
  //return dict_get( assets_images(self), k );
}

colormap_t * assets_get_colormap( assets_t * self, char * k ) {
  return dget(assets_colormaps(self),k,colormap_t*);
  //return dict_get( assets_colormaps(self), k );
}

sound_t * assets_get_sound( assets_t * self, char * k ) {
  return dget(assets_sounds(self),k,sound_t*);
  //return dict_get( assets_sounds(self), k );
}

font_t * assets_get_font( assets_t * self, char * k ) {
  return dget(assets_fonts(self),k,font_t*);
  //return dict_get( assets_fonts(self), k );
}

/*
// this does both images and colormaps as well
bool_t assets_set_image( assets_t * self, int n, image_t * a ) {
  if (inrange(n,0,assets_num_images(self))) {
    assets_images(self)[n] = a;
    log("image id %d set to '%s'" nl(), n, image_path(a));
    return true();
  }
  log("image id %d could not be set." nl(), n);
  return false();
}

bool_t assets_load_image( assets_t * self, int n, char * t ) {
  if (assets_visual(self)!=null()) {
    return assets_set_image( self, n, image(t,assets_visual(self)) );
  }
  return false();
}

image_t * assets_get_image( assets_t * self, int n ) {
  if (inrange(n,0,assets_num_images(self))) {
    return assets_images(self)[n];
  }
  return null();
}

bool_t assets_set_colormap( assets_t * self, int n, colormap_t * c ) {
  if (inrange(n,0,assets_num_colormaps(self))) {
    assets_colormaps(self)[n] = c;
    log("colormap id %d set.",n);
    return true();
  }
  log("colormap id %d could not be set.", n);
  return false();
}

colormap_t * assets_get_colormap( assets_t * self, int n ) {
  if (inrange(n,0,assets_num_colormaps(self))) {
    return assets_colormaps(self)[n];
  }
  return null();
}


bool_t assets_set_sound( assets_t * self, int n, sound_t * a ) {
  if (inrange(n,0,assets_num_sounds(self))) {
    assets_sounds(self)[n] = a;
    //log("sound id %d set to '%s'" nl(), n, sound_path(a));
    return true();
  }
  return false();
}

sound_t * assets_get_sound( assets_t * self, int n ) {
  if (inrange(n,0,assets_num_sounds(self))) {
    return assets_sounds(self)[n];
  }
  return null();
}


bool_t assets_set_font( assets_t * self, int n, font_t * a ) {
  if (inrange(n,0,assets_num_fonts(self))) {
    assets_fonts(self)[n] = a;
    return true();
  }
  return false();
}

font_t * assets_get_font( assets_t * self, int n ) {
  if (inrange(n,0,assets_num_fonts(self))) {
    return assets_fonts(self)[n];
  }
  return null();
}
*/

////////////
// events //
////////////