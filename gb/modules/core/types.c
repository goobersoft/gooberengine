
/////////////
// integer //
/////////////
// in the event that we want to haved an integer
// which can be allocated like other objects, this is the
// type we use.

// range constants
#define integer_max_value()  1000000000
#define integer_min_value() -1000000000

// type
type() {

  tag_t * tag;
  int value;

} integer_t;

// getters
#define integer_tag(self)   (self->tag)
#define integer_value(self) (self->value)

//-- new --//

// not really necessary since it has just one field,
// but kept for conventional purposes.
void integer_init( integer_t * self, int n ) {
  integer_tag(self)     = tag(self,"integer");
  integer_value(self)   = n;
}

integer_t * integer( int n ) {
  integer_t * r = alloc(integer_t);
  integer_init(r,n);
  return r;
}

void free_integer( integer_t * self ) {
  free(self);
}

//-- funcs --//

void integer_set( integer_t * self, int n ) {
  integer_value(self) = n;
}

void integer_add( integer_t * self, int n ) {
  integer_value(self) = clamp(integer_value(self)+n,integer_min_value(),integer_max_value());
}

void integer_mul( integer_t * self, int n ) {
  integer_value(self) = clamp(integer_value(self)*n,integer_min_value(),integer_max_value());
}

void integer_div( integer_t * self, int n ) {
  if (n == 0) {
    if (integer_value(self) >= 0) {
      integer_value(self) = integer_max_value();
    }
    else {
      integer_value(self) = integer_min_value();
    }
  }
  else {
    integer_value(self) /= n;
  }
}

// this function utilizes the additional bit space of the
// frac function, which internally uses a long integer.
// frac(a,b,c) uses the expression ((a*b)/c).
void integer_frac( integer_t * self, int n, int d ) {
  integer_value(self) = frac( integer_value(self), n, d );
}

// using the str() function, convert the value to a string
// representation. Keep in mind that the value itself is stored
// internally and is not your own as subsequent calls to str() will
// overwrite the previous contents. If you'd like to keep the value,
// copy the contents to another string, or use string_from(), which makes
// a string object.
char * integer_to_string( integer_t * self ) {
  return str(integer_value(self));
}

/////////////
// boolean //
/////////////
// 

type() {

  tag_t * tag;
  bool_t value;

} boolean_t;

#define boolean_tag(self)   (self->tag)
#define boolean_value(self) (self->value)

// -- new -- //

boolean_t * boolean( bool_t b ) {
  boolean_t * self      = alloc(boolean_t);
  boolean_tag(self)     = tag(self,"boolean");
  boolean_value(self)   = bool(b);
  return self;
}

// -- funcs -- //

void boolean_set( boolean_t * self, bool_t b ) {
  boolean_value(self) = bool(b);
}

void boolean_toggle( boolean_t * self ) {
  boolean_value(self) = not boolean_value(self);
}

// unlike integer_t, boolean_t will give you a string
// constant of "true" or "false" depending on its value.
// No need to allocate additional memory for each value as
// the 'true' and 'false' values here always have the same
// memory address.
char * boolean_to_string( boolean_t * self ) {
  return (boolean_value(self)==true()) ? "true" : "false";
}

/////////////////////////////
// fixed-fractional number //
/////////////////////////////
// Keep in mind that the representation of a fixed number when in the negative
// is not the same representation in general mathematics.
// you can think of a fixed point number as [whole] + [parts].
// for example: -1 whole + 995 parts is -0.005.
// The internal storage uses -1 as the whole, and 995 as the parts.
// There are 1,000 parts to a whole.
// the to_string function takes this into consideration and converts it into
// the modern mathematical representation
//
// NOTE: When making a new fixed object, you have to use the internal represent-
//  ation. (ex: to get -1.5, use fixed(-2,500). )
//  you can also use fixed(-2,+500) to make it make more sense.

#define fixed_min_whole() -1000000000
#define fixed_max_whole()  1000000000
#define fixed_max_part()         1000 // min is 0

