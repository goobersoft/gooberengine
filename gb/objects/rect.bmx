
''''''''''
'' type ''
''''''''''

Type t_rect
	Field x :float
	field y	:float
	field w	:float
	field h :float
endtype

function new_rect:t_rect(x:float, y:float, w:float, h:float)
	local r:t_rect = new t_rect
	r.x = x
	r.y = y
	r.w = low(w,0)
	r.h = low(h,0)
	return r
endfunction

function rect_set(r:t_rect, x:float, y:float, w:float, h:float)
	r.x = x
	r.y = y
	r.w = low(w,0)
	r.h = low(h,0)
endfunction
	
function rect_set_pos(r:t_rect, x:float, y:float)
	r.x = x
	r.y = y
endfunction
	
function rect_add_pos(r:t_rect, x:float, y:float)
	r.x = r.x + x
	r.y = r.y + y
endfunction

function rect_set_size(r:t_rect, w:float, h:float)
	r.w = low(w,0)
	r.h = low(h,0)
endfunction

function rect_colliding :int(a:t_rect, b:t_rect)
	return rect_in_rect( a.x, a.y, a.w, a.h, 
		b.x, b.y, b.w, b.h )
endfunction
	
function rect_slide_check :float(a:t_rect, d:byte, m:float, b:t_rect)
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
	EndSelect
	Return q
endfunction
	
function rect_slide :Float(a:t_rect, d:Byte, m:Float, b:t_rect)
	Local u :Float = rect_slide_check(a,d,m,b)
	Select (d)
		Case dir_north
			a.y = a.y - u
		Case dir_east
			a.x = a.x + u
		Case dir_south
			a.y = a.y + u
		Case dir_west
			a.x = a.x - u
	EndSelect
	Return u
endfunction
	
function rect_touching:int(a:t_rect, d:Byte, b:t_rect)
	Select d
		Case dir_north
			Return rect_in_rect(a.x, a.y-0.1, a.w-0.1, 0.1, b.x, b.y, b.w-0.1, b.h-0.1)
		Case dir_east
			Return rect_in_rect(a.x + a.w, a.y, 0.1, a.h-0.1, b.x, b.y, b.w-0.1, b.h-0.1)
		Case dir_south
			Return rect_in_rect(a.x, a.y + a.h, a.w-0.1, 0.1, b.x, b.y, b.w-0.1, b.h-0.1)
		Case dir_west
			Return rect_in_rect(a.x-0.1, a.y, 0.1, a.h-0.1, b.x, b.y, b.w-0.1, b.h-0.1)
	EndSelect
	Return False
endfunction
	
function rect_tostring:string(r:t_rect)
	return "{" + strdec(r.x) + "," + strdec(r.y) + "/" + strdec(r.w) + "," + strdec(r.h) + "}"
endfunction

''''''''''''''''''
'' integer rect ''
''''''''''''''''''

Type t_irect
	Field x :int
	field y	:int
	field w	:int
	field h :int
endtype

function new_irect:t_irect(x:int, y:int, w:int, h:int)
	local r:t_irect = new t_irect
	r.x = x
	r.y = y
	r.w = low(w,0)
	r.h = low(h,0)
	return r
endfunction

function irect_set(r:t_irect, x:int, y:int, w:int, h:int)
	r.x = x
	r.y = y
	r.w = low(h,0)
	r.h = low(h,0)
endfunction
	
function irect_set_pos(r:t_irect, x:int, y:int)
	r.x = x
	r.y = y
endfunction
	
function irect_add_pos(r:t_irect, x:int, y:int)
	r.x = r.x + x
	r.y = r.y + y
endfunction

function irect_set_size(r:t_irect, w:int, h:int)
	r.w = low(w,0)
	r.h = low(h,0)
endfunction

function irect_colliding:int(a:t_irect, b:t_irect)
	return irect_in_irect( a.x, a.y, a.w, a.h, 
		b.x, b.y, b.w, b.h )
endfunction
	
function irect_slide_check:int(a:t_irect, d:byte, m:int, b:t_irect)
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
	EndSelect
	Return q
endfunction
	
function irect_slide:int(a:t_irect, d:Byte, m:int, b:t_irect)
	Local u :int = irect_slide_check(a,d,m,b)
	Select (d)
		Case dir_north
			a.y = a.y - u
		Case dir_east
			a.x = a.x + u
		Case dir_south
			a.y = a.y + u
		Case dir_west
			a.x = a.x - u
	EndSelect
	Return u
endfunction
	
function irect_touching:int(a:t_irect, d:Byte, b:t_irect)
	Select d
		Case dir_north
			Return irect_in_irect(a.x, a.y-1, a.w-1, 1, b.x, b.y, b.w-1, b.h-1)
		Case dir_east
			Return irect_in_irect(a.x + a.w, a.y, 1, a.h-1, b.x, b.y, b.w-1, b.h-1)
		Case dir_south
			Return irect_in_irect(a.x, a.y + a.h, a.w-1, 1, b.x, b.y, b.w-1, b.h-1)
		Case dir_west
			Return irect_in_irect(a.x-1, a.y, 1, a.h-1, b.x, b.y, b.w-1, b.h-1)
	EndSelect
	Return False
endfunction
	
function irect_tostring:string(r:t_irect)
	return "{" + r.x + "," + r.y + "/" + r.w + "," + r.y + "}"
endfunction