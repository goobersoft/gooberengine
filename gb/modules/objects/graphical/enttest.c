
type() {

  entity_t * entity;

} enttest_t;

#define enttest_entity(self) (self->entity)

void init_enttest( enttest_t * self ) {
  enttest_entity(self) = entity();
}

enttest_t * enttest() {
  enttest_t * self = alloc(enttest_t);
  init_enttest(self);
  return self;
}