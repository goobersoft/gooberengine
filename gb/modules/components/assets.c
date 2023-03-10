
////////////
// Assets //
////////////
// This is a collection of media objects. It holds images, colormaps,
// sounds, and fonts. Additional media types may be added in the future.

type() {
  // a dictionary of images
  local( dict_t * images );
  // a dictionary of colormaps
  local( dict_t * colormaps );
  // a dictionary of sounds
  local( dict_t * sounds );
  // a dictionary of fonts
  local( dict_t * fonts );

} assets_t;

#define assets_images(self)        (self->images)
#define assets_colormaps(self)     (self->colormaps)
#define assets_sounds(self)        (self->sounds)
#define assets_fonts(self)         (self->fonts)

/////////
// new //
/////////

void init_assets( assets_t * self ) {
  assets_images(self)        = dict();
  assets_colormaps(self)     = dict();
  assets_sounds(self)        = dict();
  assets_fonts(self)         = dict();
}

assets_t * assets() {
  assets_t * r = alloc(assets_t);
  init_assets(r);
  return r;
}

void free_assets( assets_t * self ) {
  free_dict( assets_images(self) );
  free_dict( assets_colormaps(self) );
  free_dict( assets_sounds(self) );
  free_dict( assets_fonts(self) );
}


////////////
// macros //
////////////

#define assets_set_image(self,k,a)    dset(assets_images(self),k,a)
#define assets_set_colormap(self,k,a) dset(assets_colormaps(self),k,a)
#define assets_set_sound(self,k,a)    dset(assets_sounds(self),k,a)
#define assets_set_font(self,k,a)     dset(assets_fonts(self),k,a)

#define assets_get_image(self,k)      dget(assets_images(self),k,image_t*)
#define assets_get_colormap(self,k)   dget(assets_colormaps(self),k,colormap_t*)
#define assets_get_sound(self,k)      dget(assets_sounds(self),k,sound_t*)
#define assets_get_font(self,k)       dget(assets_fonts(self),k,font_t*)