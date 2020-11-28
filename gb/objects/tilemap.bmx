
type t_tilemap
	field size :t_ipoint
	field data :t_ipoint3[,]
endtype

function new_tilemap:t_tilemap(x:int=40, y:int=24)
	x = low(x,1)
	y = low(y,1)
	local r:t_tilemap = new t_tilemap
	r.size = new_ipoint(x,y)
	r.data = new t_ipoint3[x,y]
	
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r.data[i,j] = new_ipoint3(0,0,0)
		next
	next
	
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function tilemap_set_tile(t:t_tilemap, x:int, y:int, ti:int, tx:int, ty:int)
	x = iwrap(x,0,t.size.x)
	y = iwrap(y,0,t.size.y)
	ipoint3_set( t.data[x,y], ti, tx, ty )
endfunction

function tilemap_get_tile_id:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x,y].x
endfunction

function tilemap_get_tile_x:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x,y].y
endfunction

function tilemap_get_tile_y:int(t:t_tilemap, x:int, y:int)
	x = iclamp(x,0,t.size.x-1)
	y = iclamp(y,0,t.size.y-1)
	return t.data[x,y].z
endfunction