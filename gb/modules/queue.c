

#define queue_first(l) list_first(l)
#define queue_count(l) list_count(l)

/////////
// new //
/////////

list_t * queue() {
  return list();
}

///////////
// funcs //
///////////

void * queue_next( list_t * l ) {
  void * u = list_first(l);
  list_remove_first(l);
  return u;
}

void queue_add( list_t * l, void * u ) {
  list_add_last(l,u);
}