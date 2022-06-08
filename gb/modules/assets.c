

// default maximum values
#define assets_num_images_d()     1000
#define assets_num_colormaps_d()  1000
#define assets_num_sounds_d()     1000
#define assets_num_fonts_d()      100

type() {

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

} assets_t;

#define assets_num_images(self)    (self->num_images)
#define assets_num_colormaps(self) (self->num_colormaps)
#define assets_num_sounds(self)    (self->num_sounds)
#define assets_num_fonts(self)     (self->num_fonts)
#define assets_images(self)        (self->images)
#define assets_colormaps(self)     (self->colormaps)
#define assets_sounds(self)        (self->sounds)
#define assets_fonts(self)         (self->fonts)

/////////
// new //
/////////

assets_t * assets() {
  assets_t * r            = alloc(assets_t);
  assets_num_images(r)    = assets_num_images_d();
  assets_num_colormaps(r) = assets_num_colormaps_d();
  assets_num_sounds(r)    = assets_num_sounds_d();
  assets_num_fonts(r)     = assets_num_fonts_d();
  assets_images(r)        = allocv(image_t*,    assets_num_images_d());
  assets_colormaps(r)     = allocv(colormap_t*, assets_num_colormaps_d());
  assets_sounds(r)        = allocv(sound_t*,    assets_num_sounds_d());
  assets_fonts(r)         = allocv(font_t*,     assets_num_fonts_d());
  return r;
}

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

///////////
// funcs //
///////////

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

////////////
// events //
////////////

void assets_load( assets_t * self, visual_t * v ) {
  // load images
  assets_set_image(self,0,image("gb/media/images/gb-0.png",visual_renderer(v)));
  assets_set_image(self,1,image("gb/media/images/gb-1.png",visual_renderer(v)));
  assets_set_image(self,2,image("gb/media/images/editor-bg.png",visual_renderer(v)));
  assets_set_image(self,3,image("gb/media/images/test.png",visual_renderer(v)));

  // load colormaps from images
  assets_set_colormap(self,0,colormap_from_image(assets_get_image(self,0)));
  assets_set_colormap(self,1,colormap_from_image(assets_get_image(self,1)));
  assets_set_colormap(self,2,colormap_from_image(assets_get_image(self,2)));
  assets_set_colormap(self,3,colormap_from_image(assets_get_image(self,3)));

  // load sounds
  assets_set_sound(self,0,sound("gb/media/sounds/jake-1.ogg"));
  assets_set_sound(self,1,sound("gb/media/sounds/jake-2.ogg"));
  assets_set_sound(self,2,sound("gb/media/sounds/jake-3.ogg"));
  assets_set_sound(self,3,sound("gb/media/sounds/jake-4.ogg"));

  // load font 1
  font_t * f = font( assets_get_image(self,0) );
  font_set_pos         ( f, 0, 100  );
  font_set_tile_size   ( f, 5, 10   );
  font_set_tiles_size  ( f, 10, 10  );
  assets_set_font      ( self, 0, f );
}