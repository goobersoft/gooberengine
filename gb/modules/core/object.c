
/////////
// object //
/////////
//
// since C does not really have a way to let us know
// if a pointer is a certain type (we do this ourselves),
// the object object helps us to determine what an object is
// based on its class string.
//
// A object has a class string, an id string, and a source
// pointer to the owner of the object. 
//
// The "class" field
// helps us identify the type of object we are using. This 
// is usually handled by the parent object constructor.
//
// The "id" fiield uniquely identifies the parent source. Let's say
// we have a bunch of scenes want to use for a game. Each scene needs to be
// uniquely identified. So, we could have three different scenes with the
// ids "title", "main", and "credits". The top-layer object which is handling
// the game's state (let's say 'app' is the top layer object), the app would
// have a pointer to a scene, but not of a specific type of scene. To help the
// code know what type of scene it is, we would use the code below to grab the
// id of the scene:
//
// object_id(scene_object(app->scene))
// or if the macro was defined to make the object transparent,
// scene_id(app->scene) 
//
// macro equates to a path like:
// app->scene->object->id
// 
// Since the scene object has a object object, we can then cast appropriate types
// based on the ID of the scene:
//
// char * t = scene_id(app->scene);
// if streq(t,"title") {
//   scene_title_t * u = cast(scene_source(app->scene),scene_title_t*);
//   ... do specific stuff for the specific scene object here
// }

type() {
  // a string literal which represents the class of the object
  // containing this object
  foreign( char * class );
  // specific data
  foreign( void * data );

} object_t;

// getters
#define object_class(self)   (self->class)
#define object_data(self)    (self->data)


/////////
// new //
/////////

void object_init( object_t * self, char * cls, void * dt ) {
  object_class(self)     = cls;
  object_data(self)      = dt;
}

// src: source pointer back to owner of object
// cls: a string indicating the type of the object.
object_t * object( char * cls, void * dt ) {
  object_t * self = alloc(object_t);
  object_init(self,cls,dt);
  return self;
}

// obligatory free function.
// good convention, even though the object itself does not
// contain any local pointers, all it needs to do is
// free its own pointer.
void free_object( object_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

// set the class of the object to a string constant.
// you could theoretically use an allocated string,
// but it is not recommended.
void object_set_class( object_t * self, char * cs ) {
  object_class(self) = cs;
}

// set the ID of the object to a string constant.
void object_set_data( object_t * self, void * dt ) {
  object_data(self) = dt;
}

void object_set( object_t * self, char * cs, void * dt ) {
  object_set_class(self,cs);
  object_set_data(self,dt);
}

bool_t object_has_class( object_t * self, char * cs ) {
  return streq(object_class(self),cs);
}

bool_t object_has_data( object_t * self ) {
  return (object_data(self)!=null());
}