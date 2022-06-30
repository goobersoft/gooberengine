
#define jake_delay_max()      10
#define jake_anim_index_max() 2
#define jake_cmap_x()         100
#define jake_cmap_y()         190
#define jake_cmap_size()      20

type() {

  point_t    * pos;
  int          delay;
  int          anim_index;
  colormap_t * colormap;

} jake_t;

#define jake_pos(self)        (self->pos)
#define jake_delay(self)      (self->delay)
#define jake_anim_index(self) (self->anim_index)
#define jake_colormap(self)   (self->colormap)

/////////
// new //
/////////

void jake_init( jake_t * self, colormap_t * c ) {
  jake_pos(self)        = point(400,rnd(0,220));
  jake_delay(self)      = jake_delay_max();
  jake_anim_index(self) = 0;
  jake_colormap(self)   = c;
}

jake_t * jake( colormap_t * c ) {
  jake_t * r = alloc(jake_t);
  jake_init( r, c );
  return r;
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



void graph_draw_jake( graph_t * self, jake_t * j ) {
  if (jake_colormap(j)) {
    graph_draw_colormap_sub( 
      self, point_x(jake_pos(j)), point_y(jake_pos(j)), 
      jake_colormap(j), jake_cmap_x()+(jake_cmap_size()*jake_anim_index(j)), jake_cmap_y(), 
      jake_cmap_size(), jake_cmap_size()
    );
  }
}