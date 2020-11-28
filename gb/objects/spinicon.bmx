type t_spinicon
	field pos 				:t_point
	field angle 			:t_number
	field speed				:t_float
	field iconpos			:t_point3
	field iconlength	:t_int
endtype

function new_spinicon:t_spinicon(im:int=0, tx:float=30, ty:float=23, l:int=10)
	local r:t_spinicon = new t_spinicon
	r.pos 				= new_point(0,0)
	r.angle 			= new_number_angle(0)
	r.speed 			= new_float(250)
	r.iconpos 		= new_point3(im,tx,ty)
	r.iconlength 	= new_int(l)
	return r
endfunction

function spinicon_update(s:t_spinicon, d:float)
	number_add(s.angle, s.speed.value * d)
endfunction

function spinicon_draw(s:t_spinicon, x:float=0, y:float=0)
	gb_graph_set_handle(0.5, 0.5)
	gb_graph_set_angle( s.angle.value )
	for local i:int = 0 to s.iconlength.value-1
		gb_graph_draw_tile(s.pos.x+x, s.pos.y+y-(0.1*float(i)),
			int(s.iconpos.x), s.iconpos.y+float(i), s.iconpos.z)
	next
	gb_graph_reset()
endfunction