

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

void debug_draw_pre() {
  if (prob(1)) {
    graph_set_cls_color( gb_graph(), color(rnd(0,3),rnd(0,3),rnd(0,3)) );
    graph_cls( gb_graph() );
  }
  graph_draw_colormap( gb_graph(), debug_colormap, 10, 10 );
  graph_set_color( gb_graph(), color_random() );
  graph_draw_circle_line( gb_graph(), rnd(0,400), rnd(0,240), rnd(5,20) );
  graph_draw_rect_line( gb_graph(), rnd(0,400), rnd(0,240), rnd(5,20), rnd(5,20) );
  graph_draw_image_sub( gb_graph(), debug_image, rnd(0,400), rnd(0,240), tile10(rnd(0,40)), tile10(rnd(0,24)), 10, 10 );
}

void debug_draw_post() {

}

void debug_quit() {
}