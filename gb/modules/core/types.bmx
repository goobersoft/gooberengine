
''''''''''''
'' object ''
''''''''''''
'' meant to be a reference type for :object types.
'' this class ensures an object + a string for its type

type t_datum
  field id        :string
  field value     :object
endtype

function new_datum:t_datum (d:object=null, s:string="null")
  local r:t_datum = new t_datum
  r.value     = d
  r.id        = s
  return r
endfunction

function new_datum_bool:t_datum(v:int)
  local r:object = new_bool(v)
  return new_datum(r,"bool")
endfunction

function new_datum_byte:t_datum(v:byte)
  local r:t_byte = new_byte(v)
  return new_datum(r,"byte")
endfunction

function new_datum_short:t_datum(v:short)
  local r:t_short = new_short(v)
  return new_datum(r,"short")
endfunction

function new_datum_int:t_datum(v:int)
  local r:t_int = new_int(v)
  return new_datum(r,"int")
endfunction

function new_datum_long:t_datum(v:long)
  local r:t_long = new_long(v)
  return new_datum(r,"long")
endfunction

function new_datum_float:t_datum(v:float)
  local r:t_float = new_float(v)
  return new_datum(r,"float")
endfunction

function new_datum_double:t_datum(v:double)
  local r:t_double = new_double(v)
  return new_datum(r,"double")
endfunction

function new_datum_string:t_datum(v:string)
  local r:t_string = new_string(v)
  return new_datum(r,"string")
endfunction

function datum_set_value(o:t_datum, d:object)
  o.value = d
endfunction
function datum_set_id(o:t_datum, s:string)
  o.id = s
endfunction
function datum_set(o:t_datum, d:object, s:string)
  datum_set_value(o,d)
  if s <> null
    datum_set_id(o,s)
  endif
endfunction

function datum_value:object(o:t_datum)
  return o.value
endfunction
global datum_get:object(o:t_datum) = datum_value

function datum_id:string(o:t_datum)
  return o.id
endfunction

function datum_to_string:string(d:t_datum)
  if d
    select d.id
      case "bool"
        local u:t_bool = t_bool(d.value)
        return "bool:" + bool_to_string(u)
      case "byte"
        local u:t_byte = t_byte(d.value)
        return  "byte:" + byte_to_string(u)
      case "short" 
        local u:t_short = t_short(d.value)
        return "short:" + short_to_string(u)
      case "int"
        local u:t_int = t_int(d.value)
        return "int:" + int_to_string(u)
      case "long"
        local u:t_long = t_long(d.value)
        return "long:" + long_to_string(u)
      case "float"
        local u:t_float = t_float(d.value)
        return "float:" + float_to_string(u)
      case "double"
        local u:t_double = t_double(d.value)
        return "double:" + double_to_string(u)
      case "string"
        local u:t_string = t_string(d.value)
        return "string:" + string_to_string(u)
      default
        if d.value
          return "datum: " + datum_to_string(t_datum(d.value))
        else
          return "datum: null"
        endif
    endselect
  endif
  return ""
endfunction

''''''''''
'' bool ''
''''''''''

Type t_bool
	Field value :Byte
endtype

function new_bool:t_bool ( n:int=false )
	local r :t_bool = new t_bool
	r.value = bool(n)
	return r
endfunction

function new_bool_v:t_bool[] ( a:int, n:int=false )
	local r :t_bool[] = new t_bool[a]
	for local i:int = 0 to a-1
		r[i] = new_bool(n)
	next
	return r
endfunction

function bool_get:int ( b:t_bool )
	return b.value
endfunction

function bool_set ( b:t_bool, v:int )
	b.value = bool(v)
endfunction

function bool_set ( b:t_bool, t:t_bool )
  b.value = t.value
endfunction

function bool_toggle ( b:t_bool )
	b.value = not b.value
endfunction

function bool_nt:int ( b:t_bool )
	return not b.value
endfunction

function bool_neq:int ( b:t_bool, v:int )
	return b.value <> v
endfunction

function bool_neq:int ( b:t_bool )
	return (b.value <> true)
endfunction

function bool_eq:int ( b:t_bool, v:int )
	return (b.value = v)
endfunction

function bool_eq:int ( b:t_bool )
	if (b.value) then return true else return false
endfunction

function bool_to_string:string ( b:t_bool )
	if (b.value) then return "true" else return "false"
endfunction

''''''''''
'' byte ''
''''''''''

Type t_byte
	Field value :Byte
endtype

function new_byte:t_byte ( v:int )
	local r:t_byte = new t_byte
	r.value = v
	return r
endfunction

