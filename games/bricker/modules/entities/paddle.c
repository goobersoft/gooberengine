
#define entity_paddle_bend_up()    1
#define entity_paddle_bend_none()  0
#define entity_paddle_bend_down() -1

type() {

  local( entity_t * entity );

  // sprite object for graphical drawing
  // note: will very likely use a tilemap for the paddle in the future
  // to allow the paddle to expand and shrink
  local( sprite_t * sprite );

  // indicates how the paddle is currently bent:
  // 0: flat
  // 1: curved up (convex)
  // 2: curved down (concave)
  int bend;
  // The additional width of the paddle, 1 by default.
  // the paddle is usually 3 tiles long, but could get more
  // segments with a powerup. 
  int width;

  // indicates if the paddle is active and can collide with other things.
  bool_t active;

} entity_paddle_t;


#define entity_paddle_entity(self) (self->entity)
#define entity_paddle_pos_x(self)  entity_pos_x(self->entity)
#define entity_paddle_pos_y(self)  entity_pos_y(self->entity)
#define entity_paddle_sprite(self) (self->sprite)

#define entity_paddle_bend(self)   (self->bend)
#define entity_paddle_width(self)  (self->width)


/////////
// new //
/////////

void init_entity_paddle( entity_paddle_t * self ) {
  entity_paddle_entity(self)   = entity("paddle",self);
    entity_set_pos        (entity_paddle_entity(self),185,220);
    entity_set_size       (entity_paddle_entity(self),30,8);
  entity_paddle_sprite(self)   = sprite(gb_get_colormap("bricker-0"),70,10,30,10);
  
  entity_paddle_bend(self)     = entity_paddle_bend_none();
  // 1 indicates only one internal segment between the two end pieces.
  entity_paddle_width(self)    = 1;
}

entity_paddle_t * entity_paddle() {
  entity_paddle_t * self = alloc(entity_paddle_t);
  init_entity_paddle(self);
  return self;
}

void free_entity_paddle( entity_paddle_t * self ) {
  free_entity ( entity_paddle_entity(self) );
  free_sprite ( entity_paddle_sprite(self) );
  free(self);
}

////////////
// macros //
////////////

#define entity_paddle_collide(self,other)     entity_collide(entity_paddle_entity(self),other)
#define entity_paddle_collide_list(self,ls)   entity_collide_list(entity_paddle_entity(self),ls)

////////////
// events //
////////////

void entity_paddle_update( entity_paddle_t * self ) {
  if (gb_button(controller_button_left())) {
    entity_paddle_pos_x(self) = low(entity_paddle_pos_x(self)-2,100);
    //point_add( paddle_pos(self), -2, 0 );
  }
  else if (gb_button(controller_button_right())) {
    entity_paddle_pos_x(self) = high(entity_paddle_pos_x(self)+2,280-(entity_paddle_width(self)*10));
    //point_add( paddle_pos(self), 2, 0 );
  }

  if (gb_button(controller_button_up())) {
    sprite_set_pos( entity_paddle_sprite(self), 70, 0 );
  }
  else if (gb_button(controller_button_down())) {
    sprite_set_pos( entity_paddle_sprite(self), 70, 20 );
  }
  else {
    sprite_set_pos( entity_paddle_sprite(self), 70, 10 );
  }
}

void entity_paddle_draw( entity_paddle_t * self ) {
  graph_draw_sprite( gb_graph(), entity_paddle_pos_x(self), entity_paddle_pos_y(self), entity_paddle_sprite(self) );
}