
///////////////
// stacknode //
///////////////

struct stacknode_t {

  void *               data;
  int                  nice;
  struct stacknode_t * next;

};

type() stacknode_t stacknode_t;
#define stacknode_data(self) (self->data)
#define stacknode_nice(self) (self->nice)
#define stacknode_next(self) (self->next)

/////////
// new //
/////////

stacknode_t * stacknode() {
  stacknode_t * r = alloc(stacknode_t);
  stacknode_nice(r) = 0;
  stacknode_next(r) = null();
  stacknode_data(r) = null();
}

/////////
// DEL //
/////////

// a stack does not delete the data it contains.
// it only deletes the stack object + nodes
// the stack has no idea what the data even is, just that it has
// a pointer to the data. It is up to the user to store the data
// elsewhere (like an array).
void free_stacknode( stacknode_t * s ) {
  if (stacknode_next(s) != null()) {
    free_stacknode( stacknode_next(s) );
  }
  free(s);
}

///////////
// funcs //
///////////

void stacknode_compare_with_next( stacknode_t * self ) {
  if (stacknode_next(self) != null()) {
    if (stacknode_nice(self) > stacknode_nice(stacknode_next(self))) {
      swap( ref(stacknode_nice(self)), ref(stacknode_nice(stacknode_next(self))) );
      void * p                             = stacknode_data(self);
      stacknode_data(self)                 = stacknode_data(stacknode_next(self));
      stacknode_data(stacknode_next(self)) = p;
    }
    stacknode_compare_with_next( stacknode_next(self) );
  }
}


// ------------------------------------------- //

///////////
// stack //
///////////

type() {

  size_t        count;
  stacknode_t * first;

} stack_t;

#define stack_count(self) (self->count)
#define stack_first(self) (self->first)

/////////
// new //
/////////

stack_t * stack() {
  stack_t * r     = alloc(stack_t);
  stack_count(r)  = 0;
  stack_first(r)  = null();
  return r;
}

/////////
// del //
/////////

void free_stack(stack_t * self) {
  if (stack_first(self) != null()) {
    free_stacknode( stack_first(self) );
  }
  free(self);
}

///////////
// funcs //
///////////

// adds data to the stack with a default
// nice value of 0.
void stack_add( stack_t * self, void * d ) {
  stacknode_t * s   = stacknode();
  stacknode_data(s) = d;
  stacknode_next(s) = stack_first(self);
  stack_first(self) = s;
  stack_count(self) += 1;
}

fnptr(
  stack_push,
  stack_add,
  void,
  (stack_t*,void*)
);


void * stack_pop( stack_t * self ) {
  
  // code only executes if the stack is not empty.
  if (stack_count(self) > 0) {
    // grab the first node in the stack
    stacknode_t * s = stack_first(self);
    // grab the data the stacknode is holding
    void * v        = stacknode_data(s);
    // make the first in the stack the stacknode's next
    // (even if this is null, it is fine)
    stack_first(self) = stacknode_next(s);
    // dec the counter by 1.
    stack_count(self) -= 1;
    // return the data
    return v;
  }
  // return null otherwise.
  return null();
}

// add data to the stack with a 'nice' value
// lowest nice first, highest nice last
void stack_addp( stack_t * self, void * d, int n ) {
  stacknode_t * s   = stacknode();
  stacknode_data(s) = d;
  stacknode_nice(s) = n;
  stacknode_next(s) = stack_first(self);
  stack_first(self) = s;
  stack_count(self) += 1;

  stacknode_compare_with_next( s );
}