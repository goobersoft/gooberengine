
type() {

  scene_t * scene;

} ma_scene_title_t;

#define ma_scene_title_scene(self) (self->scene)

/////////
// new //
/////////

void ma_scene_title_init( ma_scene_title_t * self ) {
  ma_scene_title_scene(self) = scene(self);
}

ma_scene_title_t * ma_scene_title() {
  ma_scene_title_t * r = alloc(ma_scene_title_t);
  ma_scene_title_init(r);
  return r;
}

void free_ma_scene_title( ma_scene_title_t * self ) {
  free_scene(ma_scene_title_scene(self));
  free(self);
}

////////////
// events //
////////////

void ma_scene_title_start( ma_scene_title_t * self ) {

}

void ma_scene_title_update( ma_scene_title_t * self ) {

}

void ma_scene_title_draw( ma_scene_title_t * self ) {

}

void ma_scene_title_quit( ma_scene_title_t * self ) {

}