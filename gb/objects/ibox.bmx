
Type t_ibox
	Field id 			:int
	Field x				:float
	field y				:float
	field w				:float
	field h				:float
endtype

function new_ibox :t_ibox( id:int=0, x:float=0, y:float=0, w:float=1, h:float=1)
	local r:t_ibox = new t_ibox
	r.id = low(id,0)
	r.x = x
	r.y = y
	r.w = low(w,1)
	r.h = low(h,1)
	return r
endfunction

function ibox_set(b:t_ibox, id:int, x:float, y:float, w:float, h:float)
	b.id = low(id,0)
	b.x = x
	b.y = y
	b.w = low(w,1)
	b.h = low(h,1)
endfunction

function ibox_set_id(b:t_ibox, id:int)
	b.id = low(id,0)
endfunction

function ibox_set_rect(b:t_ibox, x:float, y:float, w:float, h:float)
	b.x = x
	b.y = y
	b.w = low(w,1)
	b.h = low(h,1)
endfunction

function ibox_draw(i:t_ibox, x:float, y:float)
	gb_graph_draw_tile_box(x,y, i.id, i.x, i.y, i.w, i.h)
endfunction
