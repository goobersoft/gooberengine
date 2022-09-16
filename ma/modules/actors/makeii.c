
type() {

  bool_t      active;
  // ref to actor object
  actor_t   * actor;
  // life points
  number_t  * lp;
  // ability points
  number_t  * ap;

} makeii_t;

#define makeii_active(self) (self->active)
#define makeii_actor(self)  (self->actor)
#define makeii_lp(self)     (self->lp)
#define makeii_ap(self)     (self->ap)

#define makeii_pos(self)     entity_pos(actor_entity(makeii_actor(self)))
#define makeii_pos_x(self)   point_x(entity_pos(actor_entity(makeii_actor(self))))
#define makeii_pos_y(self)   point_y(entity_pos(actor_entity(makeii_actor(self))))
#define makeii_velo(self)    actor_velo(makeii_actor(self))
#define makeii_velo_x(self)  point_x(actor_velo(makeii_actor(self)))
#define makeii_velo_y(self)  point_y(actor_velo(makeii_actor(self)))
#define makeii_gravity(self) actor_gravity(makeii_actor(self))

/////////
// new //
/////////

makeii_t * makeii() {
  makeii_t * r      = alloc(makeii_t);
  makeii_active(r)  = true();
  // define the actor stuff

  makeii_actor(r)   = actor( assets_get_colormap( gb_assets(), "ma-0" ) );
  actor_set_dir( makeii_actor(r), actor_dir_east() );
  actor_set_sprite_rect(makeii_actor(r), 70, 210, 30, 30 );

  makeii_lp(r)      = number(8,0,8);
  makeii_ap(r)      = number(8,0,8);
  return r;
}

///////////////
// functions //
///////////////

////////////
// events //
////////////

void makeii_update( makeii_t * self ) {
  // apply gravity to velocity
  point_add( makeii_velo(self), 0, makeii_gravity(self) );
  // apply velocity to position
  point_add_p( makeii_pos(self), makeii_velo(self) );
  // check if outside bounds of screen
  // (makeii's sprite is 30x30)
  

  if (controller_is_button_pressed( gb_controller(), controller_button_a() )) {
    makeii_velo_y(self) = -5000;
  }

  if (controller_is_button_held( gb_controller(), controller_button_left() )) {
    makeii_pos_x(self) -= 2500;
  }
  else if (controller_is_button_held( gb_controller(), controller_button_right() )) {
    makeii_pos_x(self) += 2500;
  }

  point_clamp(makeii_pos(self), 0, 0, 400000-30000, 240000-30000);
  
}

void makeii_draw( makeii_t * self ) {
  int u = graph_set_intensity_max( gb_graph() );
  graph_draw_sprite( gb_graph(), makeii_pos_x(self)/1000, makeii_pos_y(self)/1000, actor_sprite(makeii_actor(self)));
  graph_set_intensity( gb_graph(), u );
}