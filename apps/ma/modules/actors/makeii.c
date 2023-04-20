


type() {

  // tag
  local( tag_t * tag );
  // ref to ma_actor object
  local( ma_actor_t * actor );
  // life points
  local( number_t * lp );
  // ability points
  local( number_t * ap );

  bool_t       f_ducking;

} ma_makeii_t;


#define ma_makeii_tag(self)           (self->tag)
#define ma_makeii_actor(self)         (self->actor)
#define ma_makeii_lp(self)            (self->lp)
#define ma_makeii_ap(self)            (self->ap)

#define ma_makeii_flag_active(self)   tag_flag_active(ma_makeii_tag(self))
#define ma_makeii_flag_visible(self)  tag_flag_visible(ma_makeii_tag(self))
#define ma_makeii_f_ducking(self)  (self->f_ducking)
#define ma_makeii_pos(self)           entity_pos(ma_actor_entity(ma_makeii_actor(self)))
#define ma_makeii_pos_x(self)         point_x(entity_pos(ma_actor_entity(ma_makeii_actor(self))))
#define ma_makeii_pos_y(self)         point_y(entity_pos(ma_actor_entity(ma_makeii_actor(self))))
#define ma_makeii_velo(self)          ma_actor_velo(ma_makeii_actor(self))
#define ma_makeii_velo_x(self)        point_x(ma_actor_velo(ma_makeii_actor(self)))
#define ma_makeii_velo_y(self)        point_y(ma_actor_velo(ma_makeii_actor(self)))
#define ma_makeii_gravity(self)       ma_actor_gravity(ma_makeii_actor(self))

#define ma_makeii_set_active(self)    tag_set_active(ma_makeii_tag(self))
#define ma_makeii_set_visible(self)   tag_set_visible(ma_makeii_tag(self))
#define ma_makeii_set_lp(self,n)      number_set(ma_makeii_lp(self),n)
#define ma_makeii_add_lp(self,n)      number_add(ma_makeii_lp(self),n)
#define ma_makeii_set_ap(self,n)      number_set(ma_makeii_ap(self),n)
#define ma_makeii_add_ap(self,n)      number_add(ma_makeii_ap(self),n)
#define ma_makeii_set_dir(self,n)     ma_actor_set_dir(ma_makeii_ma_actor(self))

/////////
// new //
/////////

void ma_makeii_init( ma_makeii_t * self ) {
  // make a new tag with class "ma_makeii" and no ID
  ma_makeii_tag(self)          = tag(self,"makeii");
  // create the ma_actor
  ma_makeii_actor(self)        = ma_actor( assets_get_colormap( gb_assets(), "ma-0" ), self );
  // set the ma_actor dir
  ma_actor_set_dir             ( ma_makeii_actor(self), ma_actor_dir_east() );
  // define the sprite source rectangle
  ma_actor_set_sprite_rect     ( ma_makeii_actor(self), 70, 210, 30, 30 );

  // define the life and ability points for makeii
  ma_makeii_lp(self)      = number(8,0,8);
  ma_makeii_ap(self)      = number(8,0,8);
}

ma_makeii_t * ma_makeii() {
  ma_makeii_t * r = alloc(ma_makeii_t);
  ma_makeii_init(r);
  return r;
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void ma_makeii_update( ma_makeii_t * self ) {
  // apply gravity to velocity
  point_add( ma_makeii_velo(self), 0, ma_makeii_gravity(self) );
  // apply velocity to position
  point_add_p( ma_makeii_pos(self), ma_makeii_velo(self) );
  // check if outside bounds of screen
  // (ma_makeii's sprite is 30x30)
  

  // handle jumping
  if (controller_is_button_pressed( gb_controller(), controller_button_a() )) {
    ma_makeii_velo_y(self) = -5000;
  }

  // handle moving left and right
  if (controller_is_button_held( gb_controller(), controller_button_left() )) {
    ma_makeii_pos_x(self) -= 2500;
    sprite_flip_x(ma_actor_sprite(ma_makeii_actor(self))) = true();
  }
  else if (controller_is_button_held( gb_controller(), controller_button_right() )) {
    ma_makeii_pos_x(self) += 2500;
    sprite_flip_x(ma_actor_sprite(ma_makeii_actor(self))) = false();
  }

  // control the ducking behavior of ma_makeii
  if (controller_is_button_held( gb_controller(), controller_button_down())) {
    ma_makeii_f_ducking(self) = true();
  }
  else {
    ma_makeii_f_ducking(self) = false();
  }

  // clamp the position of ma_makeii inside of the screen
  point_clamp(ma_makeii_pos(self), 0, 0, 400000-30000, 240000-30000);
  
}

void ma_makeii_draw( ma_makeii_t * self ) {
  int u = graph_set_intensity_max( gb_graph() );
  graph_draw_sprite    ( gb_graph(), ma_makeii_pos_x(self)/1000, ma_makeii_pos_y(self)/1000, ma_actor_sprite(ma_makeii_actor(self)));
  graph_set_intensity  ( gb_graph(), u );
}