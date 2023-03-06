
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t            * bricks; // entities
  list_t            * balls;  // entities
  entity_paddle_t   * paddle;

} playfield_t;

#define playfield_bricks(self)     (self->bricks)
#define playfield_balls(self)      (self->balls)
#define playfield_paddle(self)     (self->paddle)


/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  point_t sz                = playfield_bricks_size();
  playfield_bricks(self)    = list();

  entity_brick_t * b;
  loop(i,sz.x) {
    loop(j,sz.y) {
      // set all bricks to blank bricks
      b = entity_brick();
      
      entity_set_pos( entity_brick_entity(b), 100+(i*20), (j*10) );
      entity_set_solid( entity_brick_entity(b), true() );

      // store the entity, not the brick
      list_add_last( playfield_bricks(self), entity_brick_entity(b) );
    }
  }

  // make the list of balls
  playfield_balls(self)     = list();
  playfield_paddle(self)    = entity_paddle();
}

playfield_t * playfield() {
  playfield_t * self = alloc(playfield_t);
  init_playfield(self);
  return self;
}

void free_playfield( playfield_t * self ) {
  entity_t       * e;
  entity_pball_t * eb;
  entity_brick_t * et;
  // remove all balls first
  while (list_count( playfield_balls(self) ) > 0) {
    e = list_remove_first( playfield_balls(self) );
    eb = entity_get_spec(e);
    free_entity_pball(eb);
  }
  // now free the list
  free_list( playfield_balls(self) );

  while (list_count( playfield_balls(self) ) > 0) {
    e = list_remove_first( playfield_bricks(self) );
    et = entity_get_spec(e); 
    free_entity_brick(et);
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
  entity_t * ent_a;
  // brick's entity
  entity_t * ent_b;
  // ref to current ball
  entity_pball_t * pball;
  // ref to current brick
  entity_brick_t * brick;
  // result for glide functions
  int du;

  foreach( playfield_balls(self), dt ) {
    // cast the void pointer to a pball object
    ent_a = dt;
    // get the entity for the ball
    pball = entity_get_spec(ent_a);
    
    
    // if the ball is moving left
    if (entity_pball_velo_x(pball) < 0) {
      du = entity_check_left_list( ent_a, abs(entity_pball_velo_x(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, -du, 0 );

      ent_b = entity_check_result(null());
      if (ent_b) {
        entity_pball_velo_x(pball) = -entity_pball_velo_x(pball);
        brick = entity_get_spec(ent_b);
        entity_brick_set_id(brick,"");
      }
    }
    // if the ball is moving right
    else if (entity_pball_velo_x(pball) > 0) {
      du = entity_check_right_list( ent_a, abs(entity_pball_velo_x(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, du, 0 );

      ent_b = entity_check_result(null());
      if (ent_b) {
        entity_pball_velo_x(pball) = -entity_pball_velo_x(pball);
        brick = entity_get_spec(ent_b);
        entity_brick_set_id(brick,"");
      }
    }

    // if the ball is moving up
    if (entity_pball_velo_y(pball) < 0) {
      du = entity_check_up_list( ent_a, abs(entity_pball_velo_y(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, 0, -du );

      ent_b = entity_check_result(null());
      if (ent_b) {
        entity_pball_velo_y(pball) = -entity_pball_velo_y(pball);
        brick = entity_get_spec(ent_b);
        entity_brick_set_id(brick,"");
      }
    }
    // if the ball is moving down
    else if (entity_pball_velo_y(pball) > 0) {
      du = entity_check_down_list( ent_a, abs(entity_pball_velo_y(pball)), playfield_bricks(self) );
      entity_add_pos( ent_a, 0, du );

      ent_b = entity_check_result(null());
      if (ent_b) {
        entity_pball_velo_y(pball) = -entity_pball_velo_y(pball);
        brick = entity_get_spec(ent_b);
        entity_brick_set_id(brick,"");
      }
    }

    entity_pball_update(pball);
  }
}

void playfield_draw( playfield_t * self ) {
  entity_t * e;
  // draw the bricks
  foreach( playfield_bricks(self), dt ) {
    e = dt;
    entity_brick_draw(entity_get_spec(e));
  }

  foreach( playfield_balls(self), dt ) {
    e = dt;
    entity_pball_draw(entity_get_spec(e));
  }

}