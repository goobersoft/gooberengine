

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
  debug_image     = assets_get_image(gb_assets(),0);
  debug_colormap  = colormap_from_image( debug_image );
}

void debug_update() {
  int rn;
  if (prob(1)) {
    rn = rnd(0,4);
    audio_play(gb_audio(),assets_get_sound(gb_assets(),rn),rn);
  }
}

void debug_draw_pre() {
}

void debug_draw_post() {
  graph_set_mode(gb_graph(),graph_mode_stencil());
  graph_draw_rect(gb_graph(),0,0,200,120);
  graph_draw_rect(gb_graph(),200,120,200,120);
  graph_draw_circle(gb_graph(),200,120,50);
  graph_set_stencil(gb_graph(),false());
  graph_draw_colormap_sub_ex(gb_graph(),debug_colormap,180,95,40,40,100,190,20,20);
  graph_set_stencil(gb_graph(),true());

  loop(i,0,1) {
    graph_set_mode(gb_graph(),graph_mode_normal());
    graph_set_color(gb_graph(),color_random());
    graph_draw_triangle_line(gb_graph(),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240),rnd(0,400),rnd(0,240));
  }
}

void debug_quit() {
}