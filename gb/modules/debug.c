

/////////////
// globals //
/////////////

debugpanel_t  * debug_panel;
dict_t        * debug_dict;

////////////
// events //
////////////

void debug_init() {
  debug_panel = debugpanel();
  
  debug_dict = dict();
  dict_set(debug_dict,"one","1");
  dict_set(debug_dict,"two","2");
  dict_set(debug_dict,"three","3");
  dict_set(debug_dict,"four","4");
  dict_set(debug_dict,"five","5");
  
  dict_remove(debug_dict,"three");
  dict_set(debug_dict,"six","6");

  log("%s",cast(dict_get(debug_dict,"two"),char*));

  _dict_print(debug_dict);
  
}

void debug_load() {
  
}

void debug_start() {

}

void debug_update_pre() {
}

void debug_update_post() {
}

void debug_draw_pre() {
  graph_set_color(gb_graph(),make_color(0,0,0));
  graph_draw_rect(gb_graph(),0,0,400,240);
}

void debug_draw_post() {
  debugpanel_draw(debug_panel);
}

void debug_quit() {
}