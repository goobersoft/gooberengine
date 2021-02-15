
include "gball/response.bmx"

''''''''''
'' type ''
''''''''''

const gball_state_idle        :int = 0
const gball_state_shaking     :int = 1
const gball_state_responding  :int = 2

const gball_shake_sine        :float = 2.0


type t_gball
  field response    :t_gball_response
  field state       :t_int
  field statetime   :t_float

  field shaketime   :t_float
  field shake       :t_float
  field drip        :t_float
  field dripvelo    :t_float
endtype

function new_gball:t_gball()
  local r:t_gball = new t_gball
  r.response    = new_gball_response("Waiting...",0)
  r.state       = new_int(0)
  r.statetime   = new_float(0)
  
  r.shaketime   = new_float(1)
  r.shake       = new_float(0)

  r.drip        = new_float(2)
  r.dripvelo    = new_float(0)
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

''''''''''''
'' events ''
''''''''''''

function gball_update(g:t_gball)

  if keyhit(key_space)
    int_set(g.state, gball_state_shaking)
    float_set(g.shaketime, 1)
  endif


  if g.state.get() = gball_state_shaking
    float_set(g.shake, g.shaketime.get() * rot_x(gb_uptime()*4000))
    float_add(g.shaketime,-gb_delta())
    float_set(g.drip, clamp(g.drip.value+0.2,0,2))
    float_set(g.dripvelo, 0)

    if g.shaketime.get() <= 0
      float_set(g.shaketime,0)
      int_set(g.state, gball_state_responding )
      float_set(g.statetime, 1)
      gball_response_generate(g.response)
    endif

  elseif g.state.get() = gball_state_responding
    float_set(g.shake, 0)
    float_add(g.dripvelo, -0.02)
    float_add(g.drip, g.dripvelo.get())
    float_add(g.statetime, -gb_delta())
    if g.statetime.get() < 0
      int_set(g.state, gball_state_idle)
    endif
  endif

  if g.drip.get() < 0
    g.dripvelo.value = abs(g.dripvelo.value) * 0.5
    g.drip.value = 0
  endif

endfunction

function gball_draw(g:t_gball, x:float=0, y:float=0)
  local ff:float = g.shake.get()
  gb_graph_draw_tile_box(x+1+ff,   y,                 1, 5,2,3,3)
  gb_graph_draw_tile_box(x+1.5+ff, y+1+g.drip.get(),  1, 5+(g.response.intent.get()*2),0,2,2)
  gb_graph_draw_tile_box(x+ff,     y,                 1, 0,0,5,5)


  select g.response.intent.get()
    case 0
      gb_graph_set_color(2,2,2)
    case 1
      gb_graph_set_color(1,2,0)
    case 2
      gb_graph_set_color(3,0,2)
  endselect

  if g.state.get() = gball_state_idle
    gb_graph_draw_tile_text(x+5.2+ff,y+1.5,2,g.response.value.get())
  endif

  gb_graph_reset()
endfunction