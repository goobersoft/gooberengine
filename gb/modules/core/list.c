
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
  int          count;

} list_t;

// getters
#define list_first(self)    (self->first)
#define list_last(self)     (self->last)
#define list_count(self)    (self->count)

/////////
// new //
/////////

void list_init( list_t * self ) {
  list_first(self)    = null();
  list_last(self)     = null();
  list_count(self)    = 0;
}

list_t * list() {
  list_t * r = alloc(list_t);
  list_init(r);
  return r;
}

void free_list(list_t * self) {
  listnode_t * d;
  while exists(list_first(self)) {
    d = list_first(self);
    list_first(self) = listnode_next(d);
    free_listnode(d);
  }
  free(self);
}

///////////
// funcs //
///////////

// internal functions for adding and removing listnodes

void _list_add_first( list_t * self, listnode_t * l ) {
  if (list_count(self)==0) {
    list_first(self)  = l;
    list_last(self)   = l;
    list_count(self)  = 1;
  }
  else {
    listnode_t * b    = list_first(self);
    listnode_prev(b)  = l;
    listnode_next(l)  = b;
    list_first(self)  = l;
    list_count(self)  += 1;
  }
}

void _list_add_last( list_t * self, listnode_t * l ) {
  if (list_count(self)==0) {
    list_first(self)  = l;
    list_last(self)   = l;
    list_count(self)  = 1;
  }
  else {
    listnode_t * b     = list_last(self);
    listnode_next(b)   = l;
    listnode_prev(l)   = b;
    list_last(self)    = l;
    list_count(self)  += 1;
  }
}

listnode_t * _list_remove_first( list_t * self ) {
  if (list_count(self)==1) {
    listnode_t * a    = list_first(self);
    list_first(self)  = null();
    list_last(self)   = null();
    listnode_next(a)  = null();
    listnode_prev(a)  = null();
    list_count(self)  = 0;
    return a;
  }
  else if (list_count(self) > 1) {
    listnode_t * a                  = list_first(self);
    list_first(self)                = listnode_next(a);
    listnode_prev(list_first(self)) = null();
    listnode_next(a) = null();
    listnode_prev(a) = null();
    list_count(self) -= 1;
    return a;
  }
  return null();
}

listnode_t * _list_remove_last( list_t * self ) {
  if (list_count(self)==1) {
    listnode_t * a  = list_last(self);
    list_first(self)  = null();
    list_last(self)   = null();
    listnode_next(a)  = null();
    listnode_prev(a)  = null();
    list_count(self)  = 0;
    return a;
  }
  else if (list_count(self) > 1) {
    listnode_t * a                  = list_last(self);
    list_last(self)                 = listnode_prev(a);
    listnode_next(list_last(self))  = null();
    listnode_next(a) = null();
    listnode_prev(a) = null();
    list_count(self) -= 1;
    return a;
  }
  return null();
}

// public functions

void list_add_first( list_t * self, void * d ) {
  _list_add_first(self,listnode(d));
}

void list_add_last( list_t * self, void * d ) {
  _list_add_last(self,listnode(d));
}

void * list_remove_first( list_t * self ) {
  listnode_t * l = _list_remove_first(self);
  if (l) {
    void * d = listnode_data(l);
    free_listnode(l);
    return d;
  }
  return null();
}

void * list_remove_last( list_t * self ) {
  listnode_t * l = _list_remove_last(self);
  if (l) {
    void * d = listnode_data(l);
    free_listnode(l);
    return d;
  }
  return null();
}

void * list_get_first( list_t * self ) {
  return listnode_data(list_first(self));
}

void * list_get_last( list_t * self ) {
  return listnode_data(list_last(self));
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
  if (list_count(d)>0) {
  _foreach_listnode  = list_first(d);
  _foreach_data      = listnode_data(_foreach_listnode);
  return _foreach_data;
  }
  return null();
}

void * _foreach_iterate() {
  _foreach_listnode = listnode_next(_foreach_listnode);
  if exists(_foreach_listnode) {
    _foreach_data     = listnode_data(_foreach_listnode);
    return _foreach_data;
  }
  return null();
}

#define foreach(self,dt) for(void*dt=_foreach_set_listnode(self);_foreach_listnode!=null();dt=_foreach_iterate())

/////////////////
// other funcs //
/////////////////

bool_t list_contains( list_t * self, void * dt ) {
  foreach(self,dl) {
    if (dl == dt) {
      return true();
    }
  }
  false();
}


// rotating a list allows you to check the contents of the list at either
// the head or the tail.
void list_rotate_next(list_t * self) {
  if (list_count(self) > 1) {
    listnode_t * t1 = _list_remove_first(self);
    _list_add_last(self,t1);
  }
}

void list_rotate_prev(list_t * self) {
  if (list_count(self) > 1) {
    listnode_t * t1 = _list_remove_last(self);
    _list_add_first(self,t1);
  }
}

// returns true and places the item in the front of the list if it
// is inside of the list. Returns false otherwise.
bool_t list_find(list_t * self, void * v) {
  // check to see if the item exists in the list.
  // if it does, continue to rotate the list until it is the first
  // item in the list.
  if (list_contains(self,v)) {
    while(list_get_first(self) != v) {
      list_rotate_next(self);
    }
    return true();
  }
  return false();
}