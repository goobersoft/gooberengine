



type() {
  // moving to dictionaries so that sounds/images/fonts
  // can be recalled with keys, not indices.
  dict_t * images;
  dict_t * colormaps;
  dict_t * sounds;
  dict_t * fonts;

  // reference to visual object
  visual_t * visual;

} assets_t;

#define assets_images(self)        (self->images)
#define assets_colormaps(self)     (self->colormaps)
#define assets_sounds(self)        (self->sounds)
#define assets_fonts(self)         (self->fonts)
#define assets_visual(self)        (self->visual)

/////////
// new //
/////////

void assets_init( assets_t * self, visual_t * v ) {
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


///////////
// funcs //
///////////

// -- setters -- //

void * assets_set_image( assets_t * self, char * k, image_t * a ) {
  return dset(assets_images(self),k,a);
}

void * assets_set_colormap( assets_t * self, char * k, colormap_t * c ) {
  return dset(assets_colormaps(self),k,c);
}

void * assets_set_sound( assets_t * self, char * k, sound_t * s ) {
  return dset(assets_sounds(self),k,s);
}

void * assets_set_font( assets_t * self, char * k, font_t * f ) {
  return dset(assets_fonts(self),k,f);
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
