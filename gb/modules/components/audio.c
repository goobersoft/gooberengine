

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

#define audio_playback_freq() 44100
#define audio_num_channels()  10
#define audio_buffer_size()   4096

type() {

  // the samples which will be played back in the audio engine.
  int *  data;
  // each channel has its own audio data to be mixed.
  int ** channel_data;
  // the state of each channel
  int *  channel_state;


} audio_t;

#define audio_data(self)           (self->data)
#define audio_channel_data(self)   (self->channel_data)
#define audio_channel_state(self)  (self->channel_state)

/////////
// new //
/////////

void init_audio( audio_t * self ) {
  audio_data(self)              = allocv(int,audio_buffer_size());
  audio_channel_data(self)      = allocv(int*,audio_num_channels());
  loop(i,audio_num_channels()) {
    audio_channel_data(self)[i] = allocv(int,audio_buffer_size());
  }
  audio_channel_state(self)     = allocv(int,audio_num_channels());
  Mix_AllocateChannels(audio_num_channels());
}

audio_t * audio() {
  audio_t * r = alloc(audio_t);
  init_audio(r);
  return r;
}

///////////
// funcs //
///////////

void audio_play( audio_t * self, sound_t * s, int c ) {
  c = wrap(c,0,audio_num_channels());
  Mix_PlayChannel( c, sound_source(s), 0 );
  audio_channel_state(self)[c] = audio_channel_playing();
}

void audio_play_looped( audio_t * self, sound_t * s, int c ) {
  c = wrap(c,0,audio_num_channels());
  Mix_PlayChannel( c, sound_source(s), -1 );
  audio_channel_state(self)[c] = audio_channel_playing();
}

void audio_stop( audio_t * self, int c ) {
  c = wrap(c,0,audio_num_channels());
  Mix_HaltChannel( c );
  audio_channel_state(self)[c] = audio_channel_stopped();
}

////////////
// events //
////////////

void audio_update( audio_t * self ) {
  loop(i,audio_num_channels()) {
    if istrue(Mix_Playing(i)) {
      audio_channel_state(self)[i] = audio_channel_playing();
    }
    else {
      audio_channel_state(self)[i] = audio_channel_stopped();
    }
  }
}