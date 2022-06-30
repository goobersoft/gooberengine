
type() {

  char name[32];

  void(*fn_init)(void);
  void(*fn_load)(void);
  void(*fn_start)(void);
  void(*fn_update)(void);
  void(*fn_draw)(void);
  void(*fn_quit)(void);

} cartridge_t;

#define cartridge_name(self) (self->name)

#define cartridge_fn_init(self)   (self->fn_init)
#define cartridge_fn_load(self)   (self->fn_load)
#define cartridge_fn_start(self)  (self->fn_start)
#define cartridge_fn_update(self) (self->fn_update)
#define cartridge_fn_draw(self)   (self->fn_draw)
#define cartridge_fn_quit(self)   (self->fn_quit)

/////////
// new //
/////////

void cartridge_init( cartridge_t * self ) {
  strcpy( cartridge_name(self), "Untitled" );

  cartridge_fn_init(self)   = emp;
  cartridge_fn_load(self)   = emp;
  cartridge_fn_start(self)  = emp;
  cartridge_fn_update(self) = emp;
  cartridge_fn_draw(self)   = emp;
  cartridge_fn_quit(self)   = emp;
}

cartridge_t * cartridge() {
  cartridge_t * r = alloc(cartridge_t);
  cartridge_init(r);
  return r;
}

///////////////
// functions //
///////////////

void cartridge_set_name( cartridge_t * c, char * n ) {
  strcpy( cartridge_name(c), n );
}