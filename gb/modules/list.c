
//////////////
// listnode //
//////////////

type() {

  tag_t  * tag;
  void   * prev;
  void   * next;
  void   * data;

} listnode_t;

#define listnode_tag(self)  (self->tag)
#define listnode_prev(self) (self->prev)
#define listnode_next(self) (self->next)
#define listnode_data(self) (self->data)

listnode_t * listnode(void * d) {
  listnode_t * r = alloc(listnode);
  listnode_tag(r) = tag(null());
  listnode_data(r) = d;
  return r;
}

void free_listnode( listnode_t * d ) {
  free(listnode_data(d));
  free(d);
}

//////////
// list //
//////////

type() {

  listnode_t * first;
  listnode_t * last;
  int count;

} list_t;

#define list_first(self)  (self->first)
#define list_last(self)   (self->last)
#define list_count(self)  (self->count)

list_t * list() {
  list_t * r = alloc(list_t);
  list_first(r)  = null();
  list_last(r)   = null();
  list_count(r)  = 0;
  return r;
}

///////////
// funcs //
///////////

void list_add_first( list_t * l, void * d ) {
  listnode_t * u1 = listnode(d);
  if (list_count(l) == 0) {
    list_first(l)  = u1;
    list_last(l)   = u1;
    list_count(l)  = 1;
  }
  else {
    listnode_t * u2   = list_first(l);
    listnode_prev(u2) = u1;
    listnode_next(u1) = u2;
    list_first(l)     = u1;
    list_count(l)     += 1;
  }
}

void list_add_last( list_t * l, void * d ) {
  listnode_t * u1 = listnode(d);
  if (list_count(l) == 0) {
    list_first(l)  = u1;
    list_last(l)   = u1;
    list_count(l)  = 1;
  }
  else {
    listnode_t * u2    = list_last(l);
    listnode_next(u2)  = u1;
    listnode_prev(u1)  = u2;
    list_last(l)       = u1;
    list_count(l)     += 1;
  }
}

void list_remove_first( list_t * l ) {
  listnode_t * d;
  if (list_count(l) > 0) {
    if (list_count(l) == 1) {
      list_count(l) = 0;
      d = list_first(l);
      list_first(l) = null();
      list_last(l) = null();
      free_listnode(d);
    }
    else {
      d = list_first(l);
      list_first(l) = listnode_next(d);
      listnode_prev(list_first(l)) = null();
      list_count(l) -= 1;
      free_listnode(d);
    }
  }
}

void list_remove_last( list_t * l ) {
  listnode_t * d;
  if (list_count(l) > 0) {
    if (list_count(l) == 1) {
      list_count(l) = 0;
      d = list_first(l);
      list_first(l) = null();
      list_last(l) = null();
      free_listnode(d);
    }
    else {
      d = list_last(l);
      list_last(l) = listnode_prev(d);
      listnode_next(list_last(l)) = null();
      list_count(l) -= 1;
      free_listnode(d);
    }
  }
}