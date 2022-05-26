
type() {

  char * id;
  int    nice;

} tag_t;

#define tag_id(self)    (self->id)
#define tag_nice(self)  (self->nice)

/////////
// new //
/////////

tag_t * tag( char * id, int n ) {
  tag_t * r = alloc(tag_t);
  tag_nice(r) = n;
  if (id != null()) {
    strcpy(tag_id(r),id);
  }
  return r;
}

void free_tag( tag_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

void tag_set_id( tag_t * self, char * id ) {
  // clear all chars in id
  loop(i,0,32) {
    tag_id(self)[i] = '\0';
  }
  // set new id
  strcpy(tag_id(self),id);
}

void tag_set_nice( tag_t * self, int n ) {
  tag_nice(self) = n;
}