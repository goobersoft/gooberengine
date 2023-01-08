
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t      * bricks;
  list_t      * balls;
  paddle_t    * paddle;

} playfield_t;

#define playfield_bricks(self)    (self->bricks)
#define playfield_balls(self)     (self->balls)
#define playfield_paddle(self)    (self->paddle)

/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  point_t sz              = playfield_bricks_size();
  playfield_bricks(self)  = list();

  brick_t * b;
  loop(i,sz.x) {
    loop(j,sz.y) {
      // set all bricks to blank bricks
      b = brick();
      
      entity_set_pos( brick_entity(b), 100+(i*20), (j*10) );
      entity_set_solid( brick_entity(b), true() );

      // store the entity, not the brick
      list_add_last( playfield_bricks(self), brick_entity(b) );
    }
  }

  // make the list of balls
  playfield_balls(self)   = list();
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

  entity_t * e1;
  entity_t * e2;
  pball_t * ba;
  brick_t * br;
  int du;

  foreach( playfield_balls(self), bat ) {
    // these are entities, need to cast first
    e1  = cast(bat,entity_t*);
    
    // get the ball object from the entity's source.
    ba  = entity_source(e1);
    

    if (pball_velo_x(ba) < 0) {
      du = entity_check_left_list( e1, abs(pball_velo_x(ba)), playfield_bricks(self) );
      entity_add_pos( e1, -du, 0 );

      e2 = entity_check_result(null());
      if (e2) {
        pball_velo_x(ba) = -pball_velo_x(ba);
        br = entity_source(e2);
        brick_set_id(br,"");
      }
    }
    else if (pball_velo_x(ba) > 0) {
      du = entity_check_right_list( e1, abs(pball_velo_x(ba)), playfield_bricks(self) );
      entity_add_pos( e1, du, 0 );

      e2 = entity_check_result(null());
      if (e2) {
        pball_velo_x(ba) = -pball_velo_x(ba);
        br = entity_source(e2);
        brick_set_id(br,"");
      }
    }

    if (pball_velo_y(ba) < 0) {
      du = entity_check_up_list( e1, abs(pball_velo_y(ba)), playfield_bricks(self) );
      entity_add_pos( e1, 0, -du );

      e2 = entity_check_result(null());
      if (e2) {
        pball_velo_y(ba) = -pball_velo_y(ba);
        br = entity_source(e2);
        brick_set_id(br,"");
      }
    }
    else if (pball_velo_y(ba) > 0) {
      du = entity_check_down_list( e1, abs(pball_velo_y(ba)), playfield_bricks(self) );
      entity_add_pos( e1, 0, du );

      e2 = entity_check_result(null());
      if (e2) {
        pball_velo_y(ba) = -pball_velo_y(ba);
        br = entity_source(e2);
        brick_set_id(br,"");
      }
    }

    pball_update(ba);
  }
}

void playfield_draw( playfield_t * self ) {

  entity_t * e;

  // draw the bricks
  
  brick_t * b;
  foreach( playfield_bricks(self), dt ) {
    e = cast(dt,entity_t*);
    b = entity_source(e);
    brick_draw(b);
  }

  pball_t * p;
  foreach( playfield_balls(self), dt ) {
    e = cast(dt,entity_t*);
    p = entity_source(e);
    pball_draw(p);
  }

}