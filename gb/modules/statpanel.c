
#define statpanel_pos_bottom() 0
#define statpanel_pos_top()    1

#define statpanel_max_text()       100
#define statpanel_text_pos_fps(n)  n+0
#define statpanel_text_pos_cpu(n)  n+10

type() {

  bool_t  pos;
  char    text[statpanel_max_text()];

} statpanel_t;

#define statpanel_pos(self) (self->pos)