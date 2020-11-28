
''''''''''
'' type ''
''''''''''

Type t_point
	Field x :Float
	Field y :Float
endtype

function new_point:t_point(x:float,y:float)
	local r:t_point = new t_point
	r.x = x
	r.y = y
	return r
endfunction

function clone_point:t_point(p:t_point)
	local r:t_point = new t_point
	r.x = p.x
	r.y = p.y
	return r
endfunction

function point_set(p:t_point, x:float, y:float)
	p.x = x
	p.y = y
endfunction

function point_add(p:t_point, x:float, y:float)
	p.x = p.x + x
	p.y = p.y + y
endfunction

function point_mul(p:t_point, x:float, y:float)
	p.x = p.x * x
	p.y = p.y * y
endfunction

function point_div(p:t_point, x:float, y:float)
	if (x <> 0) and (y <> 0)
		p.x = p.x / x
		p.y = p.y / y
	endif
endfunction

function point_eq:int(p:t_point, x:float, y:float)
	if (p.x = x) and (p.y = y) return true else return false
endfunction

function point_neq:int(p:t_point, x:float, y:float)
	if (p.x <> x) and (p.y <> y) return true else return false
endfunction

function point_tostring:string(p:t_point)
		return "{" + p.x + "," + p.y + "}"
endfunction

''''''''''''
'' ipoint ''
''''''''''''

Type t_ipoint
	Field x :int
	Field y :int
endtype

function new_ipoint:t_ipoint(x:int,y:int)
	local r:t_ipoint = new t_ipoint
	r.x = x
	r.y = y
	return r
endfunction

function clone_ipoint:t_ipoint(p:t_ipoint)
	local r:t_ipoint = new t_ipoint
	r.x = p.x
	r.y = p.y
	return r
endfunction

function ipoint_set(p:t_ipoint, x:int, y:int)
	p.x = x
	p.y = y
endfunction

function ipoint_add(p:t_ipoint, x:int, y:int)
	p.x = p.x + x
	p.y = p.y + y
endfunction

function ipoint_mul(p:t_ipoint, x:int, y:int)
	p.x = p.x * x
	p.y = p.y * y
endfunction

function ipoint_div(p:t_ipoint, x:int, y:int)
	if (x <> 0) and (y <> 0)
		p.x = p.x / x
		p.y = p.y / y
	endif
endfunction

function ipoint_eq:int(p:t_ipoint, x:int, y:int)
	if (p.x = x) and (p.y = y) return true else return false
endfunction

function ipoint_neq:int(p:t_ipoint, x:int, y:int)
	if (p.x <> x) and (p.y <> y) return true else return false
endfunction

function ipoint_tostring:string(p:t_ipoint)
		return "{" + p.x + "," + p.y + "}"
endfunction
