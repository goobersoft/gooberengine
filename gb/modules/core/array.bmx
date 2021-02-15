

type t_array
  field length :t_int
  field data   :t_datum[]
  method get:t_datum[](); return data; endmethod
endtype

function new_array(l:int)
  l = ilow(l,0)
  local r:t_array = new t_array
  r.length = new_int(l)
  r.data = new t_datum[l]
  if l > 0
    for local i:int = 0 to r.length.value-1
      r.data[i] = new_datum()
    next
  endif
endfunction

function new_array_v:t_array[](x:int, y:int)
  x = ilow(x,0)
  y = ilow(y,0)
  local r:t_array[] = new t_array[x]
  for local i:int = 0 to x-1
    r[i] = new t_array
    r[i].length = new_int(y)
    for local k:int = 0 to y-1
      r[i].data[k] = new_datum()
    next
  next
  return r
endfunction

'' setters

function array_set(p:t_array, n:int, d:object, t:string)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], t)
  datum_set_value(p.data[n], d)
endfunction

function array_set_bool(p:t_array, n:int, b:t_bool)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "bool")
  datum_set_value(p.data[n], b)
endfunction

function array_set_byte(p:t_array, n:int, b:t_byte)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "byte")
  datum_set_value(p.data[n], b)
endfunction

function array_set_short(p:t_array, n:int, b:t_short)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "short")
  datum_set_value(p.data[n], b)
endfunction

function array_set_int(p:t_array, n:int, b:t_int)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "int")
  datum_set_value(p.data[n], b)
endfunction

function array_set_long(p:t_array, n:int, b:t_long)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "long")
  datum_set_value(p.data[n], b)
endfunction

function array_set_float(p:t_array, n:int, b:t_float)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "float")
  datum_set_value(p.data[n], b)
endfunction

function array_set_double(p:t_array, n:int, b:t_double)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "double")
  datum_set_value(p.data[n], b)
endfunction

function array_set_string(p:t_array, n:int, b:t_string)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "string")
  datum_set_value(p.data[n], b)
endfunction

function array_set_datum(p:t_array, n:int, b:t_datum)
  n = iclamp(n,0,p.length.value-1)
  datum_set_id   (p.data[n], "datum")
  datum_set_value(p.data[n], b)
endfunction

'' getters

function array_get_id:string ( p:t_array, n:int )
  n = iclamp(n,0,p.length.value-1)
  return datum_id(p.data[n])
endfunction

function array_get_data:object (p:t_array, n:int )
  n = iclamp(n,0,p.length.value-1)
  return datum_value(p.data[n])
endfunction