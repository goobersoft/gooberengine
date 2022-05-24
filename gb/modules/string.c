

type() {

  int    length;
  char * data;

} string_t;

#define string_length(self) (self->length)
#define string_data(self)   (self->data)

/////////
// new //
/////////

string_t * string(char * c) {
  string_t * r     = alloc(string_t);
  string_length(r) = strlen(c);
  string_data(r)   = allocv(char,string_length(r)+1);
  
  loop(i,0,string_length(r)) {
    string_data(r)[i] = c[i];
  }

  return r;
}

string_t * string__length( int n ) {
  string_t * r = alloc(string_t);
  string_length(r) = n;
  string_data(r)   = allocv(char,n);
  
  loop(i,0,n) {
    string_data(r)[i] = 0;
  }

  return r;
}


string_t * string__concat( string_t * s1, string_t * s2 ) {
  string_t * r = string__length( string_length(s1) + string_length(s2) + 1);
  
  copy(string_data(s1),string_data(s2),i,0,string_length(s1));
  copy(string_data(s1),string_data(s2),i,string_length(s1),string_length(s1)+string_length(s2));

  return r;
}

//////////
// free //
//////////

void free_string( string_t * self ) {
  free(string_data(self));
  free(self);
}

///////////
// funcs //
///////////

void string_set( string_t * self, char * c ) {
  free(string_data(self));
  size_t l     = strlen(c)+1;
  char * r     = allocv(char,l);
  strncpy      (r,c,l);
}