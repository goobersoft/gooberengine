
'''''''''''''
'' globals ''
'''''''''''''

global gb_pulser_value :t_number
global gb_pulser_speed :t_float

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_pulser_get_value:float()
	return (sinf( number_get(gb_pulser_value) )/2) + 0.5
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_pulser_init()
	gb_pulser_value = new_number(0,0,1000,true)
	gb_pulser_speed = new_float(30)
endfunction

function gb_pulser_update(d:float)
	number_add(gb_pulser_value, float_get(gb_pulser_speed)*d*1000)
endfunction