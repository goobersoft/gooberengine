
#define brick_id_none() 0

type() {

  tag_t     * tag;
  entity_t  * entity;
  sprite_t  * sprite;

} brick_t;

#define brick_tag(self)     (self->tag)
#define brick_id(self)      tag_id(self->tag)
#define brick_entity(self)  (self->entity)
#define brick_sprite(self)  (self->sprite)

/////////
// new //
/////////

void init_brick( brick_t * self ) {
  brick_tag(self)     = tag(self,"brick");
  brick_entity(self)  = entity(self);
  entity_set_size( brick_entity(self), 20, 10 );
  brick_sprite(self)  = sprite( gb_get_colormap("bricker-0"), 30,0,20,10 );
}

brick_t * brick() {
  brick_t * self      = alloc(brick_t);
  init_brick(self);
  return self;
}

void free_brick( brick_t * self ) {
  free_tag    ( brick_tag(self) );
  free_entity ( brick_entity(self) );
  free_sprite ( brick_sprite(self) );
  free(self);
}

///////////////
// functions //
///////////////

void brick_set_id( brick_t * self, char * id ) {
  
  if streq(id,"") {
    sprite_set_pos(brick_sprite(self),0,0);
    sprite_set_size(brick_sprite(self),10,10);
    entity_set_solid(brick_entity(self),false());
  }
  else {
    if streq(id,"1")   sprite_set_pos(brick_sprite(self),30,0);
    if streq(id,"2")   sprite_set_pos(brick_sprite(self),30,10);
    if streq(id,"3")   sprite_set_pos(brick_sprite(self),30,20);
    if streq(id,"4")   sprite_set_pos(brick_sprite(self),50,0);
    if streq(id,"5")   sprite_set_pos(brick_sprite(self),30,10);
    if streq(id,"6")   sprite_set_pos(brick_sprite(self),30,20);
    entity_set_solid(brick_entity(self),true());
  }
}

////////////
// events //
////////////

void brick_update( brick_t * self ) {

}

void brick_draw( brick_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), entity_pos_x(brick_entity(self)), 
    entity_pos_y(brick_entity(self)), brick_sprite(self));
  graph_reset_intensity( gb_graph() );
}