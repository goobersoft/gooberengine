
type t_tilemap
	field size :t_ipoint
	field data :t_ipoint3[][]
endtype

function new_tilemap:t_tilemap(x:int=40, y:int=24)
	x = low(x,1)
	y = low(y,1)
	local r:t_tilemap = new t_tilemap
	r.size = new_ipoint(x,y)

	r.data = new t_ipoint3[x][]
	for local i:int = 0 to r.size.x-1
    r.data[i] = new t_ipoint3[y]
		for local j:int = 0 to r.size.y-1
			r.data[i][j] = new_ipoint3(0,0,0)
		next
	next
	
	return r
endfunction

function load_tilemap:t_tilemap(s:string)
  local d:t_dict = gbml_load(s)
  local v:string[]
  local w:string
  local r:t_tilemap = new_tilemap(dict_read_int(d,"size-x"),dict_read_int(d,"size-y"))
  
  for local i:int = 0 to r.size.x-1
    for local j:int = 0 to r.size.y-1
      w = dict_read_string(d,"data/"+i+","+j)
      v = tokenize(w,",")
      ipoint3_set(r.data[i][j], int(v[0]), int(v[1]), int(v[2]))
    next
  next
  return r
endfunction

function tilemap_draw(t:t_tilemap, x:float=0, y:float=0)
  for local i:int = 0 to t.size.x-1
    for local j:int = 0 to t.size.y-1
      gb_graph_draw_tile(x+i, y+j, t.data[i][j].x, t.data[i][j].y, t.data[i][j].z)
    next
  next
endfunction

function tilemap_draw3d(t:t_tilemap, x:float=0, y:float=0, z:float=0)
  for local i:int = 0 to t.size.x-1
    for local j:int = 0 to t.size.y-1
      gb_graph_draw_tile3d(x+(i*gb_tilesize), y+(j*gb_tilesize), z, t.data[i][j].x, t.data[i][j].y, t.data[i][j].z)
    next
  next
endfunction

'' plot = to canvas, not to screen.
function tilemap_plot(t:t_tilemap, x:float=0, y:float=0)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function tilemap_set_tile(t:t_tilemap, x:int, y:int, ti:int, tx:int, ty:int)
	x = iwrap(x,0,t.size.x)
	y = iwrap(y,0,t.size.y)
	ipoint3_set( t.data[x][y], ti, tx, ty )
endfunction

function tilemap_get_tile_id:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x][y].x
endfunction

function tilemap_get_tile_x:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x][y].y
endfunction

function tilemap_get_tile_y:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x][y].z
endfunction