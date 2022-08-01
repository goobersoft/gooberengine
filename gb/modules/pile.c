
///////////////
// pilenode  //
///////////////

struct pilenode_t {

  void *               data;
  int                  nice;
  struct pilenode_t * next;

};

type() pilenode_t pilenode_t;
#define pilenode_data(self) (self->data)
#define pilenode_nice(self) (self->nice)
#define pilenode_next(self) (self->next)

/////////
// new //
/////////

pilenode_t * pilenode() {
  pilenode_t * r = alloc(pilenode_t);
  pilenode_nice(r) = 0;
  pilenode_next(r) = null();
  pilenode_data(r) = null();
}

/////////
// DEL //
/////////

// a pile does not delete the data it contains.
// it only deletes the pile object + nodes
// the pile has no idea what the data even is, just that it has
// a pointer to the data. It is up to the user to store the data
// elsewhere (like an array).
void free_pilenode( pilenode_t * s ) {
  if (pilenode_next(s) != null()) {
    free_pilenode( pilenode_next(s) );
  }
  free(s);
}

///////////
// funcs //
///////////

void pilenode_compare_with_next( pilenode_t * self ) {
  if (pilenode_next(self) != null()) {
    if (pilenode_nice(self) > pilenode_nice(pilenode_next(self))) {
      swap( ref(pilenode_nice(self)), ref(pilenode_nice(pilenode_next(self))) );
      void * p                             = pilenode_data(self);
      pilenode_data(self)                 = pilenode_data(pilenode_next(self));
      pilenode_data(pilenode_next(self)) = p;
    }
    pilenode_compare_with_next( pilenode_next(self) );
  }
}


// ------------------------------------------- //

///////////
// pile //
///////////

type() {

  size_t        count;
  pilenode_t * first;

} pile_t;

#define pile_count(self) (self->count)
#define pile_first(self) (self->first)

/////////
// new //
/////////

void pile_init( pile_t * self ) {
  pile_count(self)  = 0;
  pile_first(self)  = null();
}

pile_t * pile() {
  pile_t * r     = alloc(pile_t);
  pile_init(r);
  return r;
}

/////////
// del //
/////////

void free_pile(pile_t * self) {
  if (pile_first(self) != null()) {
    free_pilenode( pile_first(self) );
  }
  free(self);
}

///////////
// funcs //
///////////

// adds data to the pile with a default
// nice value of 0.
void pile_add( pile_t * self, void * d ) {
  pilenode_t * s   = pilenode();
  pilenode_data(s) = d;
  pilenode_next(s) = pile_first(self);
  pile_first(self) = s;
  pile_count(self) += 1;
}

fnptr(
  pile_push,
  pile_add,
  void,
  (pile_t*,void*)
);


void * pile_pop( pile_t * self ) {
  
  // code only executes if the pile is not empty.
  if (pile_count(self) > 0) {
    // grab the first node in the pile
    pilenode_t * s = pile_first(self);
    // grab the data the pilenode is holding
    void * v        = pilenode_data(s);
    // make the first in the pile the pilenode's next
    // (even if this is null, it is fine)
    pile_first(self) = pilenode_next(s);
    // dec the counter by 1.
    pile_count(self) -= 1;
    // return the data
    return v;
  }
  // return null otherwise.
  return null();
}

// add data to the pile with a 'nice' value
// lowest nice first, highest nice last
void pile_addp( pile_t * self, void * d, int n ) {
  pilenode_t * s   = pilenode();
  pilenode_data(s) = d;
  pilenode_nice(s) = n;
  pilenode_next(s) = pile_first(self);
  pile_first(self) = s;
  pile_count(self) += 1;

  pilenode_compare_with_next( s );
}