

////////////////
// dict entry //
////////////////

type() {

  string_t  * key;
  void      * data;

} dictentry_t;

#define dictentry_key(self)   (self->key)
#define dictentry_data(self)  (self->data)

dictentry_t * dictentry( char * s, void * d ) {
  dictentry_t * r      = alloc(dictentry_t);
  dictentry_key(r)     = string_from(s);
  dictentry_data(r)    = d;
  return r;
}

// destroys the string object and then itself.
// does not destroy data, be careful!
void free_dictentry( dictentry_t * self ) {
  free_string(dictentry_key(self));
  free(self);
}

// will free the previous string object and create a new one.
void dictentry_set_key( dictentry_t * self, char * s ) {
  free_string(dictentry_key(self));
  dictentry_key(self) = string_from(s);
}

// note that it will not free the previous data entry if it
// exists. This is up to the user.
void dictentry_set_data( dictentry_t * self, void * d ) {
  dictentry_data(self) = d;
}

bool_t dictentry_compare_key( dictentry_t * self, char * s ) {
  return streq( string_data(dictentry_key(self)), s );
}


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

///////////////
// functions //
///////////////

// this will return true if a dictionary entry was
// over-written. false otherwise.
bool_t dict_set( dict_t * self, char * s, void * d ) {

  // check if the key already exists.
  if (list_count(dict_entries(self)) > 0) {
    dictentry_t * u;
    foreach(dict_entries(self),dt) {
      u = cast(dt,dictentry_t*);
      if (dictentry_compare_key(u,s)) {
        dictentry_set_data(u,d);
        return true();
      }
    }
  }

  // make a new key
  dictentry_t * de = dictentry(s,d);
  list_add_last(dict_entries(self),de);
  return false();
}

void * dict_get( dict_t * self, char * s ) {
  if (list_count(dict_entries(self)) > 0) {
    dictentry_t * u;
    foreach(dict_entries(self),dt) {
      u = cast(dt,dictentry_t*);
      if (dictentry_compare_key(u,s)) {
        return dictentry_data(u);
      }
    }
  }
  return null();
}

bool_t dict_remove( dict_t * self, char * s ) {

  if (list_count(dict_entries(self)) > 0) {
    int l = list_count(dict_entries(self));

    dictentry_t * u;
    while (l > 0) {
      u = cast(list_get_first(dict_entries(self)),dictentry_t*);

      if ( dictentry_compare_key(u,s) ) {
        list_remove_first(dict_entries(self));
        free_dictentry(u);
        return true();
      }

      // rotate the entries so that the next node is at the head.
      list_rotate_next(dict_entries(self));

      l--;
    }
  }
  return false();
}

void _dict_print( dict_t * self ) {
  log("count: %d",list_count(dict_entries(self)));
  log("keys:");
  dictentry_t * u;
  foreach(dict_entries(self),dt) {
    u = cast(dt,dictentry_t*);
    log("%s",string_data(dictentry_key(u)));
  }
}