function new_byte_v:t_byte[] ( a:int, v:int )
	local r :t_byte[] = new t_byte[a]
	for local i:int = 0 to a-1
		r[i] = new_byte(v)
	next
	return r
endfunction

function byte_set ( b:t_byte, v:int)
	b.value = v
endfunction

function byte_set ( b:t_byte, t:t_byte )
  b.value = t.value
endfunction

function byte_get:byte ( b:t_byte )
	return b.value
endfunction

function byte_get:byte ( o:object )
	return t_byte(o).value
endfunction

function byte_add ( b:t_byte, v:int )
	b.value = b.value + v
endfunction

function byte_mul ( b:t_byte, v:int )
	b.value = b.value * v
endfunction
	
function byte_div ( b:t_byte, v:int )
	If (v <> 0) Then b.value = b.value / v
endfunction

function byte_pow ( b:t_byte, v:int )
	b.value = b.value ^ v
endfunction

function byte_sqrt ( b:t_byte )
	b.value = sqr(b.value)	
endfunction 
	
function byte_shl ( b:t_byte, v:int )
	b.value = b.value shl v
endfunction

function byte_shr ( b:t_byte, v:int )
	b.value = b.value shr v
endfunction

function byte_nt ( b:t_byte )
	b.value = not b.value
endfunction

function byte_neq:int ( b:t_byte, v:int )
	if (b.value <> v) then return true else return false
endfunction

function byte_eq:int ( b:t_byte, v:int )
	if (b.value = v) then return true else return false
endfunction

function byte_eq:int ( b:t_byte )
	if (b.value) then return true else return false
endfunction
	
function byte_gt:int ( b:t_byte, v:int )
	if (b.value > v) then return true else return false
endfunction

function byte_lt:int ( b:t_byte, v:int )
	if (b.value < v) then return true else return false
endfunction

function byte_gte:int ( b:t_byte, v:int )
	if (b.value >= v) then return true else return false
endfunction

function byte_lte:int ( b:t_byte, v:int )
	if (b.value <= v) then return true else return false
endfunction

function byte_to_string:string ( b:t_byte )
	return string(b.value)
endfunction

'''''''''''
'' short ''
'''''''''''

Type t_short
	Field value :short
endtype

function new_short:t_short ( v:int )
	local r:t_short = new t_short
	r.value = v
	return r
endfunction

function new_short_v:t_short[] ( a:int, v:int )
	local r :t_short[] = new t_short[a]
	for local i:int = 0 to a-1
		r[i] = new_short(v)
	next
	return r
endfunction

function short_set ( b:t_short, v:int)
	b.value = v
endfunction

function short_set ( b:t_short, t:t_short )
  b.value = t.value
endfunction

function short_get:short ( b:t_short )
	return b.value
endfunction

function short_get:short ( o:object )
	return t_short(o).value
endfunction

function short_add ( b:t_short, v:int )
	b.value = b.value + v
endfunction

function short_mul ( b:t_short, v:int )
	b.value = b.value * v
endfunction
	
function short_div ( b:t_short, v:int )
	If (v <> 0) Then b.value = b.value / v
endfunction

function short_pow ( b:t_short, v:int )
	b.value = b.value ^ v
endfunction

function short_sqrt ( b:t_short, v:int )
	b.value = sqr(b.value)	
endfunction 
	
function short_shl ( b:t_short, v:int )
	b.value = b.value shl v
endfunction

function short_shr ( b:t_short, v:int )
	b.value = b.value shr v
endfunction

function short_nt ( b:t_short )
	b.value = not b.value
endfunction

function short_neq:int ( b:t_short, v:int )
	if (b.value <> v) then return true else return false
endfunction

function short_eq:int ( b:t_short, v:int )
	if (b.value = v) then return true else return false
endfunction

function short_eq:int ( b:t_short )
	if (b.value) then return true else return false
endfunction
	
function short_gt:int ( b:t_short, v:int )
	if (b.value > v) then return true else return false
endfunction

function short_lt:int ( b:t_short, v:int )
	if (b.value < v) then return true else return false
endfunction

function short_gte:int ( b:t_short, v:int )
	if (b.value >= v) then return true else return false
endfunction

function short_lte:int ( b:t_short, v:int )
	if (b.value <= v) then return true else return false
endfunction

function short_to_string:string ( b:t_short )
	return string(b.value)
endfunction

'''''''''
'' int ''
'''''''''

Type t_int
	Field value :int
endtype

function new_int:t_int ( v:int )
	local r:t_int = new t_int
	r.value = v
	return r
endfunction

