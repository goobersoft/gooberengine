
/////////
// Tag //
/////////
//
// since C does not really have a way to let us know
// if a pointer is a certain type (we do this ourselves),
// the tag object helps us to determine what an object is
// based on its class string.
//
// A tag has a class string, an id string, and a source
// pointer to the owner of the tag. 
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
// tag_id(scene_tag(app->scene))
// or if the macro was defined to make the tag transparent,
// scene_id(app->scene) 
//
// macro equates to a path like:
// app->scene->tag->id
// 
// Since the scene object has a tag object, we can then cast appropriate types
// based on the ID of the scene:
//
// char * t = scene_id(app->scene);
// if streq(t,"title") {
//   scene_title_t * u = cast(scene_source(app->scene),scene_title_t*);
//   ... do specific stuff for the specific scene object here
// }

#define tag_max_id_length() 32

type() {

  // a generic pointer to another object.
  foreign( void * source );
  // a string literal which represents the class of the object
  // containing this tag
  foreign( char * class );
  // a string literal which uniquely identifies the object
  // containing this tag
  foreign( char * id );

} tag_t;

// getters
#define tag_source(self)       (self->source)
#define tag_class(self)        (self->class)
#define tag_id(self)           (self->id)


/////////
// new //
/////////

void tag_init( tag_t * self, void * src, char * cls ) {
  tag_source(self)    = src;
  tag_class(self)     = cls;
  tag_id(self)        = "";
}

// src: source pointer back to owner of object
// cls: a string indicating the type of the object.
tag_t * tag( void * src, char * cls ) {
  tag_t * r = alloc(tag_t);
  tag_init(r,src,cls);
  return r;
}

// obligatory free function.
// good convention, even though the tag itself does not
// contain any local pointers, all it needs to do is
// free its own pointer.
void free_tag( tag_t * self ) {
  free(self);
}

///////////////
// functions //
///////////////

// set the pointer of the tag's owner
void tag_set_source( tag_t * self, void * s ) {
  tag_source(self) = s;
}

// set the class of the tag to a string constant.
// you could theoretically use an allocated string,
// but it is not recommended.
void tag_set_class( tag_t * self, char * cs ) {
  tag_class(self) = cs;
}

// set the ID of the tag to a string constant.
void tag_set_id( tag_t * self, char * id ) {
  tag_id(self) = id;
}

