
#define brick_id_none() 0

type() {

  
  entity_t  * entity;
  sprite_t  * sprite;
  char      * id;

} entity_brick_t;

#define entity_brick_entity(self)  (self->entity)
#define entity_brick_sprite(self)  (self->sprite)
#define entity_brick_id(self)      (self->id)

/////////
// new //
/////////

void init_entity_brick( entity_brick_t * self ) {
  entity_brick_entity(self)  = entity("brick",self);
  
  entity_set_size( entity_brick_entity(self), 20, 10 );
  entity_brick_sprite(self)  = sprite( gb_get_colormap("bricker-0"), 30,0,20,10 );
}

entity_brick_t * entity_brick() {
  entity_brick_t * self      = alloc(entity_brick_t);
  init_entity_brick(self);
  return self;
}

void free_entity_brick( entity_brick_t * self ) {
  free_entity ( entity_brick_entity(self) );
  free_sprite ( entity_brick_sprite(self) );
  free(self);
}

///////////////
// functions //
///////////////

void entity_brick_set_id( entity_brick_t * self, char * id ) {
  
  if streq(id,"") {
    sprite_set_pos(entity_brick_sprite(self),0,0);
    sprite_set_size(entity_brick_sprite(self),10,10);
    entity_set_solid(entity_brick_entity(self),false());
  }
  else {
    if streq(id,"1")   sprite_set_pos(entity_brick_sprite(self),30,0);
    if streq(id,"2")   sprite_set_pos(entity_brick_sprite(self),30,10);
    if streq(id,"3")   sprite_set_pos(entity_brick_sprite(self),30,20);
    if streq(id,"4")   sprite_set_pos(entity_brick_sprite(self),50,0);
    if streq(id,"5")   sprite_set_pos(entity_brick_sprite(self),30,10);
    if streq(id,"6")   sprite_set_pos(entity_brick_sprite(self),30,20);
    entity_set_solid(entity_brick_entity(self),true());
  }
}

////////////
// events //
////////////

void entity_brick_update( entity_brick_t * self ) {

}

void entity_brick_draw( entity_brick_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), entity_pos_x(entity_brick_entity(self)), 
    entity_pos_y(entity_brick_entity(self)), entity_brick_sprite(self));
  graph_reset_intensity( gb_graph() );
}