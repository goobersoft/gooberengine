
///////////
// notes //
///////////
// use the pointer address to determine the priority
// of the nodes. This would be ptr_t

//////////
// type //
//////////

type() {

  uintptr_t   priority;
  void      * data;
  void      * left;
  void      * right;

} bintree_t;

#define bintree_priority(self)  (self->priority)
#define bintree_data(self)      (self->data)
#define bintree_left(self)      (self->left)
#define bintree_right(self)     (self->right)

/////////
// new //
/////////

void free_bintree( bintree_t * self ) {
  if (bintree_left(self))   free_bintree(bintree_left(self));
  if (bintree_right(self))  free_bintree(bintree_right(self));
}

void init_bintree( bintree_t * self, void * d ) {
  bintree_data(self)      = d;
  bintree_priority(self)  = ptrtoint(d);
}

bintree_t * bintree( void * d ) {
  bintree_t * self = alloc(bintree_t);
  init_bintree    (self,d);
  return self;
}

///////////////
// functions //
///////////////

void * bintree_set_data( bintree_t * self, void * dt ) {
  void * r = bintree_data(self);
  bintree_data(self) = dt;
  return r;
}

void * bintree_set_left( bintree_t * self, bintree_t * dt ) {
  bintree_t * r = bintree_left(self);
  bintree_left(self) = dt;
  return r;
}

void * bintree_set_right( bintree_t * self, bintree_t * dt ) {
  bintree_t * r = bintree_right(self);
  bintree_right(self) = dt;
  return r;
}

void bintree_add( bintree_t * self, void * dt ) {
}

void bintree_remove( bintree_t * self, void * dt ) {
}

void bintree_find( bintree_t * self, void * dt ) {
}