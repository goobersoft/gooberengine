
type t_counter
	field init  :int
	field value :int
  field vstep :int
endtype

function new_counter:t_counter(n:int=-1)
	local r:t_counter = new t_counter
	r.value   = n
	r.init    = n
  r.vstep   = 1
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function counter_get:int(c:t_counter)
	return c.value
endfunction

function counter_reset:int(c:t_counter)
	c.value = c.init
	return c.value
endfunction

function counter_set:int(c:t_counter, n:int=0)
	c.value = n
	return c.value
endfunction

function counter_inc:int(c:t_counter, n:int=1)
	c.value = c.value + n
	return c.value
endfunction

function counter_dec:int(c:t_counter, n:int=1)
  c.value = c.value - n
  return c.value
endfunction

function counter_step:int(c:t_counter)
  c.value = c.value + c.vstep
  return c.value
endfunction