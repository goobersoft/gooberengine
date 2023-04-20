
//////////
// type //
//////////

type() {
  
  // a boolean for solidity.
  // two entities may collide only if both of them are solid.
  bool_t solid;

  // rect object to hold position and size data
  local( rect_t * rect );

} entity_t;

#define entity_rect(self)    (self->rect)
#define entity_solid(self)   (self->solid)

#define entity_pos(self)     rect_pos(entity_rect(self))
#define entity_pos_x(self)   rect_x(entity_rect(self))
#define entity_pos_y(self)   rect_y(entity_rect(self))
#define entity_size(self)    rect_size(entity_rect(self))
#define entity_size_x(self)  rect_w(entity_rect(self))
#define entity_size_y(self)  rect_h(entity_rect(self))

//#define entity_set_parent(self,p)  set(self,parent,p)
#define entity_set_pos(self,x,y)   rect_set_pos(entity_rect(self),x,y)
#define entity_add_pos(self,x,y)   rect_add_pos(entity_rect(self),x,y)
#define entity_set_size(self,w,h)  rect_set_size(entity_rect(self),w,h)
#define entity_set_solid(self,b)   set(self,solid,bool(b))

/////////
// new //
/////////

void entity_init( entity_t * self ) {
  entity_rect(self)        = rect(0,0,10,10);
  entity_solid(self)       = true();
}

entity_t * entity(void) {
  entity_t * self = alloc(entity_t);
  entity_init(self);
  return self;
}

void free_entity( entity_t * self ) {
  free_rect(entity_rect(self));
  free(self);
}

///////////
// funcs //
///////////
 
 // check if two entities are overlapping
bool_t entity_collide( entity_t * self, entity_t * other ) {
  if (entity_solid(self) and entity_solid(other)) {
    return rect_collide( entity_rect(self), entity_rect(other) );
  }
}

// it is assumed that all of the objects inside of the list are
// asso entity_t objects.
entity_t * entity_collide_list( entity_t * self, list_t * ls ) {
  bool_t b;
  foreach(ls,dt) {
    if (entity_collide( self, dt )) {
      return dt;
    }
  }
  return null();
}

///////////////////////
// gliding functions //
///////////////////////

// this function will be useful for getting the entity that was collided
// with in list-check functions. The idea is to set this function's internal
// entity pointer with an entity in the function argument. This function will
// always return the previous value before setting, so typically you'd do
// something like this:
// 
// entity_check_result( e1 );                        <-- store collision
// entity_t * e = entity_check_result( null() );     <-- retrieve collision
//
// If no collision occurred during the check function, the returned entity
// should be null().
entity_t * entity_check_result( entity_t * e ) {
  // internal storage of collided entity.
  static entity_t * self;
  // get the stored entity
  entity_t * r = self;
  // update the stored entity with e.
  self = e;
  // return stored entity
  return r;
}

void entity_clear_result() {
  entity_check_result(null());
}

int _entity_check_up( entity_t * self, int n, entity_t * other ) {
  if (!entity_solid(self) || !entity_solid(other)) return -1;

  bool_t b1 = inrange(entity_pos_x(self), entity_pos_x(other), entity_pos_x(other) + entity_size_x(other));
  bool_t b2 = inrange(entity_pos_x(self) + entity_size_x(self) - 1, entity_pos_x(other), entity_pos_x(other) + entity_size_x(other));
  bool_t b3 = inrange(entity_pos_x(other), entity_pos_x(self), entity_pos_x(self) + entity_size_x(self));
  bool_t b4 = inrange(entity_pos_x(other) + entity_size_x(other) - 1, entity_pos_x(self), entity_pos_x(self) + entity_size_x(self));

  // if one of the points of either entity bounding box is in the range
  if (b1 or b2 or b3 or b4) {
    // determine the distance from bottom of other to top of self
    int u = entity_pos_y(self) - (entity_pos_y(other) + entity_size_y(other) );
    
    if (u >= 0) {
      // return the lower of the two.
      return high(u,n);
    }
  }

  // no collision, return -1
  return -1;
}

int _entity_check_right( entity_t * self, int n, entity_t * other ) {
  if (!entity_solid(self) || !entity_solid(other)) return -1;
  bool_t b1 = inrange(entity_pos_y(self), entity_pos_y(other), entity_pos_y(other) + entity_size_y(other));
  bool_t b2 = inrange(entity_pos_y(self) + entity_size_y(self) - 1, entity_pos_y(other), entity_pos_y(other) + entity_size_y(other));
  bool_t b3 = inrange(entity_pos_y(other), entity_pos_y(self), entity_pos_y(self) + entity_size_y(self));
  bool_t b4 = inrange(entity_pos_y(other) + entity_size_y(other) - 1, entity_pos_y(self), entity_pos_y(self) + entity_size_y(self));

  // if one of the points of either entity bounding box is in the range
  if (b1 or b2 or b3 or b4) {
    // determine the distance from bottom of other to top of self
    int u = (entity_pos_x(other)) - (entity_pos_x(self) + entity_size_x(self));
    
    if (u >= 0) {
      // return the lower of the two.
      return high(u,n);
    }
  }

  // no collision, return -1
  return -1;
}

