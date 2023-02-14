
type() {

  local( scene_t * scene );
  local( playfield_t * playfield );

} scenegame_t;

#define scenegame_scene(self)      (self->scene)
#define scenegame_playfield(self)  (self->playfield)

/////////
// new //
/////////

void init_scenegame( scenegame_t * self ) {
  scenegame_scene(self)      = scene(self,"game");
  scenegame_playfield(self)  = playfield();
}

scenegame_t * scenegame() {
  scenegame_t * self = alloc(scenegame_t);
  init_scenegame(self);
  return self;
}

void free_scenegame( scenegame_t * self ) {
  free_playfield  (scenegame_playfield(self));
  free_scene      (scenegame_scene(self));
  free            (self);
}

////////////
// events //
////////////

void scenegame_start( scenegame_t * self ) {
}

void scenegame_update( scenegame_t * self ) {
}

void scenegame_draw( scenegame_t * self ) {
}

void scenegame_quit( scenegame_t * self ) {
}