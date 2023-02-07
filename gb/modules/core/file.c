
//////////
// type //
//////////

type() {

  // file path (should be a constant string)
  char * path;
  // source file
  FILE * source;

} file_t;

#define file_path(self)   (self->path)
#define file_source(self) (self->source)

/////////
// new //
/////////

void init_file( file_t * self, char * t ) {
  file_path(self)     = t;
  file_source(self)   = fopen(t,"r+");
}

file_t * file( char * t ) {
  file_t * self = alloc(file_t);
  init_file(self,t);
  return self;
}

string_t * file_read_string( file_t * self ) {

}

void file_write_string( file_t * self, string_t * s ) {

}