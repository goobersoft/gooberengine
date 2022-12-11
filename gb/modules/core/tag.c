

#define tag_max_id_length() 32

type() {

  // a string literal which represents the class of the object
  // containing this tag
  char * class;
  // a string literal which uniquely identifies the object
  // containing this tag
  char * id;
  // a generic pointer to another object.
  void * source;

} tag_t;

#define tag_class(self)        (self->class)
#define tag_id(self)           (self->id)
#define tag_source(self)       (self->source)

/////////
// new //
/////////

void tag_init( tag_t * self, char * cs, char * id, void * s ) {
  tag_class(self)     = cs;
  tag_id(self)        = id;
  tag_source(self)    = s;
}

tag_t * tag( char * cs, char * id, void * s ) {
  tag_t * r = alloc(tag_t);
  tag_init(r,cs,id,s);
  return r;
}

void free_tag( tag_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

void tag_set_class( tag_t * self, char * cs ) {
  tag_class(self) = cs;
}

void tag_set_id( tag_t * self, char * id ) {
  tag_id(self) = id;
}

void tag_set_source( tag_t * self, void * s ) {
  tag_source(self) = s;
}