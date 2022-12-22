
#define playfield_bounds()        make_rect(100,0,200,240)
#define playfield_bricks_size()   make_point(10,20)

type() {

  list_t      * bricks;
  list_t      * balls;
  paddle_t    * paddle;


} playfield_t;

#define playfield_bricks(self) (self->bricks)
#define playfield_balls(self)  (self->balls)
#define playfield_paddle(self) (self->paddle)

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

      int uu = rnd(1,6);
      if (uu==1)      brick_set_id(b,"1");
      else if (uu==2) brick_set_id(b,"2");
      else if (uu==3) brick_set_id(b,"3");
      else if (uu==4) brick_set_id(b,"4");
      else if (uu==5) brick_set_id(b,"5");
      else if (uu==6) brick_set_id(b,"6");

      entity_set_pos( brick_entity(b), 100+(i*20), (j*10) );
      entity_set_solid( brick_entity(b), true() );
      list_add_last( playfield_bricks(self), b );
    }
  }

  playfield_balls(self)   = list();
    list_add_last( playfield_balls(self), pball() );

  playfield_paddle(self)  = paddle();
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

////////////
// events //
////////////

void playfield_update( playfield_t * self ) {

  // check if any of the balls are colliding with any bricks

  // get the number of balls in the playfield
  int bc = list_count( playfield_balls(self) );
  // get the number of bricks in the playfield
  int pc = list_count( playfield_bricks(self) );

  // ball index
  int bi = 0;
  // brick index
  int pi = 0;

  // ball object
  pball_t * bo;
  // brick object
  brick_t * po;

  // ball entity
  entity_t * be;
  // brick entity
  entity_t * pe;

  while (bi < bc) {

    // get the 
    bo = list_get_first( playfield_balls(self) );
    pball_update(bo);

    be = pball_entity(bo);

    // set brick counter to 0;
    pi = 0;
    while (pi < pc) {

      po = list_get_first( playfield_bricks(self) );
      pe = brick_entity(po);

      if ( entity_collide( be, pe ) ) {
        brick_set_id( po, "" );
        //point_mul( pball_velo(bo), -1, -1 );
        point_set( pball_pos(bo), 200, 200 );
      }

      list_rotate_next( playfield_bricks(self) );
      pi += 1;
    }

    // rotate the list to the next element
    list_rotate_next( playfield_balls(self) );
    // increase ball index
    bi += 1;
  }

}

void playfield_draw( playfield_t * self ) {

  point_t p = playfield_bricks_size();

  // draw the bricks
  brick_t * b;
  foreach( playfield_bricks(self), dt ) {
    b = cast(dt,brick_t*);
    brick_draw(b);
  }

  foreach( playfield_balls(self), dt ) {
    pball_draw( dt );
  }

}