type() {
  tag_t * tag;
  int whole;
  int part;
} fixed_t;

#define fixed_tag(self)   (self->tag)
#define fixed_whole(self) (self->whole)
#define fixed_part(self)  (self->part)

void init_fixed( fixed_t * self, int w, int p ) {
  w = clamp(w,fixed_min_whole(),fixed_max_whole());
  if (w == fixed_max_whole()) {
    // if we reached the maximum, we cannot have a fractional amount
    // over this maximum.
    p = 0;
  }
  else {
    p = wrap(p,0,fixed_max_part());
  }

  fixed_whole(self) = w;
  fixed_part(self)  = p;
}

fixed_t * fixed(int w, int p) {
  fixed_t * self = alloc(fixed_t);
  init_fixed(self,w,p);
  return self;
}

void fixed_set( fixed_t * self, int w, int p ) {
  fixed_whole(self)  = clamp(w,fixed_min_whole(),fixed_max_whole());
  if (w == fixed_max_whole())  p = 0;
  else                        p = wrap(p,0,fixed_max_part());
  fixed_part(self)   = p;
}

void fixed_add( fixed_t * self, int w, int p ) {
  p = fixed_part(self) + p;
  while (p < 0) {
    p += fixed_max_part();
    w  = low(w-1,fixed_min_whole());
  }
  while (p >= fixed_max_part()) {
    p -= fixed_max_part();
    w  = high(w+1,fixed_max_whole());
  }
  fixed_whole(self) = clamp( fixed_whole(self)+w,
    fixed_min_whole(),fixed_max_whole() );
  fixed_part(self) = p;
}

char _fixed_to_string[20];
char * fixed_to_string( fixed_t * self ) {
  loop(i,20) {
    _fixed_to_string[i] = '\0';
  }
  
  int rw = 0;
  int rp = 0;
  int dc = 0;
  if ( (fixed_whole(self) < 0) and (fixed_part(self)>0)) {
    rw = fixed_whole(self) + 1;
    rp = 1000 - fixed_part(self);
    
    if (rw == 0) {
      _fixed_to_string[0] = '-';
      dc = 1;
    }
  }
  else {
    rw = fixed_whole(self);
    rp = fixed_part(self);
  }

  // get the whole number as a string
  char * u = str( rw );
  // get the length of the string
  int l = strlen(u);
  // copy the string to return string
  copy( _fixed_to_string, u, dc, l );
  // dot the string for upcoming fractional part
  _fixed_to_string[l+dc] = '.';
  // get the fractional part, padded to 3 0s
  u = right( str( rp ), 3, '0' );
  // copy that to the return string just past the dot
  copy( _fixed_to_string, u, l+dc+1, 3 );
  // return the string
  return _fixed_to_string;
}

///////////
// array //
///////////

#define array_token_quote() '\"'
#define array_token_space() ' '
#define array_max_tokens()  1024

type() {

  tag_t * tag;
  int     size;
  void ** data;

} array_t;

#define array_tag(self)  (self->tag)
#define array_size(self) (self->size)
#define array_data(self) (self->data)

array_t * array( int n ) {
  array_t * self   = alloc(array_t);
  array_tag(self)  = tag(self,"array");
  array_size(self) = clamp(n,1,1000000);
  array_data(self) = allocv(void*,array_size(self));
  return self;
}

void array_set( array_t * self, int n, void * d ) {
  n = wrap(n,0,array_size(self));
  array_data(self)[n] = d;
}

void * array_get( array_t * self, int n ) {
  n = wrap(n,0,array_size(self));
  return array_data(self)[n];
}

