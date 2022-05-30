
#define audio_channel_stopped() 0
#define audio_channel_playing() 1

#define audio_channel_sfx1()  0 // sound effect channel 1
#define audio_channel_sfx2()  1 // sound effect channel 2
#define audio_channel_sfx3()  2 // sound effect channel 3
#define audio_channel_sfx4()  3 // sound effect channel 4
#define audio_channel_bfx1()  4 // background effect 1
#define audio_channel_bfx2()  5 // background effect 2
#define audio_channel_aux()   6 // auxillary / menu
#define audio_channel_music() 7 // background music

type() {

  int num_channels;
  int * channel_state;

} audio_t;

#define audio_num_channels(self)   (self->num_channels)
#define audio_channel_state(self)  (self->channel_state)

/////////
// new //
/////////

audio_t * audio() {
  audio_t * r                = alloc(audio_t);
  audio_num_channels(r)      = 8;
  Mix_AllocateChannels(8);
  audio_channel_state(r)     = allocv(int,8);
  return r;
}

///////////
// funcs //
///////////

void audio_set_channels( audio_t * self, int n ) {
  audio_num_channels(self) = n;
  Mix_AllocateChannels(n);
  free( audio_channel_state(self) );
  audio_channel_state(self) = allocv(int,n);
}

void audio_play( audio_t * self, sound_t * s, int c ) {
  c = wrap(c,0,audio_num_channels(self));
  Mix_PlayChannel( c, sound_source(s), 0 );
  audio_channel_state(self)[c] = audio_channel_playing();
}

void audio_play_looped( audio_t * self, sound_t * s, int c ) {
  c = wrap(c,0,audio_num_channels(self));
  Mix_PlayChannel( c, sound_source(s), -1 );
  audio_channel_state(self)[c] = audio_channel_playing();
}

void audio_stop( audio_t * self, int c ) {
  c = wrap(c,0,audio_num_channels(self));
  Mix_HaltChannel( c );
  audio_channel_state(self)[c] = audio_channel_stopped();
}

////////////
// events //
////////////

void audio_update( audio_t * self ) {
  loop(i,0,audio_num_channels(self)) {
    if (Mix_Playing(i)==true()) {
      audio_channel_state(self)[i] = audio_channel_playing();
    }
    else {
      audio_channel_state(self)[i] = audio_channel_stopped();
    }
  }
}