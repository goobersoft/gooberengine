
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

bool_t string_equals( string_t * self, char * st ) {
  return streq(string_data(self),st);
}

void string_clear( string_t * self ) {
  loop(i,0,string_length(self)) {
    string_data(self)[i] = ' ';
  }
}

int string_copy_at( string_t * self, int n, char * c ) {
  if (c) {
    int l = strlen(c);
    loop(i,0,l) {
      if (inrange(n+i,0,string_length(self))) {
        string_data(self)[n+i] = c[i];
      }
      else {
        return string_length(self);
      }
    }
    return n+l;
  }
  return 0;
}

int string_copy_number_at( string_t * self, int n, int m ) {
  string_copy_at(self,n,str(m));
}

int string_copy( string_t * self, char * c ) {
  string_copy_at(self,0,c);
}

int string_copy_number( string_t * self, int m ) {
  string_copy_number_at(self,0,m);
}

int string_end( string_t * self, int n ) {
  if ( inrange(n,0,string_length(self)) ) {
    string_data(self)[n] = '\0';
    return n+1;
  }
  return 0;
}

/////////////
// alt new //
/////////////

string_t * string_from( char * a ) {
  string_t * r = string(strlen(a));
  string_copy_at(r,0,a);
  return r;
}