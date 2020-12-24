
type t_params
  field length :t_int
  field data   :t_object[]
endtype

function new_params(l:int)
  l = ilow(l,0)
  local r:t_params = new t_params
  r.length = new_int(l)
  r.data = new t_object[l]
  if l > 0
    for local i:int = 0 to r.length.value-1
      r.data[i] = new_object()
    next
  endif
endfunction

'' setters

function params_set(p:t_params, n:int, d:object, t:string)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], t)
  object_set_data(p.data[n], d)
endfunction

function params_set_bool(p:t_params, n:int, b:t_bool)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "bool")
  object_set_data(p.data[n], b)
endfunction

function params_set_byte(p:t_params, n:int, b:t_byte)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "byte")
  object_set_data(p.data[n], b)
endfunction

function params_set_short(p:t_params, n:int, b:t_short)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "short")
  object_set_data(p.data[n], b)
endfunction

function params_set_int(p:t_params, n:int, b:t_int)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "int")
  object_set_data(p.data[n], b)
endfunction

function params_set_long(p:t_params, n:int, b:t_long)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "long")
  object_set_data(p.data[n], b)
endfunction

function params_set_float(p:t_params, n:int, b:t_float)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "float")
  object_set_data(p.data[n], b)
endfunction

function params_set_double(p:t_params, n:int, b:t_double)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "double")
  object_set_data(p.data[n], b)
endfunction

function params_set_string(p:t_params, n:int, b:t_string)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "string")
  object_set_data(p.data[n], b)
endfunction

function params_set_object(p:t_params, n:int, b:t_object)
  n = iclamp(n,0,p.length.value-1)
  object_set_type(p.data[n], "object")
  object_set_data(p.data[n], b)
endfunction

'' getters

function params_get_type:string ( p:t_params, n:int )
  n = iclamp(n,0,p.length.value-1)
  return object_type(p.data[n])
endfunction

function params_get_data:object (p:t_params, n:int )
  n = iclamp(n,0,p.length.value-1)
  return object_data(p.data[n])
endfunction