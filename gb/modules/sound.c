
type() {

  Mix_Chunk * source;

} sound_t;

#define sound_source(self) (self->source)

/////////
// new //
/////////

sound_t * sound( char * f ) {
  sound_t * r = alloc(sound_t);
  sound_source(r) = Mix_LoadWAV( f );
  if (sound_source(r) != null()) {
    log("sound '%s' loaded successfully.",f);
    return r;
  }
  return null();
}

////////////
// events //
////////////

