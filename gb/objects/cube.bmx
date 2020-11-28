
''''''''''
'' type ''
''''''''''

Type t_cube
	Field x :float
	field y	:float
	field z :float
	field w	:float
	field h :float
	field d :float
endtype

function new_cube:t_cube(x:float, y:float, z:float, w:float, h:float, d:float)
	local r:t_cube = new t_cube
	r.x = x
	r.y = y
	r.z = z
	r.w = low(w,0)
	r.h = low(h,0)
	r.d = low(d,0)
	return r
endfunction

function cube_set(r:t_cube, x:float, y:float, z:float, w:float, h:float, d:float)
	r.x = x
	r.y = y
	r.z = z
	r.w = low(h,0)
	r.h = low(h,0)
	r.d = low(d,0)
endfunction
	
function cube_set_pos(r:t_cube, x:float, y:float, z:float)
	r.x = x
	r.y = y
	r.z = z
endfunction
	
function cube_add_pos(r:t_cube, x:float, y:float, z:Float)
	r.x = r.x + x
	r.y = r.y + y
	r.z = r.z + z
endfunction

function cube_set_size(r:t_cube, w:float, h:float, d:float)
	r.w = low(w,0)
	r.h = low(h,0)
	r.d = low(d,0)
endfunction

function cube_colliding :int(a:t_cube, b:t_cube)
	return cube_in_cube( a.x, a.y, a.z, a.w, a.h, a.d, 
		b.x, b.y, b.z, b.w, b.h, b.d )
endfunction
	
function cube_slide_check:float(a:t_cube, d:byte, m:float, b:t_cube)
	Local v :Float = 0
	Local q :Float = m
	Select d
		Case dir_north
			If (line_in_range( a.x, a.x+a.w, b.x, b.x+b.w ))
				v = a.y - (b.y + b.h)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_east
			If (line_in_range( a.y, a.y + a.h, b.y, b.y + b.h))
				v = b.x - (a.x + a.w)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_south
			If (line_in_range( a.x, a.x + a.w, b.x, b.x + b.w))
				v = b.y - (a.y + a.h)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_west
			If (line_in_range( a.y, a.y + a.h, b.y, b.y + b.h))
				v = a.x - (b.x + b.w)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_up
			If line_in_range( a.x, a.x+a.w, b.x, b.x+b.w )
				If line_in_range( a.y, a.y+a.h, b.y, b.y+b.h )
					v = a.z - (b.z + b.d)
					If (v >= 0)
						q = Min(m,v)
					Else
						q = m
					EndIf
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_down
			If line_in_range( a.x, a.x+a.w, b.x, b.x+b.w )
				If line_in_range( a.y, a.y+a.h, b.y, b.y+b.h )
					v = b.z - (a.z + a.d)
					If (v >= 0)
						q = Min(m,v)
					Else
						q = m
					EndIf
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
	EndSelect
	Return q
endfunction
	
function cube_slide:Float(a:t_cube, d:Byte, m:Float, b:t_cube)
	Local u :Float = cube_slide_check(a,d,m,b)
	Select (d)
			Case dir_north
				a.y = a.y - u
			Case dir_east
				a.x = a.x + u
			Case dir_south
				a.y = a.y + u
			Case dir_west
				a.x = a.x - u
			Case dir_up
				a.z = a.z - u
			Case dir_down
				a.z = a.z + u
		EndSelect
	Return u
endfunction
	
