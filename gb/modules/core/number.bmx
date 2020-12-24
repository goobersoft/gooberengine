
''''''''''''''''''
'' Float Number ''
''''''''''''''''''

Type t_number
	Field value 			:Float
	Field low					:Float
	Field high				:Float
	Field wrappable		:Byte
endtype

function new_number:t_number(n:float=0, l:float=0, h:float=100, w:byte=false)
	local r:t_number = new t_number
	Local u :Float = 0
	If (l > h)
		u = l
		l = h
		h = u
	EndIf
	r.low = l
	r.high = h
	r.value = clamp(n,l,h)
	r.wrappable = bool(w)
	return r
endfunction

''	I seriously make numbers like this a lot, so I made a function
''	to just make it easier for me.
function new_number_angle:t_number(n:float)
	return new_number(n,0,1000,true)
endfunction

function number_set_bounds(n:t_number, l:float, h:float)
	Local u :Float = 0
	If (l > h)
		u = l
		l = h
		h = u
	EndIf
	n.low = l
	n.high = h
	n.value = clamp(n.value,n.low,n.high)
endfunction

function number_get:Float(n:t_number)
	Return n.value
endfunction

function number_set(n:t_number, v:Float)
	If (n.wrappable)
		n.value = wrap(v,n.low,n.high)	
	Else
		n.value = clamp(v,n.low,n.high)
	EndIf
endfunction
	
function number_add(n:t_number, v:float)
	If (n.wrappable)
		n.value = wrap(n.value+v,n.low,n.high)	
	Else
		n.value = clamp(n.value+v,n.low,n.high)
	EndIf
endfunction
	
function number_addr:float(n:t_number, v:float)
	local r:float = 0.0
	
	n.value = n.value + v
	if (n.value > n.high)
		r = n.value - n.high
	elseif (n.value < n.low)
		r = n.value + n.low
	endif
	
	if (n.wrappable)
		n.value = wrap(n.value+v,n.low,n.high)
	else
		n.value = clamp(n.value+v,n.low,n.high)
	endif
	
	return r
endfunction
	
function number_mul(n:t_number, v:float)
	If (n.wrappable)
		n.value = wrap(n.value*v,n.low,n.high)	
	Else
		n.value = clamp(n.value*v,n.low,n.high)
	EndIf
endfunction

function number_div(n:t_number, v:float)
	if (v <> 0)
		If (n.wrappable)
			n.value = wrap(n.value/v,n.low,n.high)	
		Else
			n.value = clamp(n.value/v,n.low,n.high)
		EndIf
	endif
endfunction

function number_tostring:string(n:t_number)
	return "{" + n.low + "<=" + n.value + "<=" + n.high + "}"
endfunction

function number_to_low(n:t_number)
	n.value = n.low
endfunction

function number_to_high(n:t_number)
	n.value = n.high
endfunction

function number_eq:int(n:t_number, v:float)
	return (n.value = v)
endfunction

function number_neq:int(n:t_number, v:float)
	return (n.value <> v)
endfunction

function number_gt:int(n:t_number,v:float)
	return (n.value > v)
endfunction

function number_gte:int(n:t_number,v:float)
	return (n.value >= v)
endfunction

function number_lt:int(n:t_number,v:float)
	return (n.value < v)
endfunction

function number_lte:int(n:t_number,v:float)
	return (n.value <= v)
endfunction

''''''''''''''''''''
'' Integer Number ''
''''''''''''''''''''

Type t_inumber
	Field value 			:int
	Field low					:int
	Field high				:int
	Field wrappable		:Byte
endtype

function new_inumber:t_inumber(n:int=0, l:int=0, h:int=100, w:byte=false)
	local r:t_inumber = new t_inumber
	Local u :int = 0
	If (l > h)
		u = l
		l = h
		h = u
	EndIf
	r.low = l
	r.high = h
	r.value = clamp(n,l,h)
	r.wrappable = bool(w)
	return r
endfunction

function inumber_setbounds(n:t_inumber, l:int, h:int)
	Local u :int = 0
	If (l > h)
		u = l
		l = h
		h = u
	EndIf
	n.low = l
	n.high = h
	n.value = clamp(n.value,n.low,n.high)
endfunction

function inumber_get:int(n:t_inumber)
	Return n.value
endfunction

function inumber_set(n:t_inumber, v:int)
	If (n.wrappable)
		n.value = wrap(v,n.low,n.high)	
	Else
		n.value = clamp(v,n.low,n.high)
	EndIf
endfunction
	
function inumber_add(n:t_inumber, v:int)
	If (n.wrappable)
		n.value = wrap(n.value+v,n.low,n.high)	
	Else
		n.value = clamp(n.value+v,n.low,n.high)
	EndIf
endfunction
	
function inumber_addr:int(n:t_inumber, v:int)
	local r:int = 0.0
	
	n.value = n.value + v
	if (n.value > n.high)
		r = n.value - n.high
	elseif (n.value < n.low)
		r = n.value + n.low
	endif
	
	if (n.wrappable)
		n.value = wrap(n.value+v,n.low,n.high)
	else
		n.value = clamp(n.value+v,n.low,n.high)
	endif
	
	return r
endfunction
	
function inumber_mul(n:t_inumber, v:int)
	If (n.wrappable)
		n.value = wrap(n.value*v,n.low,n.high)	
	Else
		n.value = clamp(n.value*v,n.low,n.high)
	EndIf
endfunction

function inumber_div(n:t_inumber, v:int)
	if (v <> 0)
		If (n.wrappable)
			n.value = wrap(n.value/v,n.low,n.high)	
		Else
			n.value = clamp(n.value/v,n.low,n.high)
		EndIf
	endif
endfunction

function inumber_tostring:string(n:t_inumber)
	return "{" + n.low + "<=" + n.value + "<=" + n.high + "}"
endfunction

function inumber_to_low(n:t_inumber)
	n.value = n.low
endfunction

function inumber_to_high(n:t_inumber)
	n.value = n.high
endfunction

function inumber_eq:int(n:t_inumber, v:int)
	return (n.value = v)
endfunction

function inumber_neq:int(n:t_inumber, v:int)
	return (n.value <> v)
endfunction

function inumber_gt:int(n:t_inumber,v:int)
	return (n.value > v)
endfunction

function inumber_gte:int(n:t_inumber,v:int)
	return (n.value >= v)
endfunction

function inumber_lt:int(n:t_inumber,v:int)
	return (n.value < v)
endfunction

function inumber_lte:int(n:t_inumber,v:int)
	return (n.value <= v)
endfunction