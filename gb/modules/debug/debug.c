
//////////
// type //
//////////

type() {

  debugpanel_t * panel;
  colormap_t * colormap;


  entity_menu_t * entmenu;


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

  
  debug.entmenu = entity_menu(4);

}

void debug_update_pre() {
  debugpanel_update(debug.panel);
}

void debug_update_post() {
}

void debug_draw_pre() {
}

void debug_draw_post() {
  debugpanel_draw(debug.panel);
  entity_menu_draw(debug.entmenu);
}

void debug_quit() {
}