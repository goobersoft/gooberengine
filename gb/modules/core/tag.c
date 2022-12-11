

#define tag_max_id_length() 32

type() {

  // a generic pointer to another object.
  foreign( void * source );
  // a string literal which represents the class of the object
  // containing this tag
  foreign( char * class );
  // a string literal which uniquely identifies the object
  // containing this tag
  foreign( char * id );

} tag_t;

#define tag_source(self)       (self->source)
#define tag_class(self)        (self->class)
#define tag_id(self)           (self->id)


/////////
// new //
/////////

void tag_init( tag_t * self, void * src, char * cls ) {
  tag_source(self)    = src;
  tag_class(self)     = cls;
  tag_id(self)        = "";
}

tag_t * tag( void * src, char * cls ) {
  tag_t * r = alloc(tag_t);
  tag_init(r,src,cls);
  return r;
}

void free_tag( tag_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

void tag_set_source( tag_t * self, void * s ) {
  tag_source(self) = s;
}

void tag_set_class( tag_t * self, char * cs ) {
  tag_class(self) = cs;
}

void tag_set_id( tag_t * self, char * id ) {
  tag_id(self) = id;
}

