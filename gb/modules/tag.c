

#define tag_max_id_length() 32

type() {

  // a string literal which represents the class of the object
  // containing this tag
  const char * class;
  // a string literal which uniquely identifies the object
  // containing this tag
  const char * id;

} tag_t;

#define tag_class(self)     (self->class)
#define tag_id(self)        (self->id)

/////////
// new //
/////////

void tag_init( tag_t * self, char * cs, char * id ) {
  tag_class(self) = cs;
  tag_id(self)    = id;
}

tag_t * tag( char * cs, char * id ) {
  tag_t * r = alloc(tag_t);
  tag_init(r,cs,id);
  return r;
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