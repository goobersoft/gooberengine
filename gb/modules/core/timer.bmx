
''''''''''
'' type ''
''''''''''

type t_timer
	field value :t_number
endtype

function new_timer:t_timer(n:float)
	local r:t_timer = new t_timer
	r.value = new_number(n,0,n)
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function timer_set(t:t_timer, n:float)
	number_set_bounds(t.value, 0, n)
	number_to_high(t.value)
endfunction

function timer_get:float(t:t_timer)
	return number_get(t.value)
endfunction

function timer_reset(t:t_timer)
	number_to_high(t.value)
endfunction

function timer_finished:int(t:t_timer)
	return number_eq(t.value,0)
endfunction

function timer_value:float(t:t_timer)
  return t.value.value
endfunction

''''''''''''
'' events ''
''''''''''''

function timer_update(t:t_timer)
	number_add(t.value,-gb_delta())
endfunction

