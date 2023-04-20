
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t    * bricks;
  list_t    * balls;
  paddle_t  * paddle;
  list_t    * events;

} playfield_t;

#define playfield_bricks(self)      (self->bricks)
#define playfield_balls(self)       (self->balls)
#define playfield_paddle(self)      (self->paddle)
#define playfield_events(self)      (self->events)

#define playfield_move_paddle_left(self)  paddle_move_left(playfield_paddle(self))
#define playfield_move_paddle_right(self) paddle_move_right(playfield_paddle(self))

/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  playfield_events(self)      = list();
  point_t sz                  = playfield_bricks_size();
  playfield_bricks(self)      = list();
  playfield_balls(self)       = list();
  playfield_paddle(self)      = paddle();

  brick_t * b;

  loop(i,sz.x) {
    loop(j,sz.y) {
      // set all bricks to blank bricks
      b  = brick();
      entity_set_pos( brick_entity(b), 100000+(i*20000), (j*10000) );
      entity_set_solid( brick_entity(b), true() );
      list_add_last( playfield_bricks(self), b );
    }
  }
  
}

playfield_t * playfield(void) {
  playfield_t * self = alloc(playfield_t);
  init_playfield(self);
  return self;
}

void free_playfield( playfield_t * self ) {
  
  pball_t  * ab;
  while (list_count( playfield_balls(self) ) > 0) {
    ab = list_remove_first( playfield_balls(self) );
    free_pball(ab);
  }
  free_list( playfield_balls(self) );

  brick_t  * at;
  while (list_count( playfield_balls(self) ) > 0) {
    at  = list_remove_first( playfield_bricks(self) );
    free_brick(at);
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
  pball_t * b = pball();
  list_add_last( playfield_balls(self), b );
}

void playfield_set_brick( playfield_t * self, int bx, int by, char * b) {
  int u = 0;
  foreach( playfield_bricks(self), dt ) {
    if (u == (by/20)+(bx%20)) {
      brick_set_id(dt,b);
      return;
    }
    u += 1;
  }
}

////////////
// events //
////////////

void playfield_update( playfield_t * self ) {
  list_clear( playfield_events(self) );

  // ref to current ball
  pball_t  * pball;
  // ref to current brick
  brick_t  * brick;
  //
  paddle_t * paddle = playfield_paddle(self);
  // result for glide functions
  int dx;
  int ax;
  int dy;
  int ay;

  foreach( playfield_balls(self), dt ) {
    pball = dt;

    foreach( playfield_bricks(self), dt2 ) {
      brick = dt2;

      // if the ball is moving left
      if (pball_velo_x(pball) < 0) {
        ax = -1;
        dx = entity_check_left( pball_entity(pball), abs(pball_velo_x(pball)), 
          brick_entity(brick) );

        if (entity_check_result(null())) {
          list_add_last(playfield_events(self),"ball_hit_brick");
          pball_velo_x(pball) = -pball_velo_x(pball);
          brick_set_id(brick,"");
        }
      }
      // if the ball is moving right
      else if (pball_velo_x(pball) > 0) {
        ax = 1;
        dx = entity_check_right( pball_entity(pball), abs(pball_velo_x(pball)), 
          brick_entity(brick) );

        if (entity_check_result(null())) {
          list_add_last(playfield_events(self),"ball_hit_brick");
          pball_velo_x(pball) = -pball_velo_x(pball);
          brick_set_id(brick,"");
        }
      }

      // if the ball is moving up
      if (pball_velo_y(pball) < 0) {
        ay = -1;
        dy = entity_check_up( pball_entity(pball), abs(pball_velo_y(pball)), 
          brick_entity(brick) );

        if (entity_check_result(null())) {
          list_add_last(playfield_events(self),"ball_hit_brick");
          pball_velo_y(pball) = -pball_velo_y(pball);
          brick_set_id(brick,"");
        }
      }
      // if the ball is moving down
      else if (pball_velo_y(pball) > 0) {
        ay = 1;
        dy = entity_check_down( pball_entity(pball), abs(pball_velo_y(pball)), 
          brick_entity(brick) );
        
        if (entity_check_result(null())) {
          list_add_last(playfield_events(self),"ball_hit_brick");
          pball_velo_y(pball) = -pball_velo_y(pball);
          brick_set_id(brick,"");
        } 
      }
    }
    // check against paddle
    if (pball_velo_y(pball)>0) {
      dy = entity_check_down( pball_entity(pball), abs(pball_velo_y(pball)),
        paddle_entity(paddle) );
      if (entity_check_result(null())) {
        list_add_last(playfield_events(self),"ball_hit_paddle");
        pball_velo_y(pball) = -abs(pball_velo_y(pball));
        ay = -1;
      }
    }
      
    
    
    entity_add_pos(pball_entity(pball),dx*ax,dy*ay);

    pball_update(pball);
    
  }

  paddle_update(playfield_paddle(self));
}

void playfield_draw( playfield_t * self ) {
  // draw the bricks
  foreach( playfield_bricks(self), dt ) {
    brick_draw(dt);
  }

  foreach( playfield_balls(self), dt ) {
    pball_draw(dt);
  }

  paddle_draw( playfield_paddle(self) );
}