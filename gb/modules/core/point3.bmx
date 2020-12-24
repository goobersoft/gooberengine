
''''''''''
'' type ''
''''''''''

Type t_point3
	Field x :Float
	Field y :Float
	field z :float
endtype

function new_point3:t_point3(x:float,y:float,z:float)
	local r:t_point3 = new t_point3
	r.x = x
	r.y = y
	r.z = z
	return r
endfunction

function clone_point3:t_point3(p:t_point3)
	local r:t_point3 = new t_point3
	r.x = p.x
	r.y = p.y
	r.z = p.z
	return r
endfunction

function point3_set(p:t_point3, x:float, y:float,z:float)
	p.x = x
	p.y = y
	p.z = z
endfunction

function point3_add(p:t_point3, x:float, y:float, z:float)
	p.x = p.x + x
	p.y = p.y + y
	p.z = p.z + z
endfunction

function point3_mul(p:t_point3, x:float, y:float, z:float)
	p.x = p.x * x
	p.y = p.y * y
	p.z = p.z * z
endfunction

function point3_div(p:t_point3, x:float, y:float, z:float)
	if (x <> 0) and (y <> 0) and (z <> 0)
		p.x = p.x / x
		p.y = p.y / y
		p.z = p.z / z
	endif
endfunction

function point3_eq:int(p:t_point3, x:float, y:float, z:float)
	if (p.x = x) and (p.y = y) and (p.z = z) return true else return false
endfunction

function point3_neq:int(p:t_point3, x:float, y:float, z:float)
	if (p.x <> x) and (p.y <> y) and (p.z <> z) return true else return false
endfunction

function point3_tostring:string(p:t_point3)
		return "{" + p.x + "," + p.y + "," + p.z + "}"
endfunction

''''''''''''
'' ipoint ''
''''''''''''

Type t_ipoint3
	Field x :int
	Field y :int
	field z :int
endtype

function new_ipoint3:t_ipoint3(x:int,y:int,z:int)
	local r:t_ipoint3 = new t_ipoint3
	r.x = x
	r.y = y
	r.z = z
	return r
endfunction

function clone_ipoint3:t_ipoint3(p:t_ipoint3)
	local r:t_ipoint3 = new t_ipoint3
	r.x = p.x
	r.y = p.y
	r.z = p.z
	return r
endfunction

function ipoint3_set(p:t_ipoint3, x:int, y:int,z:int)
	p.x = x
	p.y = y
	p.z = z
endfunction

function ipoint3_add(p:t_ipoint3, x:int, y:int, z:int)
	p.x = p.x + x
	p.y = p.y + y
	p.z = p.z + z
endfunction

function ipoint3_mul(p:t_ipoint3, x:int, y:int, z:int)
	p.x = p.x * x
	p.y = p.y * y
	p.z = p.z * z
endfunction

function ipoint3_div(p:t_ipoint3, x:int, y:int, z:int)
	if (x <> 0) and (y <> 0) and (z <> 0)
		p.x = p.x / x
		p.y = p.y / y
		p.z = p.z / z
	endif
endfunction

function ipoint3_eq:int(p:t_ipoint3, x:int, y:int, z:int)
	if (p.x = x) and (p.y = y) and (p.z = z) return true else return false
endfunction

function ipoint3_neq:int(p:t_ipoint3, x:int, y:int, z:int)
	if (p.x <> x) and (p.y <> y) and (p.z <> z) return true else return false
endfunction

function ipoint3_tostring:string(p:t_ipoint3)
		return "{" + p.x + "," + p.y + "," + p.z + "}"
endfunction
