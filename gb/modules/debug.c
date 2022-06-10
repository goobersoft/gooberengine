

/////////////
// globals //
/////////////

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  assets_load_image( gb_assets(), 4, "ma/media/images/ma-0.png" );
}

void debug_update() {
  if (prob(1)) log("%d",timing_fps(gb_timing()));
}

void debug_draw_pre() {
  
}

void debug_draw_post() {
  //graph_set_mode           ( gb_graph(), graph_mode_sub() );
  //graph_set_color          ( gb_graph(), color(1,1,1) );
  //graph_draw_rect_spray    ( gb_graph(), 0, 0, 400, 240, 100 );
  loop(i,0,300) {
    graph_set_mode           ( gb_graph(), graph_mode_normal() );
    graph_set_color          ( gb_graph(), color(3,3,3) );
    graph_draw_triangle_line ( gb_graph(), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240), rnd(0,400), rnd(0,240) );
  }
  if (prob(1)) log("%d",graph_frame_dots(gb_graph()));
}

void debug_quit() {
}