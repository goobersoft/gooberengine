
type() {

  actor_t   * actor;
  char      * id;

} brick_t;

#define brick_actor(self)   (self->actor)
#define brick_entity(self)  actor_entity(brick_actor(self))
#define brick_sprite(self)  actor_sprite(brick_actor(self))

#define brick_id(self)      (self->id)

/////////
// new //
/////////

void init_brick( brick_t * self ) {
  brick_actor(self) = actor();
  entity_set_size( brick_entity(self), 20000, 10000 );
  brick_sprite(self)  = sprite( gb_get_colormap("bricker-0"), 30,0,20,10 );
}

brick_t * brick(void) {
  brick_t * self = alloc(brick_t);
  init_brick(self);
  return self;
}

void free_brick( brick_t * self ) {
  free_actor( brick_actor(self) );
  free(self);
}

///////////////
// functions //
///////////////

void brick_set_id( brick_t * self, char * id ) {
  entity_t * e = brick_entity(self);
  sprite_t * s = brick_sprite(self);

  if streq(id,"") {
    sprite_set_pos(s,0,0);
    sprite_set_size(s,10,10);
    entity_set_solid(e,false());
  }
  else {
    if streq(id,"1")   sprite_set_pos(s,80,100);
    if streq(id,"2")   sprite_set_pos(s,80,110);
    if streq(id,"3")   sprite_set_pos(s,80,120);
    if streq(id,"4")   sprite_set_pos(s,100,100);
    if streq(id,"5")   sprite_set_pos(s,100,110);
    if streq(id,"6")   sprite_set_pos(s,100,120);
    entity_set_solid(e,true());
  }
}

////////////
// events //
////////////

void brick_update( brick_t * self ) {

}

void brick_draw( brick_t * self ) {
  entity_t * e = brick_entity(self);
  sprite_t * s = brick_sprite(self);

  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), entity_pos_x(e)/1000, entity_pos_y(e)/1000, s );
  graph_reset_intensity( gb_graph() );
}