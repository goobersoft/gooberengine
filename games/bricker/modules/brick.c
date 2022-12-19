
#define brick_id_none() 0

type() {

  tag_t     * tag;
  entity_t  * entity;

} brick_t;

#define brick_tag(self)     (self->tag);
#define brick_id(self)      tag_id(self->tag)
#define brick_entity(self)  (self->entity)

/////////
// new //
/////////

brick_t * brick(char * id) {
  brick_t * self      = alloc(brick_t);
  brick_tag(self)     = tag(self,"brick");
  brick_entity(self)  = entity(self);
  return self;
}
