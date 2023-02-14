
// a gbml object is the same as a dictionary
typedef dict_t gbml_t;

#define gbml_token_quote()      '"'
#define gbml_token_scopein()    '{'
#define gbml_token_scopeout()   '}'
#define gbml_token_assign()     '='
#define gbml_token_statement()  ';'
#define gbml_token_number()     '#'
#define gbml_token_string()     '$'

/////////
// new //
/////////

dict_t * gbml( char * s ) {
  dict_t * self = dict();

  // if a path is given, get the file.
  if (s) {
    log("a file was given - yadda.");
  }

  return self;
}

///////////////
// functions //
///////////////

// convert a GBML object into GBML notation (string)
string_t * gbml_to_string( gbml_t * in ) {
} 

// create a gbml object from string input
dict_t * gbml_from_string( char * in ) {
}

// get the data of a key in the gbml dictionary
void * gbml_read( gbml_t * self, char * p ) {
}

// write data into the key of a gbml object
void gbml_write( gbml_t * self, char * p, void * v ) {
}

////////////
// macros //
////////////

#define gbml_set        dict_set
#define gbml_get        dict_get
#define gbml_remove     dict_remove
#define gbml_contains   dict_contains

//////////
// load //
//////////

gbml_t * gbml_load() {
}

gbml_t * gbml_save() {
}