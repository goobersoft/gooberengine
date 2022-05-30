

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
  static int sx = 0;
  static int tm = 0;
  static int ssx = 0;
  static int ssy = 0;
  static int wrp = 0;

  //wrp = (wrp + 10) % 240;

  
  //graph_set_clip(gb_graph(), 100, 60, 200, 120);
  
  wrp = (wrp + 1) % 6;
  if (wrp==0) {
    graph_set_mode( gb_graph(), graph_mode_sub() );
    graph_set_color( gb_graph(), color(1,1,1) );
    graph_draw_rect_dither_even( gb_graph(), 0, 0, 400, 240 );
  }
  else if (wrp==3) {
    graph_set_mode( gb_graph(), graph_mode_sub() );
    graph_set_color( gb_graph(), color(1,1,1) );
    graph_draw_rect_dither_odd( gb_graph(), 0, 0, 400, 240 );
  }
  //graph_draw_rect_spray( gb_graph(), 0, 0, 400, 240, 3 );
  //graph_draw_rect( gb_graph(), 0, wrp, 400, 10 );
  //graph_draw_rect( gb_graph(), 0, rnd(0,240), 400, 1 );
  //graph_draw_circle_spray( gb_graph(), rnd(0,400), rnd(0,240), rnd(30,50), 75 );
  /*
  graph_set_mode( gb_graph(), graph_mode_add() );
  loop(i,0,1) {
    tm = wrap(tm+386,0,1000);
    sx = 60 + frac(60,sine(tm),1000);
    ssx = tile10(rnd(0,40));
    ssy = tile10(rnd(0,24));
    graph_draw_colormap_sub_ex( gb_graph(), debug_colormap, rnd(0,400), rnd(0,240), sx, sx, ssx, ssy, 10, 10 );
  }
  */
  graph_set_mode( gb_graph(), graph_mode_add() );
  graph_set_color(gb_graph(),color_random());
  graph_draw_triangle_line(gb_graph(),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240));
}



/*
void debug_draw_pre() {
  graph_set_mode( gb_graph(), graph_mode_add() );
  graph_draw_colormap_sub( gb_graph(), debug_colormap, rnd(0,400), rnd(0,240), rnd(0,390), rnd(0,230), 10, 10 );
}
*/
void debug_draw_post() {

}

void debug_quit() {
}