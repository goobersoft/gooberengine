
///////////////
// pstacknode  //
///////////////

struct pstacknode_t {

  void *               data;
  int                  nice;
  struct pstacknode_t * next;

};

type() pstacknode_t pstacknode_t;
#define pstacknode_data(self) (self->data)
#define pstacknode_nice(self) (self->nice)
#define pstacknode_next(self) (self->next)

/////////
// new //
/////////

pstacknode_t * pstacknode() {
  pstacknode_t * r = alloc(pstacknode_t);
  pstacknode_nice(r) = 0;
  pstacknode_next(r) = null();
  pstacknode_data(r) = null();
}

/////////
// DEL //
/////////

// a pstack does not delete the data it contains.
// it only deletes the pstack object + nodes
// the pstack has no idea what the data even is, just that it has
// a pointer to the data. It is up to the user to store the data
// elsewhere (like an array).
void free_pstacknode( pstacknode_t * s ) {
  if (pstacknode_next(s) != null()) {
    free_pstacknode( pstacknode_next(s) );
  }
  free(s);
}

///////////
// funcs //
///////////

void pstacknode_compare_with_next( pstacknode_t * self ) {
  if (pstacknode_next(self) != null()) {
    if (pstacknode_nice(self) > pstacknode_nice(pstacknode_next(self))) {
      swap( ref(pstacknode_nice(self)), ref(pstacknode_nice(pstacknode_next(self))) );
      void * p                                = pstacknode_data(self);
      pstacknode_data(self)                   = pstacknode_data(pstacknode_next(self));
      pstacknode_data(pstacknode_next(self))  = p;
    }
    pstacknode_compare_with_next( pstacknode_next(self) );
  }
}


// ------------------------------------------- //

////////////
// pstack //
////////////

type() {

  size_t        count;
  pstacknode_t * first;

} pstack_t;

#define pstack_count(self) (self->count)
#define pstack_first(self) (self->first)

/////////
// new //
/////////

void pstack_init( pstack_t * self ) {
  pstack_count(self)  = 0;
  pstack_first(self)  = null();
}

pstack_t * pstack() {
  pstack_t * r     = alloc(pstack_t);
  pstack_init(r);
  return r;
}

/////////
// del //
/////////

void free_pstack(pstack_t * self) {
  if exists( pstack_first(self) ) {
    free_pstacknode( pstack_first(self) );
  }
  free(self);
}

///////////
// funcs //
///////////

// adds data to the pstack with a default
// nice value of 0.
void pstack_push( pstack_t * self, void * d ) {
  pstacknode_t * s   = pstacknode();
  pstacknode_data(s) = d;
  pstacknode_next(s) = pstack_first(self);
  pstack_first(self) = s;
  pstack_count(self) += 1;
}


void * pstack_pop( pstack_t * self ) {
  
  // code only executes if the pstack is not empty.
  if (pstack_count(self) > 0) {
    // grab the first node in the pstack
    pstacknode_t * s = pstack_first(self);
    // grab the data the pstacknode is holding
    void * v        = pstacknode_data(s);
    // make the first in the pstack the pstacknode's next
    // (even if this is null, it is fine)
    pstack_first(self) = pstacknode_next(s);
    // dec the counter by 1.
    pstack_count(self) -= 1;
    // return the data
    return v;
  }
  // return null otherwise.
  return null();
}

// add data to the pstack with a 'nice' value
// lowest nice first, highest nice last
//          v (not a typo)
void pstack_ppush( pstack_t * self, void * d, int n ) {
  pstacknode_t * s   = pstacknode();
  pstacknode_data(s) = d;
  pstacknode_nice(s) = n;
  pstacknode_next(s) = pstack_first(self);
  pstack_first(self) = s;
  pstack_count(self) += 1;

  pstacknode_compare_with_next( s );
}

///////////////////
// helper macros //
///////////////////

#define ppush(s,d,n) pstack_push(s,cast(d,void*),n)
#define ppop(s,t)    cast(pstack_pop(s),t)