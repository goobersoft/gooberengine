
''''''''''
'' type ''
''''''''''

type t_pulser
  field value :t_number
  field speed :t_float
endtype

function new_pulser:t_pulser(s:float=30.0)
  local r:t_pulser = new t_pulser
  r.value = new_number(0,0,1000,true)
  r.speed = new_float(s)
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function pulser_get:float(p:t_pulser)
	return (sinf( number_get(p.value) )/2) + 0.5
endfunction

function pulser_set_speed ( p:t_pulser, n:float=30.0 )
  float_set(p.speed, n)
endfunction

function pulser_update(p:t_pulser)
	number_add(p.value, float_get(p.speed)*gb_delta()*1000)
endfunction

