

/////////////
// globals //
/////////////

////////////
// events //
////////////

void debug_init() {
}

void debug_load() {
}

void debug_start() {
  graph_t * g = gb_graph();
  graph_set_cls_stencil(g,1);
  graph_set_stencil(g,0);
  graph_cls_ext(g,1,1,1);
  graph_set_mode(g,graph_mode_stencil());
  graph_draw_circle(g,200,120,60);
  graph_set_mode(g,graph_mode_normal());
}

void debug_update_pre() {
}

void debug_update_post() {
}

void debug_draw_pre() {
  graph_t * g = gb_graph();
  graph_set_color(g,make_color_random());
  graph_draw_rect(g,rnd(160,280),rnd(0,240),10,10);
}

void debug_draw_post() {
}

void debug_quit() {
}