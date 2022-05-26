

/////////////
// globals //
/////////////

colormap_t * debug_colormap;
image_t *    debug_image;
list_t *     debug_list;
int          debug_data;

////////////
// events //
////////////

void debug_init() {

  list_t * l = list();
  loop(i,0,10) {
    list_add_last(l,int2ptr(i*10));
  }

  // l: list
  // d: void * pointer name
  foreach(l,d) {
    log("%d",ptr2int(d));
  }
}

void debug_load() {
  //debug_colormap = colormap(100,100);
  debug_image = assets_get_image(gb_assets(),0);
  debug_colormap = colormap_from_image( debug_image );
}

void debug_update() {
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
  static int cls = 0;
  static int _init = 0;
  static int xs[4];
  static int ys[4];
  static int xv[4];
  static int yv[4];
  static color_t cc;
  if (_init == 0) {
    cls = color(3,3,3);
    loop(i,0,4) {
      xs[i] = 1000*rnd(0,400);
      ys[i] = 1000*rnd(0,240);
      xv[i] = rnd(-2000,2000);
      yv[i] = rnd(-2000,2000);
    }
    cc = rnd(1,64);
    _init = 1;
  }

  loop(i,0,4) {

    xs[i]+=xv[i];
    if (!inrange(xs[i],0,400000)) {
      xv[i] = -xv[i];
      xs[i] = clamp(xs[i],0,3999999);
    }
    ys[i]+=yv[i];
    if (!inrange(ys[i],0,240000)) {
      yv[i] = -yv[i];
      ys[i] = clamp(ys[i],0,239999);
    }
  }

  
  graph_set_clip( gb_graph(), 100, 60, 200, 120 );

  graph_set_mode( gb_graph(), graph_mode_sub() );
  graph_set_color( gb_graph(), color(1,1,1) );
  graph_draw_rect_spray( gb_graph(), 0, 0, 400, 240, 5 );
  

  graph_set_mode( gb_graph(), graph_mode_normal() );
  graph_set_color( gb_graph(), cc );
  loop(i,0,4) {
    graph_draw_line( gb_graph(), xs[i]/1000, ys[i]/1000, xs[(i+1)%4]/1000, ys[(i+1)%4]/1000 );
  }

  

  if (chance(1,1000)) cc  = rnd(1,64);
  if (chance(1,1000)) cls = color(rnd(0,3),rnd(0,3),rnd(0,3)); 
  //log("%d",timing_fps(gb_timing()));
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