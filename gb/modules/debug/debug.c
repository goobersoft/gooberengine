
//////////
// type //
//////////

type() {

  debugpanel_t * panel;
  colormap_t * colormap;

  entity_t  * ent1;
  list_t    * entl;

  int t;

} debug_t;

/////////////
// globals //
/////////////

debug_t debug;

////////////
// events //
////////////

void debug_init() {
  debug.panel   = debugpanel();
}

void debug_load() {
}


void debug_start() {
  debug.colormap = gb_get_colormap("bricker-0");
  
  debug.entl = list();
  entity_t * e;
  loop(i,10) {
    e = entity(null());
    entity_set_pos(e,rnd(0,380),rnd(0,220));
    entity_set_size(e,20,20);
    list_add_last(debug.entl, e);
  }

  debug.ent1 = entity(null());
  entity_set_pos(debug.ent1, 100, 100);
  entity_set_size(debug.ent1, 20, 20);
}

void debug_update_pre() {
  debugpanel_update(debug.panel);

  int u;
  if (gb_button(0)) {
    u = entity_check_up_list( debug.ent1, 1, debug.entl );
    entity_pos_y(debug.ent1) -= u;
  }
  else if (gb_button(1)) {
    u = entity_check_right_list( debug.ent1, 1, debug.entl );
    entity_pos_x(debug.ent1) += u;
  }
  else if (gb_button(2)) {
    u = entity_check_down_list( debug.ent1, 1, debug.entl );
    entity_pos_y(debug.ent1) += u;
  }
  else if (gb_button(3)) {
    u = entity_check_left_list( debug.ent1, 1, debug.entl );
    entity_pos_x(debug.ent1) -= u;
  }
  
}

void debug_update_post() {
}

void debug_draw_pre() {
  gb_cls();
  gb_set_intensity_max();

  gb_set_color( 3,3,3 );
  gb_draw_rect( entity_pos_x(debug.ent1), entity_pos_y(debug.ent1),
    entity_size_x(debug.ent1), entity_size_y(debug.ent1) );

  gb_set_color( 3,3,3 );
  entity_t * e;
  foreach( debug.entl, dt ) {
    e = cast(dt,entity_t*);
    gb_draw_rect( entity_pos_x(e), entity_pos_y(e),
      entity_size_x(e), entity_size_y(e) );
  }

  e = entity_check_result(null());
  if (e) {
    gb_set_color( 0,3,0 );
    gb_draw_rect( entity_pos_x(e), entity_pos_y(e),
      entity_size_x(e), entity_size_y(e) );
  }
  


  gb_reset_intensity();
}

void debug_draw_post() {
  debugpanel_draw(debug.panel);
}

void debug_quit() {
}