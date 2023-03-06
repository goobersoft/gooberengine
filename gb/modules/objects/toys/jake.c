
#define jake_delay_max()      10
#define jake_anim_index_max() 2
#define jake_cmap_x()         100
#define jake_cmap_y()         190
#define jake_cmap_size()      20

type() {

  local( point_t * pos );

  int          delay;
  int          anim_index;
  
  foreign( colormap_t * colormap );
  foreign( graph_t    * graph );

} jake_t;

#define jake_pos(self)        (self->pos)
#define jake_delay(self)      (self->delay)
#define jake_anim_index(self) (self->anim_index)
#define jake_colormap(self)   (self->colormap)
#define jake_graph(self)      (self->graph)

/////////
// new //
/////////

void jake_init( jake_t * self ) {
  jake_pos(self)        = point(400,rnd(0,220));
  jake_delay(self)      = jake_delay_max();
  jake_anim_index(self) = 0;
  jake_colormap(self)   = gb_get_colormap("gb-0");
}

jake_t * jake( ) {
  jake_t * self = alloc(jake_t);
  jake_init( self );
  return self;
}

////////////
// events //
////////////

void jake_update( jake_t * self ) {
  jake_delay(self) -= 1;
  
  if (jake_delay(self) == 0) {
    jake_anim_index(self) = (jake_anim_index(self) + 1) % jake_anim_index_max();
    jake_delay(self) = jake_delay_max();
    point_x(jake_pos(self)) -= 1;
    if ( point_x(jake_pos(self)) <= -20 ) {
      
      point_x(jake_pos(self)) = 400;
      point_y(jake_pos(self)) = rnd(0,220);
    }
  }
}



void jake_draw( jake_t * self ) {
  if jake_colormap(self) {
    graph_draw_colormap_sub( 
      gb_graph(), point_x(jake_pos(self)), point_y(jake_pos(self)), 
      jake_colormap(self), jake_cmap_x()+(jake_cmap_size()*jake_anim_index(self)), jake_cmap_y(), 
      jake_cmap_size(), jake_cmap_size()
    );
  }
}