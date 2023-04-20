
#define paddle_bending_up()    1
#define paddle_bending_none()  0
#define paddle_bending_down() -1

type() {

  local( actor_t * actor );

  // indicates how the paddle is currently bent:
  //  0: flat
  //  1: curved up (convex)
  // -1: curved down (concave)
  int bending;
  // The additional width of the paddle, 1 by default.
  // the paddle is usually 3 tiles long, but could get more
  // segments with a powerup. 
  int width;
  // can the player control this paddle?
  bool_t controllable;
  // indicates if the paddle is active and can collide with other things.
  bool_t active;

} paddle_t;

#define paddle_actor(self)    (self->actor)
#define paddle_entity(self)   actor_entity(paddle_actor(self))
#define paddle_sprite(self)   actor_sprite(paddle_actor(self))
#define paddle_bending(self)  get(self,bending)
#define paddle_width(self)    get(self,width)

#define paddle_pos_x(self)  entity_pos_x(paddle_entity(self))
#define paddle_pos_y(self)  entity_pos_y(paddle_entity(self))

#define paddle_controllable(self) (self->controllable)
#define paddle_active(self)       (self->active)

/////////
// new //
/////////

void init_paddle( paddle_t * self ) {
  paddle_actor(self)   = actor();
  actor_t * a   = paddle_actor(self);

  entity_t * ae = paddle_entity(self);
  entity_set_pos    (ae,185000,220000);
  entity_set_size   (ae,30000,8000);

  sprite_t * as = paddle_sprite(self);
  sprite_set(as,gb_get_colormap("bricker-0"),70,10,30,10);
  sprite_set_offset(as,0,1);

  paddle_bending(self)        = paddle_bending_none();
  // 1 indicates only one internal segment between the two end pieces.
  paddle_width(self)          = 1;
  paddle_controllable(self)   = false();
  paddle_active(self)         = true();
}

paddle_t * paddle(void) {
  paddle_t * self = alloc(paddle_t);
  init_paddle(self);
  return self;
}

void free_paddle( paddle_t * self ) {
  free_actor( paddle_actor(self) );
  free(self);
}

///////////////
// functions //
///////////////

void paddle_move_left( paddle_t * self ) {
  if (paddle_active(self)) {
    paddle_pos_x(self) = low(paddle_pos_x(self)-2000,100000);
  }
}

void paddle_move_right( paddle_t * self ) {
  if (paddle_active(self)) {
    paddle_pos_x(self) = high(paddle_pos_x(self)+2000,280000-(paddle_width(self)*10000));
  }
}

void paddle_bend( paddle_t * self, int d ) {
  d = clamp(d,-1,1);
  paddle_bending(self) = d; 
  // set the sprite accordingly.
  switch(d) {
    case 0:   sprite_set_pos( paddle_sprite(self), 70, 10 ); break;
    case 1:   sprite_set_pos( paddle_sprite(self), 70, 0 );  break;
    case -1:  sprite_set_pos( paddle_sprite(self), 70, 20 ); break;
  }
}

void paddle_bend_down( paddle_t * self ) {
  paddle_bend(self,-1);
}

void paddle_bend_up( paddle_t * self ) {
  paddle_bend(self,1);
}



////////////
// events //
////////////

void paddle_update( paddle_t * self ) {
  // reset bending for this frame.
  paddle_bend(self,0);
  // only move this paddle if controllable.
  if (paddle_controllable(self)) {
    if (gb_button(controller_button_left())) {
      paddle_move_left(self);
      //point_add( paddle_pos(self), -2, 0 );
    }
    else if (gb_button(controller_button_right())) {
      paddle_move_right(self);
      //point_add( paddle_pos(self), 2, 0 );
    }

    if (gb_button(controller_button_up())) {
      paddle_bend_up(self);
    }
    else if (gb_button(controller_button_down())) {
      paddle_bend_down(self);
    }
  }
}

void paddle_draw( paddle_t * self ) {
  if (paddle_active(self)) {
    graph_set_intensity_max( gb_graph() );
    graph_draw_sprite( gb_graph(), paddle_pos_x(self)/1000, paddle_pos_y(self)/1000, paddle_sprite(self) );
    graph_reset_intensity( gb_graph() );
  }
}