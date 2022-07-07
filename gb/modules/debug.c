

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

  graph_enable_depth(g,true());

  graph_set_mode(g,graph_mode_depth());
  
  graph_set_depth(g,graph_min_depth());
  graph_draw_rect(g,0,0,400,240);

  graph_set_depth(g,0);
  graph_draw_circle(g,200,120,120);
  graph_set_mode(g,graph_mode_normal());
}

void debug_update_pre() {
}

void debug_update_post() {
}

void debug_draw_pre() {
  graph_t * g = gb_graph();
  graph_set_color(g,make_color_random());
  graph_draw_rect(g,rnd(0,400),rnd(0,240),10,10);
}

void debug_draw_post() {
}

void debug_quit() {
}