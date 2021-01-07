
''''''''''
'' type ''
''''''''''

type t_tri
  field color   :t_color
  field points  :t_point3[]
endtype

'''''''''
'' new ''
'''''''''

function new_tri:t_tri(p1:t_point3, p2:t_point3, p3:t_point3, c:t_color=null)
  local r:t_tri = new t_tri
  if c
    r.color       = c
  else
    r.color       = new_color(3,3,3,3)
  endif
  r.points      = new t_point3[3]
  r.points[0]   = p1
  r.points[1]   = p2
  r.points[2]   = p3
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function tri_draw(t:t_tri)
  gb_graph_draw_tri( t.points[0], t.points[1], t.points[2] )
endfunction

function tri_draw3d(t:t_tri)
  gb_graph_draw_tri3d( t.points[0], t.points[1], t.points[2] )
endfunction

function tri_draw_wire(t:t_tri)
  gb_graph_draw_tri_wire( t.points[0], t.points[1], t.points[2] )
endfunction

function tri_draw_wire3d(t:t_tri)
  gb_graph_draw_tri_wire3d( t.points[0], t.points[1], t.points[2] )
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function tri_center:t_point3(t:t_tri)
  local xx:float = (t.points[0].x + t.points[1].x + t.points[2].x) / 3 
  local yy:float = (t.points[0].y + t.points[1].y + t.points[2].y) / 3 
  local zz:float = (t.points[0].z + t.points[1].z + t.points[2].z) / 3 
  return new_point3( xx, yy, zz )
endfunction

function tri_get_point:t_point3(t:t_tri, n:int)
  return t.points[ iclamp(n,0,2) ]
endfunction

function tri_set_point(t:t_tri, n:int, p:t_point3)
  t.points[ iclamp(n,0,2) ] = p
endfunction

function tri_get_color:t_color(t:t_tri)
  return t.color
endfunction

function tri_set_color(t:t_tri, c:t_color=null)
  t.color = c
endfunction