int _entity_check_down( entity_t * self, int n, entity_t * other ) {
  if (!entity_solid(self) || !entity_solid(other)) return -1;
  bool_t b1 = inrange(entity_pos_x(self), entity_pos_x(other), entity_pos_x(other) + entity_size_x(other));
  bool_t b2 = inrange(entity_pos_x(self) + entity_size_x(self) - 1, entity_pos_x(other), entity_pos_x(other) + entity_size_x(other));
  bool_t b3 = inrange(entity_pos_x(other), entity_pos_x(self), entity_pos_x(self) + entity_size_x(self));
  bool_t b4 = inrange(entity_pos_x(other) + entity_size_x(other) - 1, entity_pos_x(self), entity_pos_x(self) + entity_size_x(self));

  // if one of the points of either entity bounding box is in the range
  if (b1 or b2 or b3 or b4) {
    // determine the distance from bottom of other to top of self
    int u = entity_pos_y(other) - (entity_pos_y(self) + entity_size_y(other));
    
    if (u >= 0) {
      // return the lower of the two.
      return high(u,n);
    }
  }

  // no collision, return -1
  return -1;
}

int _entity_check_left( entity_t * self, int n, entity_t * other ) {
  if (!entity_solid(self) || !entity_solid(other)) return -1;
  bool_t b1 = inrange(entity_pos_y(self), entity_pos_y(other), entity_pos_y(other) + entity_size_y(other));
  bool_t b2 = inrange(entity_pos_y(self) + entity_size_y(self) - 1, entity_pos_y(other), entity_pos_y(other) + entity_size_y(other));
  bool_t b3 = inrange(entity_pos_y(other), entity_pos_y(self), entity_pos_y(self) + entity_size_y(self));
  bool_t b4 = inrange(entity_pos_y(other) + entity_size_y(other) - 1, entity_pos_y(self), entity_pos_y(self) + entity_size_y(self));

  // if one of the points of either entity bounding box is in the range
  if (b1 or b2 or b3 or b4) {
    // determine the distance from bottom of other to top of self
    int u = entity_pos_x(self) - (entity_pos_x(other) + entity_size_x(other)); 
    
    if (u >= 0) {
      // return the lower of the two.
      return high(u,n);
    }
  }

  // no collision, return -1
  return -1;
}


//////////////////////////


int entity_check_up( entity_t * self, int n, entity_t * other ) {
  int u = _entity_check_up( self, n, other);
  if (u >= 0) {
    if (u < n) {
      entity_check_result( other );
    }
    n = min(n,u);
  }
  return n;
}

int entity_check_up_list( entity_t * self, int n, list_t * l ) {
  int u;
  foreach(l,dt) {
    u = _entity_check_up( self, n, dt );
    if (u >= 0) {
      if (u < n) {
        entity_check_result( dt );
      }
      n = min(n,u);
    }
  }
  return n;
}

int entity_check_right( entity_t * self, int n, entity_t * other ) {
  int u = _entity_check_right( self, n, other );
  if (u >= 0) {
    if (u < n) {
      entity_check_result( other );
    }
    n = min(n,u);
  }
  return n;
}

int entity_check_right_list( entity_t * self, int n, list_t * l ) {
  int u;
  foreach(l,dt) {
    u = _entity_check_right( self, n, dt );
    if (u >= 0) {
      if (u < n) {
        entity_check_result( dt );
      }
      n = min(n,u);
    }
  }
  return n;
}

int entity_check_down( entity_t * self, int n, entity_t * other ) {
  int u = _entity_check_down( self, n, other );
  if (u >= 0) {
    if (u < n) {
      entity_check_result( other );
    }
    n = min(n,u);
  }
  return n;
}

int entity_check_down_list( entity_t * self, int n, list_t * l ) {
  int u;
  foreach(l,dt) {
    u = _entity_check_down( self, n, dt );
    if (u >= 0) {
      if (u < n) {
        entity_check_result( dt );
      }
      n = min(n,u);
    }
  }
  return n;
}

int entity_check_left( entity_t * self, int n, entity_t * other ) {
  int u = _entity_check_left( self, n, other);
  if (u >= 0) {
    if (u < n) {
      entity_check_result( other );
    }
    n = min(n,u);
  }
  return n;
}

int entity_check_left_list( entity_t * self, int n, list_t * l ) {
  int u;
  foreach(l,dt) {
    u = _entity_check_left( self, n, dt );
    if (u >= 0) {
      if (u < n) {
        entity_check_result( dt );
      }
      n = min(n,u);
    }
  }
  return n;
}