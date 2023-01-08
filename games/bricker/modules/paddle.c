
#define paddle_bend_up()    1
#define paddle_bend_none()  0
#define paddle_bend_down() -1

type() {

  // tag
  local( tag_t * tag );
  // sprite object for graphical drawing
  // note: will very likely use a tilemap for the paddle in the future
  // to allow the paddle to expand and shrink
  local( sprite_t * sprite );
  // entity
  local( entity_t * entity );

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

} paddle_t;

#define paddle_tag(self)    (self->tag)
#define paddle_id(self)     (self->tag->id)
#define paddle_entity(self) (self->entity)
#define paddle_pos_x(self)  entity_pos_x(self->entity)
#define paddle_pos_y(self)  entity_pos_y(self->entity)
#define paddle_sprite(self) (self->sprite)

#define paddle_bend(self)   (self->bend)
#define paddle_width(self)  (self->width)


/////////
// new //
/////////

void init_paddle( paddle_t * self ) {
  paddle_tag(self)      = tag(self,"paddle");
  paddle_entity(self)   = entity(self);
    entity_set_pos        (paddle_entity(self),185,220);
    entity_set_size       (paddle_entity(self),30,8);
  paddle_sprite(self)   = sprite(gb_get_colormap("bricker-0"),70,10,30,10);
  
  paddle_bend(self)     = paddle_bend_none();
  // 1 indicates only one internal segment between the two end pieces.
  paddle_width(self)    = 1;
}

paddle_t * paddle() {
  paddle_t * self = alloc(paddle_t);
  init_paddle(self);
  return self;
}

void free_paddle( paddle_t * self ) {
  free_tag    ( paddle_tag(self) );
  free_sprite ( paddle_sprite(self) );
  free_entity ( paddle_entity(self) );
  free(self);
}

////////////
// macros //
////////////

#define paddle_collide(self,other)     entity_collide(pball_entity(self),other)
#define paddle_collide_list(self,ls)   entity_collide_list(pball_entity(self),ls)

////////////
// events //
////////////

void paddle_update( paddle_t * self ) {
  if (gb_button(controller_button_left())) {
    paddle_pos_x(self) = low(paddle_pos_x(self)-2,100);
    //point_add( paddle_pos(self), -2, 0 );
  }
  else if (gb_button(controller_button_right())) {
    paddle_pos_x(self) = high(paddle_pos_x(self)+2,280-(paddle_width(self)*10));
    //point_add( paddle_pos(self), 2, 0 );
  }

  if (gb_button(controller_button_up())) {
    sprite_set_pos( paddle_sprite(self), 70, 0 );
  }
  else if (gb_button(controller_button_down())) {
    sprite_set_pos( paddle_sprite(self), 70, 20 );
  }
  else {
    sprite_set_pos( paddle_sprite(self), 70, 10 );
  }
}

void paddle_draw( paddle_t * self ) {
  graph_draw_sprite( gb_graph(), paddle_pos_x(self), paddle_pos_y(self), paddle_sprite(self) );
}