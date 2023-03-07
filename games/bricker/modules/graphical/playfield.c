
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t            * bricks; // entities
  list_t            * balls;  // entities
  actor_paddle_t    * paddle;

  list_t            * events;

} playfield_t;

#define playfield_bricks(self)     (self->bricks)
#define playfield_balls(self)      (self->balls)
#define playfield_paddle(self)     (self->paddle)
#define playfield_events(self)     (self->events)

#define playfield_move_paddle_left(self) paddle_move_left(playfield_paddle(self))
#define playfield_move_paddle_right(self) paddle_move_right(playfield_paddle(self))

/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  playfield_events(self)    = list();
  point_t sz                = playfield_bricks_size();
  playfield_bricks(self)    = list();

  actor_brick_t * b;
  actor_t       * ba;
  entity_t      * ae;

  loop(i,sz.x) {
    loop(j,sz.y) {
      // set all bricks to blank bricks
      b = actor_brick();
      ba = actor_brick_actor(b);
      ae = actor_entity(ba);
      
      entity_set_pos( ae, 100+(i*20), (j*10) );
      entity_set_solid( ae, true() );

      // store the entity, not the brick
      list_add_last( playfield_bricks(self), ae );
    }
  }

  // make the list of balls
  playfield_balls(self)     = list();
  playfield_paddle(self)    = actor_paddle();
}

playfield_t * playfield() {
  playfield_t * self = alloc(playfield_t);
  init_playfield(self);
  return self;
}

void free_playfield( playfield_t * self ) {
  entity_t       * e;
  actor_t        * ea;
  actor_pball_t  * ab;
  actor_brick_t  * at;

  // remove all balls first
  
  while (list_count( playfield_balls(self) ) > 0) {
    e = list_remove_first( playfield_balls(self) );
    ea = entity_get_spec(e);
    ab = actor_get_spec(e);
    free_actor_pball(ab);
  }
  // now free the list
  free_list( playfield_balls(self) );

  while (list_count( playfield_balls(self) ) > 0) {
    e  = list_remove_first( playfield_bricks(self) );
    ea = entity_get_spec(e); 
    at = actor_get_spec(ea);
    free_actor_brick(at);
  }
  free_list( playfield_bricks(self) );

  free_list( playfield_events(self) );
}

///////////////
// functions //
///////////////

// x: x pos of ball (relative to playfield)
// y: y pos of ball (relative to playfield)
// t: the type of ball (future feature?)
void playfield_add_ball( playfield_t * self, int x, int y, int t ) {

}

void playfield_set_brick( playfield_t * self, int bx, int by, int b ) {

}

////////////
// events //
////////////

void playfield_update( playfield_t * self ) {
  list_clear( playfield_events(self) );

  // ball's entity
  entity_t * ent_a;
  // brick's entity
  entity_t * ent_b;
  //
  actor_t  * act_a;
  //
  actor_t  * act_b;
  // ref to current ball
  actor_pball_t  * pball;
  // ref to current brick
  actor_brick_t  * brick;
  //
  actor_paddle_t * paddle = playfield_paddle(self);
  // result for glide functions
  int du;

  foreach( playfield_balls(self), dt ) {
    // cast the void pointer to a pball object
    ent_a = dt;
    act_a = entity_get_spec(ent_a);
    // get the entity for the ball
    pball = actor_get_spec(act_a);
    
    
    // if the ball is moving left
    if (actor_pball_velo_x(pball) < 0) {
      du = entity_check_left_list( ent_a, abs(actor_pball_velo_x(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, -du, 0 );

      ent_b = entity_check_result(null());
      if (ent_b) {
        list_add_last(playfield_events(self),"ball_hit_brick");
        actor_pball_velo_x(pball) = -actor_pball_velo_x(pball);
        act_b = entity_get_spec(ent_b);
        brick = actor_get_spec(act_b);
        actor_brick_set_id(brick,"");
      }
    }
    // if the ball is moving right
    else if (actor_pball_velo_x(pball) > 0) {
      du = entity_check_right_list( ent_a, abs(actor_pball_velo_x(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, du, 0 );

      ent_b = entity_check_result(null());
      if (ent_b) {
        list_add_last(playfield_events(self),"ball_hit_brick");
        actor_pball_velo_x(pball) = -actor_pball_velo_x(pball);
        act_b = entity_get_spec(ent_b);
        brick = actor_get_spec(act_b);
        actor_brick_set_id(brick,"");
      }
    }

    // if the ball is moving up
    if (actor_pball_velo_y(pball) < 0) {
      du = entity_check_up_list( ent_a, abs(actor_pball_velo_y(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, 0, -du );

      ent_b = entity_check_result(null());
      if (ent_b) {
        list_add_last(playfield_events(self),"ball_hit_brick");
        actor_pball_velo_y(pball) = -actor_pball_velo_y(pball);
        act_b = entity_get_spec(ent_b);
        brick = actor_get_spec(act_b);
        actor_brick_set_id(brick,"");
      }
    }
    // if the ball is moving down
    else if (actor_pball_velo_y(pball) > 0) {
      du = entity_check_down_list( ent_a, abs(actor_pball_velo_y(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, 0, du );
      
      ent_b = entity_check_result(null());
      if (ent_b) {
        list_add_last(playfield_events(self),"ball_hit_brick");
        actor_pball_velo_y(pball) = -actor_pball_velo_y(pball);
        act_b = entity_get_spec(ent_b);
        brick = actor_get_spec(act_b);
        actor_brick_set_id(brick,"");
      }

      // check against paddle
      du = entity_check_down( ent_a, abs(actor_pball_velo_y(pball)), actor_paddle_entity(paddle) );
      ent_b = entity_check_result(null());
      
      if (ent_b) {
        list_add_last(playfield_events(self),"ball_hit_paddle");
        actor_pball_velo_y(pball) = -abs(actor_pball_velo_y(pball));
      }
    }

    actor_pball_update(pball);
  }

  actor_paddle_update(playfield_paddle(self));

  // check events.
  char * se;
  foreach( playfield_events(self), dt ) {
    se = dt;
    log("event: %s",se);
  }
}

void playfield_draw( playfield_t * self ) {
  entity_t * e;
  actor_t  * a;
  // draw the bricks
  foreach( playfield_bricks(self), dt ) {
    e = dt;
    a = entity_get_spec(e);
    actor_brick_draw(actor_get_spec(a));
  }

  foreach( playfield_balls(self), dt ) {
    e = dt;
    a = entity_get_spec(e);
    actor_pball_draw(actor_get_spec(a));
  }

  actor_paddle_draw( playfield_paddle(self) );

}