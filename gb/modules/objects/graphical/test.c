
type() {

  entity_t * entity;

} entity_test_t;

#define entity_test_entity(self) (self->entity)

void init_entity_test( entity_test_t * self ) {
  entity_test_entity(self) = entity("test",self);
}

entity_test_t * entity_test() {
  entity_test_t * self = alloc(entity_test_t);
  init_entity_test(self);
  return self;
}