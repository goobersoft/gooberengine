

////////////////
// dict entry //
////////////////

type() {

  char      * key;
  void      * data;

} dictentry_t;

// getters
#define dictentry_key(self)   (self->key)
#define dictentry_data(self)  (self->data)

/////////
// new //
/////////

void dictentry_init( dictentry_t * self, char * s, void * d ) {
  dictentry_key(self)     = s;
  dictentry_data(self)    = d;
  // using a tag can help us identify what type of data the dictentry
  // is holding in other modules.
}

dictentry_t * dictentry( char * s, void * d ) {
  dictentry_t * r      = alloc(dictentry_t);
  dictentry_init(r,s,d);
  return r;
}

// destroys the string object and then itself.
// does not destroy data, be careful!
void free_dictentry( dictentry_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

// will free the previous string object and create a new one.
void dictentry_set_key( dictentry_t * self, char * s ) {
  dictentry_key(self) = s;
}

// note that it will not free the previous data entry if it
// exists. This is up to the user.
void dictentry_set_data( dictentry_t * self, void * d ) {
  dictentry_data(self) = d;
}

////////////////////////////////////////////////////////////////////////////////

///////////////
// dict type //
///////////////

type() {

  // uses a linked list to store the entries.
  list_t * entries;

} dict_t;

#define dict_entries(self) (self->entries)

/////////
// new //
/////////

dict_t * dict() {
  dict_t * r = alloc(dict_t);
  dict_entries(r) = list();
  return r;
}

void free_dict( dict_t * self ) {
  dictentry_t * d;
  while ( list_count(dict_entries(self)) ) {
    d = list_remove_first(dict_entries(self));
    free_dictentry(d);
  }
  free(self);
}

///////////////
// functions //
///////////////

bool_t dict_contains( dict_t * self, char * k ) {
  int u = list_count(dict_entries(self));
  dictentry_t * d;
  while (u > 0) {
    d = list_get_first(dict_entries(self));
    if (dictentry_key(d) == k) {
      return true();
    }
    list_rotate_next(dict_entries(self));
    u--;
  }
  return false();
}

// this will return the old data if the data was overwritten.
// will return null otherwise.
void dict_set( dict_t * self, char * s, void * d ) {
  dictentry_t * de;
  // check first if the dictionary already has an entry
  if (dict_contains(self,s)) {
    // set the new data.
    de = list_get_first(dict_entries(self));
    dictentry_set_data(de,d);
  }
  else {
    // make a new key
    dictentry_t * de = dictentry(s,d);
    list_add_last(dict_entries(self),de);
  }
}

void * dict_get( dict_t * self, char * s ) {
  dictentry_t * d;
  if (dict_contains(self,s)) {
    d = list_get_first(dict_entries(self));
    return dictentry_data(d);
  }
  return null();
}

// will return the retaining data for the key given
// if it exists. null otherwise.
void dict_remove( dict_t * self, char * s ) {
  if (dict_contains(self,s)) {
    list_remove_first(dict_entries(self));
  }
}

void _dict_print( dict_t * self ) {
  log("count: %d",list_count(dict_entries(self)));
  log("keys:");
  dictentry_t * u;
  foreach(dict_entries(self),dt) {
    u = dt;
    log("%s",dictentry_key(u));
  }
}

///////////////////
// helper macros //
///////////////////

#define dset(d,k,v)   dict_set(d,k,cast(v,void*))
#define dget(d,k,c)   cast(dict_get(d,k),c)