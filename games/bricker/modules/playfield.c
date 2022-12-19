
#define bricker_playfield_bounds()        make_rect(100,0,200,240)
#define bricker_playfield_bricks_size()   make_point(10,16)

type() {

  array2d_t   * bricks;
  list_t      * balls;


} playfield_t;

#define playfield_bricks(self) (self->bricks)

/////////
// new //
/////////

void init_playfield( playfield_t * self ) {
  point_t sz              = bricker_playfield_bricks_size()
  playfield_bricks(self)  = array2d(sz.x, sz.y);

  loop(i,sz.x) {
    loop(j,sz.y) {
      a2set( playfield_bricks(self), i, j, brick() );
    }
  }
}

playfield_t * playfield() {
  playfield_t * self = alloc(playfield_t);
  init_playfield(self);
  return self;
}