function cube_touching:int(a:t_cube, d:Byte, b:t_cube)
	Select d
			Case dir_north
				Return cube_in_cube( 	..
					a.x, 							..
					a.y-0.1, 					..
					a.z, 							..
					a.w-0.1, 				..
					0.1,								..
					a.d-0.1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_east
				Return cube_in_cube( 	..
					a.x+a.w, 			..
					a.y, 							..
					a.z, 							..
					0.1, 								..
					a.h-0.1,					..
					a.d-0.1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_south
				Return cube_in_cube( 	..
					a.x, 							..
					a.y+a.h, 			..
					a.z, 							..
					a.w-0.1, 				..
					0.1,								..
					a.d-0.1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_west
				Return cube_in_cube( 	..
					a.x-0.1, 					..
					a.y, 							..
					a.z, 							..
					0.1, 								..
					a.h-0.1,					..
					a.d-0.1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_up
				Return cube_in_cube( 	..
					a.x, 							..
					a.y, 							..
					a.z-0.1, 					..
					a.w-0.1, 				..
					a.h-0.1,					..
					0.1,								..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_down
				Return cube_in_cube( 	..
					a.x, 							..
					a.y, 							..
					a.z+a.d, 			..
					a.w-0.1, 				..
					a.h-0.1,					..
					0.1,								..
					b.x, b.y, b.z, b.w, b.h, b.d )
		EndSelect
	Return False
endfunction
	
function cube_tostring:string(r:t_cube)
	return "{" + r.x + "," + r.y + "," + r.z + "/" + r.w + "," + r.y + "," + r.d + "}"
endfunction

''''''''''''''''''
'' integer cube ''
''''''''''''''''''

Type t_icube
	Field x :int
	field y	:int
	field z :int
	field w	:int
	field h :int
	field d :int
endtype

function new_icube:t_icube(x:int, y:int, z:int, w:int, h:int, d:int)
	local r:t_icube = new t_icube
	r.x = x
	r.y = y
	r.z = z
	r.w = low(w,0)
	r.h = low(h,0)
	r.d = low(d,0)
	return r
endfunction

function icube_set(r:t_icube, x:int, y:int, z:int, w:int, h:int, d:int)
	r.x = x
	r.y = y
	r.z = z
	r.w = low(h,0)
	r.h = low(h,0)
	r.d = low(d,0)
endfunction
	
function icube_set_pos(r:t_icube, x:int, y:int, z:int)
	r.x = x
	r.y = y
	r.z = z
endfunction
	
function icube_add_pos(r:t_icube, x:int, y:int, z:int)
	r.x = r.x + x
	r.y = r.y + y
	r.z = r.z + z
endfunction

function icube_set_size(r:t_icube, w:int, h:int, d:int)
	r.w = low(w,0)
	r.h = low(h,0)
	r.d = low(d,0)
endfunction

function icube_colliding :int(a:t_icube, b:t_icube)
	return icube_in_icube( a.x, a.y, a.z, a.w, a.h, a.d, 
		b.x, b.y, b.z, b.w, b.h, b.d )
endfunction
	
function icube_slide_check:int(a:t_icube, d:byte, m:int, b:t_icube)
	Local v :int = 0
	Local q :int = m
	Select d
		Case dir_north
			If (line_in_range( a.x, a.x+a.w, b.x, b.x+b.w ))
				v = a.y - (b.y + b.h)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_east
			If (line_in_range( a.y, a.y + a.h, b.y, b.y + b.h))
				v = b.x - (a.x + a.w)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_south
			If (line_in_range( a.x, a.x + a.w, b.x, b.x + b.w))
				v = b.y - (a.y + a.h)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_west
			If (line_in_range( a.y, a.y + a.h, b.y, b.y + b.h))
				v = a.x - (b.x + b.w)
				If (v >= 0)
					q = Min(m,v)
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_up
			If line_in_range( a.x, a.x+a.w, b.x, b.x+b.w )
				If line_in_range( a.y, a.y+a.h, b.y, b.y+b.h )
					v = a.z - (b.z + b.d)
					If (v >= 0)
						q = Min(m,v)
					Else
						q = m
					EndIf
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
		Case dir_down
			If line_in_range( a.x, a.x+a.w, b.x, b.x+b.w )
				If line_in_range( a.y, a.y+a.h, b.y, b.y+b.h )
					v = b.z - (a.z + a.d)
					If (v >= 0)
						q = Min(m,v)
					Else
						q = m
					EndIf
				Else
					q = m
				EndIf
			Else
				q = m
			EndIf
	EndSelect
	Return q
endfunction
	
function icube_slide:int(a:t_icube, d:Byte, m:int, b:t_icube)
	Local u :int = icube_slide_check(a,d,m,b)
	Select (d)
			Case dir_north
				a.y = a.y - u
			Case dir_east
				a.x = a.x + u
			Case dir_south
				a.y = a.y + u
			Case dir_west
				a.x = a.x - u
			Case dir_up
				a.z = a.z - u
			Case dir_down
				a.z = a.z + u
		EndSelect
	Return u
endfunction
	
function icube_touching:int(a:t_icube, d:Byte, b:t_icube)
	Select d
			Case dir_north
				Return icube_in_icube( 	..
					a.x, 							..
					a.y-1, 					..
					a.z, 							..
					a.w-1, 				..
					1,								..
					a.d-1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_east
				Return icube_in_icube( 	..
					a.x+a.w, 			..
					a.y, 							..
					a.z, 							..
					1, 								..
					a.h-1,					..
					a.d-1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_south
				Return icube_in_icube( 	..
					a.x, 							..
					a.y+a.h, 			..
					a.z, 							..
					a.w-1, 				..
					1,								..
					a.d-1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_west
				Return icube_in_icube( 	..
					a.x-1, 					..
					a.y, 							..
					a.z, 							..
					1, 								..
					a.h-1,					..
					a.d-1,					..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_up
				Return icube_in_icube( 	..
					a.x, 							..
					a.y, 							..
					a.z-1, 					..
					a.w-1, 				..
					a.h-1,					..
					1,								..
					b.x, b.y, b.z, b.w, b.h, b.d )
			Case dir_down
				Return icube_in_icube( 	..
					a.x, 							..
					a.y, 							..
					a.z+a.d, 			..
					a.w-1, 				..
					a.h-1,					..
					1,								..
					b.x, b.y, b.z, b.w, b.h, b.d )
		EndSelect
	Return False
endfunction
	
function icube_tostring:string(r:t_icube)
	return "{" + r.x + "," + r.y + "," + r.z + "/" + r.w + "," + r.y + "," + r.d + "}"
endfunction