function new_int_v:t_int[] ( a:int, v:int )
	local r :t_int[] = new t_int[a]
	for local i:int = 0 to a-1
		r[i] = new_int(v)
	next
	return r
endfunction

function int_set ( b:t_int, v:int )
	b.value = v
endfunction

function int_set ( b:t_int, t:t_int )
  b.value = t.value
endfunction

function int_get:int ( b:t_int )
	return b.value
endfunction

function int_get:int ( o:object )
	return t_int(o).value
endfunction

function int_add ( b:t_int, v:int )
	b.value = b.value + v
endfunction

function int_mul ( b:t_int, v:int )
	b.value = b.value * v
endfunction
	
function int_div ( b:t_int, v:int )
	If (v <> 0) Then b.value = b.value / v
endfunction

function int_pow ( b:t_int, v:int )
	b.value = b.value ^ v
endfunction

function int_sqrt ( b:t_int, v:int )
	b.value = sqr(b.value)	
endfunction 
	
function int_shl ( b:t_int, v:int )
	b.value = b.value shl v
endfunction

function int_shr ( b:t_int, v:int )
	b.value = b.value shr v
endfunction

function int_nt ( b:t_int )
	b.value = not b.value
endfunction

function int_neq:int ( b:t_int, v:int )
	if (b.value <> v) then return true else return false
endfunction

function int_eq:int ( b:t_int, v:int )
	if (b.value = v) then return true else return false
endfunction

function int_eq:int ( b:t_int )
	if (b.value) then return true else return false
endfunction
	
function int_gt:int ( b:t_int, v:int )
	if (b.value > v) then return true else return false
endfunction

function int_lt:int ( b:t_int, v:int )
	if (b.value < v) then return true else return false
endfunction

function int_gte:int ( b:t_int, v:int )
	if (b.value >= v) then return true else return false
endfunction

function int_lte:int ( b:t_int, v:int )
	if (b.value <= v) then return true else return false
endfunction

function int_to_string:string ( b:t_int )
	return string(b.value)
endfunction

''''''''''
'' long ''
''''''''''

Type t_long
	Field value :long
endtype

function new_long:t_long ( v:long )
	local r:t_long = new t_long
	r.value = v
	return r
endfunction

function long_set ( b:t_long, v:long )
	b.value = v
endfunction

function long_set ( b:t_long, t:t_long )
  b.value = t.value
endfunction

function long_get:long ( b:t_long )
	return b.value
endfunction

function long_get:long ( o:object )
	return t_long(o).value
endfunction

function long_add ( b:t_long, v:long )
	b.value = b.value + v
endfunction

function long_mul ( b:t_long, v:long )
	b.value = b.value * v
endfunction
	
function long_div ( b:t_long, v:long )
	If (v <> 0) Then b.value = b.value / v
endfunction

function long_pow ( b:t_long, v:long )
	b.value = b.value ^ v
endfunction

function long_sqrt ( b:t_long, v:long )
	b.value = sqr(b.value)	
endfunction 
	
function long_shl ( b:t_long, v:int )
	b.value = b.value shl v
endfunction

function long_shr ( b:t_long, v:int )
	b.value = b.value shr v
endfunction

function long_nt ( b:t_long )
	b.value = not b.value
endfunction

function long_neq:int ( b:t_long, v:long )
	if (b.value <> v) then return true else return false
endfunction

function long_eq:int ( b:t_long, v:long )
	if (b.value = v) then return true else return false
endfunction

function long_eq:int ( b:t_long )
	if (b.value) then return true else return false
endfunction
	
function long_gt:int ( b:t_long, v:long )
	if (b.value > v) then return true else return false
endfunction

function long_lt:int ( b:t_long, v:long )
	if (b.value < v) then return true else return false
endfunction

function long_gte:int ( b:t_long, v:long )
	if (b.value >= v) then return true else return false
endfunction

function long_lte:int ( b:t_long, v:long )
	if (b.value <= v) then return true else return false
endfunction

function long_to_string:string ( b:t_long )
	return string(b.value)
endfunction

'''''''''''
'' float ''
'''''''''''

Type t_float
	Field value :float
endtype

function new_float:t_float ( v:float )
	local r:t_float = new t_float
	r.value = v
	return r
endfunction

function float_set ( b:t_float, v:float)
	b.value = v
endfunction

function float_set ( b:t_float, t:t_float )
  b.value = t.value
endfunction

function float_get:float ( b:t_float )
	return b.value
endfunction

function float_get:float ( o:object )
	return t_float(o).value
endfunction

function float_add ( b:t_float, v:float )
	b.value = b.value + v
endfunction

