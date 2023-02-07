
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t      * bricks;
  list_t      * brickents;
  list_t      * balls;
  paddle_t    * paddle;

} playfield_t;

#define playfield_bricks(self)     (self->bricks)
#define playfield_brickents(self)  (self->brickents)
#define playfield_balls(self)      (self->balls)
#define playfield_paddle(self)     (self->paddle)


/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  point_t sz                = playfield_bricks_size();
  playfield_bricks(self)    = list();
  playfield_brickents(self) = list();

  brick_t * b;
  loop(i,sz.x) {
    loop(j,sz.y) {
      // set all bricks to blank bricks
      b = brick();
      
      entity_set_pos( brick_entity(b), 100+(i*20), (j*10) );
      entity_set_solid( brick_entity(b), true() );

      // store the entity, not the brick
      list_add_last( playfield_bricks(self), b );
      list_add_last( playfield_brickents(self), brick_entity(b) );
    }
  }

  // make the list of balls
  playfield_balls(self)     = list();
  playfield_paddle(self)    = paddle();
}

playfield_t * playfield() {
  playfield_t * self = alloc(playfield_t);
  init_playfield(self);
  return self;
}

void free_playfield( playfield_t * self ) {
  void * u;
  // remove all balls first
  while (list_count( playfield_balls(self) ) > 0) {
    u = list_remove_first( playfield_balls(self) );
    free_pball(u);
  }
  // now free the list
  free_list( playfield_balls(self) );

  while (list_count( playfield_balls(self) ) > 0) {
    u = list_remove_first( playfield_bricks(self) );
    free_brick(u);
  }
  free_list( playfield_bricks(self) );
}

///////////////
// functions //
///////////////

// x: x pos of ball (relative to playfield)
// y: y pos of ball (relative to playfield)
// t: the type of ball (future feature?)
void playfield_add_ball( int x, int y, int t ) {

}

void playfield_set_brick( int bx, int by, int b ) {

}

////////////
// events //
////////////

void playfield_update( playfield_t * self ) {

  // ball's entity
  entity_t * ea;
  // brick's entity
  entity_t * eb;
  // ref to current ball
  pball_t * ba;
  // ref to current brick
  brick_t * bb;
  // result for glide functions
  int du;

  foreach( playfield_balls(self), bat ) {
    // cast the void pointer to a pball object
    ba = cast(bat,pball_t*);
    // get the entity for the ball
    ea = pball_entity(ba);
    
    
    // if the ball is moving left
    if (pball_velo_x(ba) < 0) {
      du = entity_check_left_list( ea, abs(pball_velo_x(ba)), playfield_brickents(self) );
      entity_add_pos( ea, -du, 0 );

      eb = entity_check_result(null());
      if (eb) {
        pball_velo_x(ba) = -pball_velo_x(ba);
        bb = entity_source(eb);
        brick_set_id(bb,"");
      }
    }
    // if the ball is moving right
    else if (pball_velo_x(ba) > 0) {
      du = entity_check_right_list( ea, abs(pball_velo_x(ba)), playfield_brickents(self) );
      entity_add_pos( ea, du, 0 );

      eb = entity_check_result(null());
      if (eb) {
        pball_velo_x(ba) = -pball_velo_x(ba);
        bb = entity_source(eb);
        brick_set_id(bb,"");
      }
    }

    // if the ball is moving up
    if (pball_velo_y(ba) < 0) {
      du = entity_check_up_list( ea, abs(pball_velo_y(ba)), playfield_brickents(self) );
      entity_add_pos( ea, 0, -du );

      eb = entity_check_result(null());
      if (eb) {
        pball_velo_y(ba) = -pball_velo_y(ba);
        bb = entity_source(eb);
        brick_set_id(bb,"");
      }
    }
    // if the ball is moving down
    else if (pball_velo_y(ba) > 0) {
      du = entity_check_down_list( ea, abs(pball_velo_y(ba)), playfield_brickents(self) );
      entity_add_pos( ea, 0, du );

      eb = entity_check_result(null());
      if (eb) {
        pball_velo_y(ba) = -pball_velo_y(ba);
        bb = entity_source(eb);
        brick_set_id(bb,"");
      }
    }

    pball_update(ba);
  }
}

void playfield_draw( playfield_t * self ) {
  // draw the bricks
  foreach( playfield_bricks(self), dt ) {
    brick_draw(dt);
  }

  foreach( playfield_balls(self), dt ) {
    pball_draw(dt);
  }

}