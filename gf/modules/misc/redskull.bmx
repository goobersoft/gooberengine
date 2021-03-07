
''''''''''''''
'' redskull ''
''''''''''''''

type t_redskull
  field pos :t_point
endtype

function new_redskull:t_redskull(x:float=0, y:float=0)
  local r:t_redskull = new t_redskull
  r.pos = new_point(x,y)
  return r
endfunction

function redskull_update(r:t_redskull)
  local p:t_point = new_point( ..
    (gb_mouse_tile_x()-(r.pos.x+2))*0.01,
    (gb_mouse_tile_y()-(r.pos.y+2))*0.01)
  point_add( r.pos, frand(-0.05,0.05)+p.x, frand(-0.05,0.05)+p.y )
endfunction

function redskull_draw(r:t_redskull, x:float=0, y:float=0)
  local ff:float = 0
  local gg:float = 0
  if prob(20)
    gg = frand(0,1)
  else
    gb_graph_set_mode( gb_graph_mode_add )
  endif
  for local i:int = 0 to 39
    ff = float(i) * 0.1
    gb_graph_draw_tile_box(r.pos.x+x+frand(-gg,gg), r.pos.y+y+ff, 1, 11, float(i)*0.1, 4, 0.1)
  next
  gb_graph_set_mode( gb_graph_mode_normal )
endfunction