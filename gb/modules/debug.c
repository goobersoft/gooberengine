

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
image_t *    debug_image;
sound_t *    debug_sound;
list_t *     debug_list;
int          debug_data;

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
  //debug_colormap = colormap(100,100);
  debug_image = assets_get_image(gb_assets(),0);
  debug_colormap = colormap_from_image( debug_image );
}

void debug_update() {
  int rn;
  if (prob(1)) {
    rn = rnd(0,4);
    audio_play(gb_audio(),assets_get_sound(gb_assets(),rn),rn);
  }
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
  graph_set_mode( gb_graph(), graph_mode_stencil() );

}



/*
void debug_draw_pre() {
  graph_set_mode( gb_graph(), graph_mode_add() );
  graph_draw_colormap_sub( gb_graph(), debug_colormap, rnd(0,400), rnd(0,240), rnd(0,390), rnd(0,230), 10, 10 );
}
*/
void debug_draw_post() {
  graph_set_mode(gb_graph(),graph_mode_stencil());
  graph_draw_rect(gb_graph(),0,0,200,120);
  graph_draw_rect(gb_graph(),200,120,200,120);
  graph_draw_circle(gb_graph(),200,120,50);
  graph_set_stencil(gb_graph(),false());
  graph_draw_circle(gb_graph(),200,120,40);
  graph_set_stencil(gb_graph(),true());

  graph_set_mode(gb_graph(),graph_mode_normal());
  graph_set_color(gb_graph(),color_random());
  graph_draw_triangle_line(gb_graph(),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240));
}

void debug_quit() {
}