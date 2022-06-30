

#define tag_max_id_length() 32

type() {

  string_t *  id;
  int         nice;

} tag_t;

#define tag_id(self)    (self->id)
#define tag_nice(self)  (self->nice)

/////////
// new //
/////////

void tag_init( tag_t * self, char * id, int n ) {
  tag_nice(self)  = n;
  tag_id(self)    = string(tag_max_id_length());
  string_copy(tag_id(self),id);
}

tag_t * tag( char * id, int n ) {
  tag_t * r = alloc(tag_t);
  tag_init(r,id,n);
  return r;
}

///////////////
// functions //
///////////////

void tag_set_id( tag_t * self, char * id ) {
  string_copy(tag_id(self),id);
}

void tag_set_nice( tag_t * self, int n ) {
  tag_nice(self) = n;
}