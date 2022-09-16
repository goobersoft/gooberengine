
type() {

  Mix_Chunk * source;

} sound_t;

#define sound_source(self) (self->source)

/////////
// new //
/////////

void sound_init( sound_t * self, char * f ) {
  
}

sound_t * sound( char * f ) {
  sound_t * r = alloc(sound_t);
  sound_source(r) = Mix_LoadWAV( f );
  if (sound_source(r) != null()) {
    //log("sound '%s' loaded successfully.",f);
    return r;
  }
  else {
    free(r);
    return null();
  }
}

////////////
// events //
////////////

