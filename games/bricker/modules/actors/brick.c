
type() {

  actor_t   * actor;
  char      * id;

} actor_brick_t;

#define actor_brick_actor(self)   (self->actor)
#define actor_brick_entity(self)  actor_entity(actor_brick_actor(self))
#define actor_brick_sprite(self)  actor_sprite(actor_brick_actor(self))

#define actor_brick_id(self)      (self->id)

/////////
// new //
/////////

void init_actor_brick( actor_brick_t * self ) {
  actor_brick_actor(self) = actor("brick",self);
  actor_t * a = actor_brick_actor(self);
  entity_set_size( actor_entity(a), 20, 10 );
  actor_brick_sprite(self)  = sprite( gb_get_colormap("bricker-0"), 30,0,20,10 );
}

actor_brick_t * actor_brick() {
  actor_brick_t * self = alloc(actor_brick_t);
  init_actor_brick(self);
  return self;
}

void free_actor_brick( actor_brick_t * self ) {
  free_actor( actor_brick_actor(self) );
  free(self);
}

///////////////
// functions //
///////////////

void actor_brick_set_id( actor_brick_t * self, char * id ) {
  entity_t * e = actor_brick_entity(self);
  sprite_t * s = actor_brick_sprite(self);

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

void actor_brick_update( actor_brick_t * self ) {

}

void actor_brick_draw( actor_brick_t * self ) {
  entity_t * e = actor_brick_entity(self);
  sprite_t * s = actor_brick_sprite(self);

  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), entity_pos_x(e), entity_pos_y(e), s );
  graph_reset_intensity( gb_graph() );
}