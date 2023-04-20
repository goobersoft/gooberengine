
type() {

  rect_t *  rect;
  int       delay;

} animframe_t;

#define animframe_rect(self)  (self->rect)
#define animframe_delay(self) (self->delay)

#define animframe_set_rect(self,x,y,w,h)  rect_set(self,x,y,w,h)
#define animframe_set_delay(self,n)       (animframe_delay(self)=low(n,1))

void init_animframe( animframe_t * self, int x, int y, int w, int h, int d ) {
  animframe_rect(self)    = rect(x,y,w,h);
  animframe_delay(self)   = low(d,1);
}

animframe_t * animframe( int x, int y, int w, int h, int d ) {
  animframe_t * self = alloc(animframe_t);
  init_animframe(self,x,y,w,h,d);
  return self;
}

animframe_t * animframe_as_blank(void) {
  return animframe(0,0,10,10,1);
}

void animframe_set( animframe_t * self, int x, int y, int w, int h, int d ) {
  animframe_set_rect(animframe_rect(self),x,y,w,h);
  animframe_delay(self) = low(d,1);
}

///////////////////////////////////////////////////////////////////////////////

type() {
  
  int length;
  animframe_t ** frames;
  
} animframeset_t;

#define animframeset_length(self) (self->length)
#define animframeset_frames(self) (self->frames)

void init_animframeset( animframeset_t * self, int l ) {
  animframeset_length(self) = low(l,1);
  animframeset_frames(self) = allocv(animframe_t*,l);
  loop(i,l) {
    animframeset_frames(self)[i] = animframe_as_blank();
  }
}

animframeset_t * animframeset( int l ) {
  animframeset_t * self = alloc(animframeset_t);
  init_animframeset(self,l);
  return self;
}

///////////////////////////////////////////////////////////////////////////////

#define animation_playmode_stopped() 0
#define animation_playmode_playing() 1
#define animation_playmode_looping() 2

type() {
  int               curr_frame;
  int               curr_delay;
  int               playmode;
  foreign( animframeset_t * frameset );

} animation_t;


#define animation_curr_frame(self)   (self->curr_frame)
#define animation_curr_delay(self)   (self->curr_delay)
#define animation_playmode(self)     (self->playmode)
#define animation_frameset(self)     (self->frameset)
#define animation_length(self)       animframeset_length(animation_frameset(self))
#define animation_frames(self)       animframeset_frames(animation_frameset(self))
#define animation_has_frameset(self) (animation_frameset(self)!=null())

#define animation_get_curr_frame(self) \
  animation_frames(self)[animation_curr_frame(self)]
#define animation_get_curr_rect(self) \
  animframe_rect(animation_get_curr_frame(self))


void init_animation( animation_t * self, animframeset_t * a ) {
  animation_curr_frame(self)  = 0;
  animation_curr_delay(self)  = 0;
  animation_playmode(self)    = animation_playmode_stopped();
  animation_frameset(self)    = a;
}

animation_t * animation( animframeset_t * a ) {
  animation_t * self = alloc(animation_t);
  init_animation(self,a);
  return self;
}

//-- functions --//

void animation_fill_delay( animation_t * self ) {
  animation_curr_delay(self)  = 
    animframe_delay(animation_frames(self)[animation_curr_frame(self)]);
}

void animation_inc_curr_frame( animation_t * self ) {
  if (animation_curr_frame(self) == (animation_length(self)-1)) {
    if (animation_playmode(self) == animation_playmode_looping()) {
      animation_curr_frame(self) = 0;
      animation_fill_delay(self);
    }
    else {
      animation_playmode(self) = animation_playmode_stopped();
    }
  }
  else if (animation_curr_frame(self) < (animation_length(self)-1)) {
    animation_curr_frame(self) += 1;
    animation_fill_delay(self);
  }
}

// plays and restarts the animation
void animation_play( animation_t * self ) {
  animation_playmode(self)    = animation_playmode_playing();
  animation_curr_frame(self)  = 0;
  animation_fill_delay        (self);
}

// plays and restarts the animation - sets itself to loop
void animation_loop( animation_t * self ) {
  animation_playmode(self)    = animation_playmode_looping();
  animation_curr_frame(self)  = 0;
}

// stops and rewinds the animation
void animation_stop( animation_t * self ) {
  animation_playmode(self)    = animation_playmode_stopped();
  animation_curr_frame(self)  = 0;
}

// does not rewind the animation but does stop it.
void animation_pause( animation_t * self ) {
  animation_playmode(self)    = animation_playmode_stopped();
}

//-- events --//

void animation_update( animation_t * self ) {
  if (animation_playmode(self) != animation_playmode_stopped()) {
    if (animation_curr_delay(self) > 0) {
      animation_curr_delay(self) -= 1;
    }
    else if (animation_curr_delay(self) <= 0) {
      animation_inc_curr_frame(self);
    }
  }
}