// this function will replace the 'tokenize' function.
// Since tokenize was on the functions module layer, it cannot
// make use of the array_t object. As such, we need to make the tokenize
// process part of this module instead.
// The tokenizer will check for spaces as delimiters.
array_t * array_from_string( char * s ) {
  array_t * self = array(1024);

  // current char
  char c;
  // current word
  char w[1024];
  // word counter
  int  wc = 0;
  // cyrrent array index
  int  ai = 0;
  // quote mode
  int  qm = false();
  // character array
  char * ca;

  for (int i = 0; i < strlen(s); i++) {
    // get current char
    c = s[i];
    //log("%c",c);
    // is it a space?
    if (c == array_token_space()) {
      //log("%s","---SPACE");
      // is quote mode enabled?
      if (qm) {
        //log("%s","---QUOTE MODE IS ON");
        // add the space
        w[wc] = c;
        // increment the word counter
        wc += 1;
        //log("wc: %d",wc);
      }
      // is the word counter larger than 0?
      else if (wc > 0) {
        //log("%s","---ADD WORD");
        // mark the index as a null terminator
        w[wc] = '\0';
        //log("word: %s",w);
        // create a new char array with the size of the word counter
        ca = allocv(char,wc+1);
        // fill the new char array with the word data
        copy(ca,w,0,wc+1);
        // add this to the array
        array_data(self)[ai] = ca;
        // set the word counter to 0
        wc = 0;
        // increment the array index
        ai += 1;
        //log("wc: %d",wc);
        //log("ai: %d",ai);
      }
    }
    // is it a quote?
    else if (c == array_token_quote()) {
      //log("%s","---TOGGLE QUOTE MODE");
      // toggle the quote mode
      qm = !qm;
      //log("qm: %d",qm);
    }
    // otherwise
    else {
      //log("%s","---ADD CHAR");
      // add the character
      w[wc] = c;
      // increment the word counter
      wc += 1;
      //log("wc: %d",wc);
    }
  }

  // is there any trailing characters in the current word?
  if (wc > 0) {
    //log("%s","---LEFTOVERS");
    // mark the index as a null terminator
    w[wc] = '\0';
    //log("word: %s",w);
    // create a new char array with the size of the word counter
    ca = allocv(char,wc+1);
    // fill the new char array with the word data
    copy(ca,w,0,wc+1);
    // add this to the array
    array_data(self)[ai] = ca;
    // set wc to 0
    wc = 0;
    // increment the array index
    ai += 1;
    //log("wc: %d",wc);
    //log("ai: %d",ai);
  }

  // resize the array to be only the data we need.
  array_data(self) = resize(array_data(self),1024,ai);
  // set the size of the array
  array_size(self) = ai;


  loop(i,ai) {
    char * ss = array_data(self)[i];
    log("%p",ss);
  }

  // return the array
  return self;

}

#define aset(a,n,v) array_set(a,n,v)
#define aget(a,n)   array_get(a,n)

/////////////
// array2d //
/////////////

type() {

  tag_t * tag;
  int     size;
  int     w, h;
  void ** data;

} array2d_t;

#define array2d_tag(self)    (self->tag)
#define array2d_data(self)   (self->data)
#define array2d_width(self)  (self->w)
#define array2d_height(self) (self->h)
#define array2d_size(self)   (self->size)

array2d_t * array2d( int w, int h ) {
  array2d_t * self     = alloc(array2d_t);
  array2d_tag(self)    = tag(self,"array2d");
  array2d_data(self)   = allocv(void*,w*h);
  array2d_width(self)  = w;
  array2d_height(self) = h;
  array2d_size(self)   = w*h;
  return self;
}

// keep in mind that the data inside of the 2d array should be handled
// elsewhere because the array does not know what the data even is.
void free_array2d( array2d_t * self ) {
  free( array2d_data(self) );
  free(self);
}

void array2d_set( array2d_t * self, int x, int y, void * d ) {
  int u = wrap( (y*array2d_width(self)) + x, 0, array2d_size(self) );
  array2d_data(self)[ u ] = d;
}

void * array2d_get( array2d_t * self, int x, int y ) {
  int u = wrap( (y*array2d_width(self)) + x, 0, array2d_size(self) );
  return array2d_data(self)[ u ];
}

// helper macros
#define a2set(a,x,y,d) array2d_set(a,x,y,d)
#define a2get(a,x,y)   array2d_get(a,x,y)