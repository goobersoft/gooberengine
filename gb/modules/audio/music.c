
type() {

  Mix_Music * source;

} music_t;

#define music_source(self) (self->source)

/////////
// new //
/////////

void init_music( music_t * self, char * s ) {
  music_source(self) = Mix_LoadMUS(s);
}

music_t * music( char * s ) {
  music_t * self = alloc(music_t);
  init_music(self,s);
  if (music_source(self) != null()) {
    return self;
  }
  else {
    free(self);
    return null();
  }
  return self;
}

void free_music( music_t * self ) {
  Mix_FreeMusic(music_source(self));
  free(self);
}