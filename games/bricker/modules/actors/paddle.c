
#define actor_paddle_bend_up()    1
#define actor_paddle_bend_none()  0
#define actor_paddle_bend_down() -1

type() {

  local( actor_t * actor );

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

} actor_paddle_t;

#define actor_paddle_actor(self)  (self->actor)
#define actor_paddle_entity(self) actor_entity(actor_paddle_actor(self))
#define actor_paddle_sprite(self) actor_sprite(actor_paddle_actor(self))
#define actor_paddle_bend(self)   get(self,bend)
#define actor_paddle_width(self)  get(self,width)

#define actor_paddle_pos_x(self)  entity_pos_x(actor_paddle_entity(self))
#define actor_paddle_pos_y(self)  entity_pos_y(actor_paddle_entity(self))

/////////
// new //
/////////

void init_actor_paddle( actor_paddle_t * self ) {
  actor_paddle_actor(self)   = actor("paddle",self);
  actor_t * a   = actor_paddle_actor(self);

  entity_t * ae = actor_paddle_entity(self);
  entity_set_pos    (ae,185,220);
  entity_set_size   (ae,30,8);

  sprite_t * as = actor_paddle_sprite(self);
  sprite_set(as,gb_get_colormap("bricker-0"),70,10,30,10);
  sprite_set_offset(as,0,1);

  actor_paddle_bend(self)     = actor_paddle_bend_none();
  // 1 indicates only one internal segment between the two end pieces.
  actor_paddle_width(self)    = 1;
}

actor_paddle_t * actor_paddle() {
  actor_paddle_t * self = alloc(actor_paddle_t);
  init_actor_paddle(self);
  return self;
}

void free_actor_paddle( actor_paddle_t * self ) {
  free_actor( actor_paddle_actor(self) );
  free(self);
}

///////////////
// functions //
///////////////

void paddle_move_left( actor_paddle_t * self ) {
  actor_paddle_pos_x(self) = low(actor_paddle_pos_x(self)-2,100);
}

void paddle_move_right( actor_paddle_t * self ) {
  actor_paddle_pos_x(self) = high(actor_paddle_pos_x(self)+2,280-(actor_paddle_width(self)*10));
}

////////////
// events //
////////////

void actor_paddle_update( actor_paddle_t * self ) {
  if (gb_button(controller_button_left())) {
    paddle_move_left(self);
    //point_add( paddle_pos(self), -2, 0 );
  }
  else if (gb_button(controller_button_right())) {
    paddle_move_right(self);
    //point_add( paddle_pos(self), 2, 0 );
  }

  if (gb_button(controller_button_up())) {
    sprite_set_pos( actor_paddle_sprite(self), 70, 0 );
  }
  else if (gb_button(controller_button_down())) {
    sprite_set_pos( actor_paddle_sprite(self), 70, 20 );
  }
  else {
    sprite_set_pos( actor_paddle_sprite(self), 70, 10 );
  }
}

void actor_paddle_draw( actor_paddle_t * self ) {
  graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), actor_paddle_pos_x(self), actor_paddle_pos_y(self), actor_paddle_sprite(self) );
  graph_reset_intensity( gb_graph() );
}