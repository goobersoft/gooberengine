
type() {

  int    length;
  char * data;

} string_t;

#define string_length(self) (self->length)
#define string_data(self)   (self->data)

/////////
// new //
/////////

void string_init( string_t * self, int l ) {
  string_length(self) = l;
  string_data(self)   = allocv(char,string_length(self)+1);
  loop(i,0,string_length(self)) {
    string_data(self)[i] = ' ';
  }
  string_data(self)[string_length(self)] = '\0';
}

string_t * string( int l ) {
  string_t * r     = alloc(string_t);
  string_init(r,l);
  return r;
}

///////////
// funcs //
///////////

void string_clear( string_t * self ) {
  loop(i,0,string_length(self)) {
    string_data(self)[i] = ' ';
  }
}

void string_copy_at( string_t * self, int n, char * c ) {
  if (c) {
    int l = strlen(c);
    loop(i,0,l) {
      if (inrange(n+i,0,string_length(self))) {
        string_data(self)[n+i] = c[i];
      }
      else {
        break;
      }
    }
  }
}

void string_copy( string_t * self, char * c ) {
  string_copy_at(self,0,c);
}

void string_end( string_t * self, int n ) {
  if ( inrange(n,0,string_length(self)) ) {
    string_data(self)[n] = '\0';
  }
}