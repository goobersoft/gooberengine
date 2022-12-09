
type() {

  int    length;
  char * data;

} string_t;

// the real length of the string, which does not account
// for null terminators. The actual size of the char array is
// length+1.
#define string_length(self) (self->length)
// macro to get the inner character data from the string.
#define string_data(self)   (self->data)
// macro to determine what C would see in terms of a string length
// using strlen(). Note that this will check for a null terminator.
// if the first character is a null terminator, then the virtual
// length of the string if 0.
#define string_virtual_length(self) strlen(string_data(self))

/////////
// new //
/////////

void string_init( string_t * self, int l ) {
  string_length(self) = l;
  // use one extra byte for null terminator
  string_data(self)   = allocv(char,string_length(self)+1);
  loop(i,string_length(self)) {
    string_data(self)[i] = ' ';
  }
  // make sure that the string has a null terminator at the end.
  string_data(self)[string_length(self)] = '\0';
}

string_t * string( int l ) {
  string_t * r     = alloc(string_t);
  string_init(r,l);
  return r;
}

void free_string( string_t * self ) {
  free(string_data(self));
  free(self);
}

///////////
// funcs //
///////////

// this will return 0 if it is out of range.
// it will otherwise pluck a character from the array.
char string_get( string_t * self, int p ) {
  if (inrange(p,0,string_length(self))) {
    return string_data(self)[p];
  }
  return 0;
}


bool_t string_equals( string_t * self, char * st ) {
  return streq(string_data(self),st);
}

int string_clear( string_t * self ) {
  loop(i,string_length(self)) {
    string_data(self)[i] = '\0';
  }
  return 0;
}

int string_clear_with( string_t * self, char c ) {
  loop(i,string_length(self)) {
    string_data(self)[i] = c;
  }
  return 0;
}

int string_put( string_t * self, int n, char c ) {
  if (inrange(n,0,string_length(self))) {
    string_data(self)[n] = c;
    return n + 1;
  }
  else {
    return n;
  }
}

int string_copy_at( string_t * self, int n, char * c ) {
  if (c) {
    int l = strlen(c);
    loop(i,l) {
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