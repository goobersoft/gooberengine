
#define statpanel_pos_bottom() 0
#define statpanel_pos_top()    1

#define statpanel_max_text()       100
#define statpanel_text_pos_fps(n)  n+0
#define statpanel_text_pos_cpu(n)  n+10

type() {

  bool_t  visible;
  bool_t  pos;
  char    text[statpanel_max_text()];

} statpanel_t;

#define statpanel_visible(self) (self->visible)
#define statpanel_pos(self)     (self->pos)
#define statpanel_text(self,a)  (self->text[a])

/////////
// new //
/////////

statpanel_t * statpanel() {
  statpanel_t * self        = alloc(statpanel_t);
  statpanel_visible(self)   = true();
  statpanel_pos(self)       = statpanel_pos_bottom();
  loop(i,0,statpanel_max_text()) {
    statpanel_text(self,i) = '\0';
  }
  return self;
}

////////////
// events //
////////////

void statpanel_update( statpanel_t * self ) {

}