

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
image_t *    debug_image;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  image_t * uu = assets_get_image(gb_assets(),3);
  debug_colormap = colormap_from_image( uu );
  //debug_colormap = colormap(100,100);
  debug_image = assets_get_image(gb_assets(),0);
}

void debug_update() {
  //log("%d\n",timing_fps(gb_timing()));
}

/*
void debug_draw_pre() {
  if (chance(1,1000)) {
    graph_set_cls_color( gb_graph(), color(rnd(0,3),rnd(0,3),rnd(0,3)) );
    graph_cls( gb_graph() );
  }
  graph_draw_colormap( gb_graph(), debug_colormap, 10, 10 );
  graph_set_color( gb_graph(), color_random() );
  graph_draw_circle_line( gb_graph(), rnd(0,400), rnd(0,240), rnd(5,20) );
  graph_draw_rect_line( gb_graph(), rnd(0,400), rnd(0,240), rnd(5,20), rnd(5,20) );
  graph_draw_image_sub( gb_graph(), debug_image, rnd(0,400), rnd(0,240), tile10(rnd(0,40)), tile10(rnd(0,24)), 10, 10 );
}
*/

void debug_draw_pre() {
  //graph_cls( gb_graph() );
  graph_set_color( gb_graph(), 0 );
  graph_draw_rect_spray( gb_graph(), 0, 0, 400, 240, 1 );
  
  static int x  = 0;
  static int x2 = 0;
  static int c  = 0;
  static int cs = 0;
  x = wrap(x+5+prob(50),0,1000);
  x2 = wrap(x2+1+prob(50),0,1000);
  cs += 1;
  if (cs==1) {
    cs = 0;
    c = wrap(c+1,1,4);
  }
  graph_set_color( gb_graph(), color(0,3,0) );
  graph_draw_circle_dots( gb_graph(), 200, 120, 60 + frac(sine(x2),1000,60), 3, x );
}

void debug_draw_post() {

}

void debug_quit() {
}