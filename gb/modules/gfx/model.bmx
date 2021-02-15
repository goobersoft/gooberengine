
type t_model
  field color   :t_color
  field points  :t_point3[]
  field tris    :t_tri[]
  field center  :t_point3
endtype

function new_model_cube:t_model(x:float, y:float, z:float, s:float, c:t_color=null)
  
  local r:t_model = new t_model
  r.center = new_point3(x + (s/2), y + (s/2), z + (s/2))

  local p:t_point3[] = new t_point3[8]
  p[0] = new_point3(x,   y,   z)
  p[1] = new_point3(x+s, y,   z)
  p[2] = new_point3(x,   y+s, z)
  p[3] = new_point3(x+s, y+s, z)
  p[4] = new_point3(x,   y,   z+s)
  p[5] = new_point3(x+s, y,   z+s)
  p[6] = new_point3(x,   y+s, z+s)
  p[7] = new_point3(x+s, y+s, z+s)
  r.points = p

  local t:t_tri[] = new t_tri[12]
  t[0]  = new_tri(p[0], p[1], p[2], new_color_random())
  t[1]  = new_tri(p[1], p[2], p[3], new_color_random())
  t[2]  = new_tri(p[1], p[5], p[3], new_color_random())
  t[3]  = new_tri(p[5], p[3], p[7], new_color_random())
  t[4]  = new_tri(p[5], p[4], p[7], new_color_random())
  t[5]  = new_tri(p[4], p[7], p[6], new_color_random())
  t[6]  = new_tri(p[4], p[0], p[6], new_color_random())
  t[7]  = new_tri(p[0], p[6], p[2], new_color_random())
  t[8]  = new_tri(p[0], p[1], p[4], new_color_random())
  t[9]  = new_tri(p[1], p[4], p[5], new_color_random())
  t[10] = new_tri(p[3], p[2], p[7], new_color_random())
  t[11] = new_tri(p[2], p[7], p[6], new_color_random())
  r.tris = t

  
  if c
    r.color = c
  else
    r.color = new_color(3,3,3,3)
  endif
  
  return r
endfunction

''       4---5
''      /   /|
''6--> 0---1 7
''     |   |/
''     2---3

''''''''''''
'' events ''
''''''''''''

function model_draw_wire(m:t_model, x:float=0, y:float=0)
  gb_graph_set_mode(gb_graph_mode_add)
  for local i:int = 0 to m.tris.length-1
    gb_graph_set_color(m.tris[i].color)
    gb_graph_draw_tri_wire(m.tris[i].points[0],
      m.tris[i].points[1], m.tris[i].points[2])
  next
  gb_graph_reset()
endfunction

function model_draw_wire3d(m:t_model, x:float=0, y:float=0, z:float=0)
  gb_graph_set_mode(gb_graph_mode_add)
  for local i:int = 0 to m.tris.length-1
    gb_graph_set_color(m.tris[i].color)
    gb_graph_draw_tri_wire3d(m.tris[i].points[0], m.tris[i].points[1], m.tris[i].points[2])
  next
  gb_graph_reset()
endfunction

function model_draw(m:t_model, x:float=0, y:float=0)
endfunction

function model_draw3d(m:t_model, x:float=0, y:float=0, z:float=0)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function model_move_center(m:t_model, x:float, y:float, z:float)
  point3_set(m.center, x, y, z)
endfunction

function model_move(m:t_model, x:float, y:float, z:float)
  local p:t_point3 = new_point3( x-m.center.x, y-m.center.y, z-m.center.z )
  point3_set(m.center, x, y, z)
  for local i:int = 0 to m.points.length-1
    point3_add(m.points[i], p)
  next
endfunction

function model_calc_center(m:t_model)
  local xx:float = 0
  local yy:float = 0
  local zz:float = 0
  for local i:int = 0 to m.points.length-1
    xx = xx + m.points[i].x
    yy = yy + m.points[i].y
    zz = zz + m.points[i].z
  next
  xx = xx / m.points.length
  yy = yy / m.points.length
  zz = zz / m.points.length
  point3_set(m.center, xx, yy, zz)
endfunction