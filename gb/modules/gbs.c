
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


/////////////////////////
// gbs node type union //
/////////////////////////

// node type union
union() {

  gbsnode_type_assign_t assign;

} gbsnode_type_u;

///////////////////
// gbs node type //
///////////////////

// node type
type() {

  int             node_type;
  gbsnode_type_u  node_data;

} gbsnode_t;


////////////////
// gbs object //
////////////////

type() {

  int         count;
  gbsnode_t * node_first;
  gbsnode_t * node_curr;
  list_t    * bookmarks;

} gbs_t;

/////////
// new //
/////////

gbs_t * gbs() {
  gbs_t * r = alloc(gbs_t);
  return r;
}

///////////////
// functions //
///////////////