function float_mul ( b:t_float, v:float )
	b.value = b.value * v
endfunction
	
function float_div ( b:t_float, v:float )
	If (v <> 0) Then b.value = b.value / v
endfunction

function float_pow ( b:t_float, v:float )
	b.value = b.value ^ v
endfunction

function float_sqrt ( b:t_float, v:float )
	b.value = sqr(b.value)	
endfunction 

function float_nt ( b:t_float )
	b.value = not b.value
endfunction

function float_neq:int ( b:t_float, v:float )
	if (b.value <> v) then return true else return false
endfunction

function float_eq:int ( b:t_float, v:float )
	if (b.value = v) then return true else return false
endfunction

function float_eq:int ( b:t_float )
	if (b.value) then return true else return false
endfunction
	
function float_gt:int ( b:t_float, v:float )
	if (b.value > v) then return true else return false
endfunction

function float_lt:int ( b:t_float, v:float )
	if (b.value < v) then return true else return false
endfunction

function float_gte:int ( b:t_float, v:float )
	if (b.value >= v) then return true else return false
endfunction

function float_lte:int ( b:t_float, v:float )
	if (b.value <= v) then return true else return false
endfunction

function float_to_string:string ( b:t_float )
	return strdec(b.value,3)
endfunction

''''''''''''
'' double ''
''''''''''''

Type t_double
	Field value :double
endtype

function new_double:t_double ( v:double )
	local r:t_double = new t_double
	r.value = v
	return r
endfunction

function double_set ( b:t_double, v:double )
	b.value = v
endfunction

function double_set ( b:t_double, t:t_double )
  b.value = t.value
endfunction

function double_get:double ( b:t_double )
	return b.value
endfunction

function double_get:double ( o:object )
	return t_double(o).value
endfunction

function double_add ( b:t_double, v:double )
	b.value = b.value + v
endfunction

function double_mul ( b:t_double, v:double )
	b.value = b.value * v
endfunction
	
function double_div ( b:t_double, v:double )
	If (v <> 0) Then b.value = b.value / v
endfunction

function double_pow ( b:t_double, v:double )
	b.value = b.value ^ v
endfunction

function double_sqrt ( b:t_double, v:double )
	b.value = sqr(b.value)	
endfunction 

function double_nt ( b:t_double )
	b.value = not b.value
endfunction

function double_neq:int ( b:t_double, v:double )
	if (b.value <> v) then return true else return false
endfunction

function double_eq:int ( b:t_double, v:double )
	if (b.value = v) then return true else return false
endfunction

function double_eq:int ( b:t_double )
	if (b.value) then return true else return false
endfunction
	
function double_gt:int ( b:t_double, v:double )
	if (b.value > v) then return true else return false
endfunction

function double_lt:int ( b:t_double, v:double )
	if (b.value < v) then return true else return false
endfunction

function double_gte:int ( b:t_double, v:double )
	if (b.value >= v) then return true else return false
endfunction

function double_lte:int ( b:t_double, v:double )
	if (b.value <= v) then return true else return false
endfunction

function double_to_string:string ( b:t_double )
	return strdbl(b.value,6)
endfunction

''''''''''''
'' String ''
''''''''''''

type t_string
	field value :string
endtype

function new_string:t_string ( v:string )
	local r:t_string = new t_string
	r.value = v
	return r
endfunction

function new_string_v:t_string[]  ( n:int, v:string="" )
  local r:t_string[] = new t_string[n]
  for local i:int = 0 to n-1
    r[i] = new_string(v)
  next
  return r
endfunction

function string_get:string ( s:t_string )
	return s.value
endfunction

function string_get:string ( o:object )
	return t_string(o).value
endfunction

function string_length:int ( s:t_string )
	return len(s.value)
endfunction

function string_set ( s:t_string, v:string )
	s.value = v
endfunction

function string_set ( b:t_string, t:t_string )
  b.value = t.value
endfunction

function string_add ( s:t_string, v:string )
	s.value = s.value + v
endfunction

function string_neq:int( s:t_string, v:string )
	if (s.value <> v) then return true else return false
endfunction

function string_eq:int( s:t_string, v:string )
	if (s.value = v) then return true else return false
endfunction

function string_eq:int( b:t_string )
	if (b.value) then return true else return false
endfunction

function string_del:string ( b:t_string, n:int=-1 )
	local c:string = ""
	if len(b.value) > 0
		n = iwrap(n,0,b.value.length)
		c = b.value[n]
		b.value = b.value[0..n] + b.value[n+1..b.value.length]
	endif
	return c
endfunction

function string_to_string:string(s:t_string)
  return s.value
endfunction