
///////////////
// ordernode  //
///////////////

type() {

  void  * data;
  int     nice;
  void  * next;

} ordernode_t;

#define ordernode_data(self) (self->data)
#define ordernode_nice(self) (self->nice)
#define ordernode_next(self) (self->next)

/////////
// new //
/////////

ordernode_t * ordernode() {
  ordernode_t * r = alloc(ordernode_t);
  ordernode_nice(r) = 0;
  ordernode_next(r) = null();
  ordernode_data(r) = null();
}

/////////
// DEL //
/////////

// a order does not delete the data it contains.
// it only deletes the order object + nodes
// the order has no idea what the data even is, just that it has
// a pointer to the data. It is up to the user to store the data
// elsewhere (like an array).
void free_ordernode( ordernode_t * s ) {
  if (ordernode_next(s) != null()) {
    free_ordernode( ordernode_next(s) );
  }
  free(s);
}

///////////
// funcs //
///////////

void ordernode_compare_with_next( ordernode_t * self ) {
  ordernode_t * nx;
  if (ordernode_next(self)) {
    nx = ordernode_next(self);
    if (ordernode_nice(self) > ordernode_nice(nx)) {
      swap                   (ref(ordernode_nice(self)),ref(ordernode_nice(nx)));
      void * p               = ordernode_data(self);
      ordernode_data(self)   = ordernode_data(nx);
      ordernode_data(nx)     = p;
    }
    ordernode_compare_with_next( nx );
  }
}


// ------------------------------------------- //

////////////
// order //
////////////

type() {

  size_t         count;
  ordernode_t * first;

} order_t;

#define order_count(self) (self->count)
#define order_first(self) (self->first)

/////////
// new //
/////////

void init_order( order_t * self ) {
  order_count(self)  = 0;
  order_first(self)  = null();
}

order_t * order() {
  order_t * r     = alloc(order_t);
  init_order(r);
  return r;
}

/////////
// del //
/////////

void free_order(order_t * self) {
  if exists( order_first(self) ) {
    free_ordernode( order_first(self) );
  }
  free(self);
}

///////////
// funcs //
///////////

// adds data to the order with a default
// nice value of 0.
void order_push( order_t * self, void * d ) {
  ordernode_t * s   = ordernode();
  ordernode_data(s) = d;
  ordernode_next(s) = order_first(self);
  order_first(self) = s;
  order_count(self) += 1;
}


void * order_pop( order_t * self ) {
  
  // code only executes if the order is not empty.
  if (order_count(self) > 0) {
    // grab the first node in the order
    ordernode_t * s = order_first(self);
    // grab the data the ordernode is holding
    void * v        = ordernode_data(s);
    // make the first in the order the ordernode's next
    // (even if this is null, it is fine)
    order_first(self) = ordernode_next(s);
    // dec the counter by 1.
    order_count(self) -= 1;
    // return the data
    return v;
  }
  // return null otherwise.
  return null();
}

// add data to the order with a 'nice' value
// lowest nice first, highest nice last
//          v (not a typo)
void order_push_p( order_t * self, void * d, int n ) {
  ordernode_t * s   = ordernode();
  ordernode_data(s) = d;
  ordernode_nice(s) = n;
  ordernode_next(s) = order_first(self);
  order_first(self) = s;
  order_count(self) += 1;

  ordernode_compare_with_next( s );
}