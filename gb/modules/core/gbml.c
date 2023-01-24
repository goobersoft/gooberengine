
// a gbml object is the same as a dictionary
typedef dict_t gbml_t;

#define gbml_token_quote()      '"'
#define gbml_token_scopein()    '{'
#define gbml_token_scopeout()   '}'
#define gbml_token_assign()     '='
#define gbml_token_statement()  ';'
#define gbml_token_number()     '#'
#define gbml_token_string()     '$'

///////////////
// functions //
///////////////

void gbml_to_string( gbml_t * in ) {
} 

void gbml_from_string( char * in ) {
}

void * gbml_read( gbml_t * self, char * p ) {

}

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