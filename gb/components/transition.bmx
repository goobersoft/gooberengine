

const gb_transition_id_squares 	:int = 1
const gb_transition_tiles_x			:int = 40
const gb_transition_tiles_y			:int = 24


'''''''''''''
'' globals ''
'''''''''''''

global gb_transition_playing 	:t_bool
global gb_transition_sprites	:t_sprite[,]
global gb_transition_style		:t_int
global gb_transition_fade			:t_bool

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_transition_reset()
	'int_set(gb_transition_id, 0)
	bool_set(gb_transition_playing, false)
endfunction

function gb_transition_set_style(n:int)
endfunction

function gb_transition_set_fade(n:int)
endfunction

function gb_transition_finished:int()
	return bool_eq(gb_transition_playing,false)
endfunction

function gb_transition_fade_out()
endfunction

function gb_transition_fade_in()
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_transition_init()
	gb_transition_playing 	= new_bool(false)
	gb_transition_sprites		= new t_sprite[gb_transition_tiles_x,gb_transition_tiles_y]
	gb_transition_style			= new_int(0)
	gb_transition_fade			= new_bool(true)
endfunction

