
/////////////////////////
// gooberscript system //
/////////////////////////
/*
  Gooberscript is not interpreted.
  Instead it uses friendly function names:

  SetSpritePosition 2 4
  Set x 10
  Set y 20
  Add x y
  Print x          // 30
  
  The semicolon at the end is omitted as well
*/


////////////////////
// gbs node types //
////////////////////

type() {

} gbsnode_type_assign_t;


///////////////////
// gbs node type //
///////////////////

// node type
type() {

  int node_type;
  local( void * node_data );

} gbsnode_t;


////////////////
// gbs object //
////////////////

type() {

  int count;
  local( gbsnode_t * node_first );
  local( gbsnode_t * node_curr );
  local( list_t    * bookmarks );

} gbs_t;

/////////
// new //
/////////

void gbs_init( gbs_t * self ) {

}

gbs_t * gbs() {
  gbs_t * r = alloc(gbs_t);
  gbs_init(r);
  return r;
}

///////////////
// functions //
///////////////

