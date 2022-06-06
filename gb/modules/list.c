
//////////////
// listnode //
//////////////

type() {

  void   * prev;
  void   * next;
  void   * data;

} listnode_t;

#define listnode_tag(self)      (self->tag)
#define listnode_prev(self)     (self->prev)
#define listnode_next(self)     (self->next)
#define listnode_data(self)     (self->data)

listnode_t * listnode(void * d) {
  listnode_t * r       = alloc(listnode_t);
  listnode_data(r)     = d;
  return r;
}

void free_listnode( listnode_t * self ) {
  free(self);
}

//////////
// list //
//////////

type() {

  listnode_t * first;
  listnode_t * last;
  int count;

} list_t;

#define list_first(self)    (self->first)
#define list_last(self)     (self->last)
#define list_count(self)    (self->count)

list_t * list() {
  list_t * r       = alloc(list_t);
  list_first(r)    = null();
  list_last(r)     = null();
  list_count(r)    = 0;
  return r;
}

void free_list(list_t * self) {
  listnode_t * d;
  while (list_first(self) != null()) {
    d = list_first(self);
    list_first(self) = listnode_next(d);
    free_listnode(d);
  }
  free(self);
}

///////////
// funcs //
///////////

void list_add_first( list_t * self, void * d ) {
  if (list_count(self)==0) {
    listnode_t * a    = listnode(d);
    list_first(self)  = a;
    list_last(self)   = a;
    list_count(self)  = 1;
  }
  else {
    listnode_t * a    = listnode(d);
    listnode_t * b    = list_first(self);
    listnode_prev(b)  = a;
    listnode_next(a)  = b;
    list_first(self)  = a;
    list_count(self)  += 1;
  }
}

void list_add_last( list_t * self, void * d ) {
  if (list_count(self)==0) {
    listnode_t * a    = listnode(d);
    list_first(self)  = a;
    list_last(self)   = a;
    list_count(self)  = 1;
  }
  else {
    listnode_t * a     = listnode(d);
    listnode_t * b     = list_last(self);
    listnode_next(b)   = a;
    listnode_prev(a)   = b;
    list_last(self)    = a;
    list_count(self)  += 1;
  }
}

void * list_remove_first( list_t * self ) {
  if (list_count(self)==1) {
    listnode_t * a  = list_first(self);
    void * d        = listnode_data(a);
    free_listnode(a);
    list_count(self) = 0;
    return d;
  }
  else if (list_count(self) > 1) {
    listnode_t * a                  = list_first(self);
    void * d                        = listnode_data(a);
    list_first(self)                = listnode_next(a);
    listnode_prev(list_first(self)) = null();
    free_listnode(a);
    list_count(self) -= 1;
    return d;
  }
  return null();
}

void * list_remove_last( list_t * self ) {
  if (list_count(self)==1) {
    listnode_t * a  = list_first(self);
    void * d        = listnode_data(a);
    free_listnode(a);
    list_count(self) = 0;
    return d;
  }
  else if (list_count(self) > 1) {
    listnode_t * a                  = list_last(self);
    void * d                        = listnode_data(a);
    list_last(self)                 = listnode_prev(a);
    listnode_next(list_last(self))  = null();
    free_listnode(a);
    list_count(self) -= 1;
    return d;
  }
  return null();
}


/////////////
// foreach //
/////////////
// usage:
//
//  list: the list to iterate through
//  ptr:  the exposed pointer for manipulation
//
//  foreach(list,ptr) {
//    // do things with the pointer here. example
//    char * p = (char*)ptr;
//    log("%s",p);
//    strcpy(p,"Hello world.");
//  }
//
//  it's not a good idea to use foreach() more than one at a time.


listnode_t * _foreach_listnode;
void       * _foreach_data;

void * _foreach_set_listnode( list_t * d ) {
 _foreach_listnode  = list_first(d);
 _foreach_data      = listnode_data(_foreach_listnode);
 return _foreach_data;
}

void * _foreach_iterate() {
  _foreach_listnode = listnode_next(_foreach_listnode);
  if (_foreach_listnode != null()) {
    _foreach_data     = listnode_data(_foreach_listnode);
    return _foreach_data;
  }
  return null();
}

#define foreach(self,dt) void*dt=_foreach_set_listnode(self);for(;_foreach_listnode!=null();dt=_foreach_iterate())