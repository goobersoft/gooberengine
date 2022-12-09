
type() {

  // reference to colormap
  colormap_t  * colormap;
  // texture rect
  rect_t      * rect;
  // offset from topleft
  point_t     * origin;

  field(flip_x,bool_t,1);
  field(flip_y,bool_t,1);

} sprite_t;

#define sprite_colormap(self) (self->colormap)
#define sprite_rect(self)     (self->rect)
#define sprite_origin(self)   (self->origin)
#define sprite_flip_x(self)   (self->flip_x)
#define sprite_flip_y(self)   (self->flip_y)

#define sprite_width(self)    rect_w(sprite_rect(self)) 
#define sprite_height(self)   rect_h(sprite_rect(self))

/////////
// new //
/////////

void sprite_init( sprite_t * self, colormap_t * c, int cx, int cy, int cw, int ch ) {
  sprite_colormap(self)  = c;
  sprite_rect(self)      = rect(cx,cy,cw,ch);
  sprite_origin(self)    = point(0,0);
  sprite_flip_x(self)    = false();
  sprite_flip_y(self)    = false();
}

sprite_t * sprite( colormap_t * c, int cx, int cy, int cw, int ch ) {
  sprite_t * r        = alloc(sprite_t);
  sprite_init(r,c,cx,cy,cw,ch);
  return r;
}

sprite_t * clone_sprite( sprite_t * o ) {
  sprite_t * r       = alloc(sprite_t);
  sprite_colormap(r) = sprite_colormap(o);
  sprite_rect(r)     = clone_rect(sprite_rect(o));
  sprite_origin(r)   = clone_point(sprite_origin(o));
  sprite_flip_x(r)   = sprite_flip_x(o);
  sprite_flip_y(r)   = sprite_flip_y(o);
  return r;
}

void free_sprite( sprite_t * self ) {
  // does not free colormap - this is an external pointer
  free(sprite_rect(self));
  free(sprite_origin(self));
  free(self);
}

///////////
// funcs //
///////////

// because sprite is defined BEFORE graph, the graph module will
// have functions for drawing the sprite.

void sprite_set_colormap( sprite_t * self, colormap_t * c ) {
  sprite_colormap(self) = c;
}

void sprite_set_rect( sprite_t * self, int x, int y, int w, int h ) {
  rect_set(sprite_rect(self),x,y,w,h);
}

void sprite_set_rect_pos( sprite_t * self, int x, int y ) {
  rect_set_pos(sprite_rect(self), x, y);
}

void sprite_set_rect_size( sprite_t * self, int w, int h ) {
  rect_set_size(sprite_rect(self), w, h);
}

void sprite_set_origin( sprite_t * self, int x, int y ) {
  point_set( sprite_origin(self), x, y );
}