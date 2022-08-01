

//////////////
// treenode //
//////////////

#define treenode_type_tree() 1
#define treenode_type_int()  2


type() {

  char     * key;
  void     * data;

} treenode_t;

#define treenode_key(self)       (self->key)
#define treenode_data(self)      (self->data)

// new //
// cs: the class name of the data
// da: void pointer to data
// ld: a flag indicating if the data is locally allocated (for freeing later)
treenode_t * treenode( char * ky, void * da ) {
  treenode_t * r        = alloc(treenode_t);
  treenode_key(r)       = ky;
  treenode_data(r)      = da;
  return r;
}

#define free_treenode(self) free(self)

// funcs //

bool_t treenode_has_key( treenode_t * self, char * ky ) {
  return (treenode_key(self) == ky);
}

void treenode_set_data( treenode_t * self, void * dt ) {
  treenode_data(self) = dt;
}

//////////
// tree //
//////////

type() {

  list_t * nodes;

} tree_t;

#define tree_nodes(self) (self->nodes)
#define tree_count(self) list_count(tree_nodes(self))

// new //
tree_t * tree() {
  tree_t * r = alloc(tree_t);
  tree_nodes(r) = list();
  return r;
}

void free_tree( tree_t * self ) {
  free_list(tree_nodes(self));
  free(self);
}

// funcs //

// keep in mind that this is checking the ADDRESS
// of the string, not if it is equal to a string.
// this function ensures the found key is first.
bool_t tree_has_key( tree_t * self, char * ky ) {
  int l = tree_count(self);
  treenode_t * tn;
  while (l > 0) {
    tn = cast(list_get_first(tree_nodes(self)),treenode_t*);
    if (treenode_has_key(tn,ky)) {
      return true();
    }
    list_rotate_next(tree_nodes(self));
    l--;
  }
  return false();
}

// internal function for adding new keys to the tree
void _tree_add( tree_t * self, char * ky, void * da ) {
  treenode_t * tn = treenode(ky,da);
  list_add_first(tree_nodes(self),tn);
}

void * tree_get( tree_t * self, char * ky ) {
  if (tree_has_key(self,ky)) {
    return treenode_data(cast(list_get_first(tree_nodes(self)),treenode_t*));
  }
  return null();
}

void tree_set( tree_t * self, char * ky, void * da ) {
  treenode_t * tn;
  if (tree_has_key(self,ky)) {
    tn = cast(list_get_first(tree_nodes(self)),treenode_t*);
    return;
  }
  _tree_add(self,ky,da);
}


bool_t tree_remove( tree_t * self, char * ky ) {
  treenode_t   * tn;
  list_t * ls  = tree_nodes(self);
  int l        = list_count(ls);
  if (l > 0) {
    int i = l;
    while (i > 0) {
      tn = cast(list_get_first(tree_nodes(self)),treenode_t*);
      if (treenode_has_key(tn,ky)) {
        list_remove_first(tree_nodes(self));
        free_treenode(tn);
        return true();
      }
      list_rotate_next(ls);
      i--;
    }
  }
  return false();
}