
'''''''''''''
'' globals ''
'''''''''''''

global gb_timing_curr								:uint
global gb_timing_old								:uint

global gb_timing_fps_ticks 					:int
global gb_timing_fps_value 					:int
global gb_timing_fps_old						:uint

global gb_timing_deltatime_mdiff 		:int
global gb_timing_deltatime_sdiff		:float

global gb_timing_stopwatch	:t_stopwatch

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_get_deltatime:float()
	return gb_timing_deltatime_sdiff
endfunction

function gb_get_virtual_deltatime:float()
	return gb_timing_deltatime_sdiff * gb_speed.value
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_timing_init()
	local d:long = sdlgetticks()

	gb_timing_curr 					= d
	gb_timing_old						= d
	
	gb_timing_fps_old 			= d
	gb_timing_fps_ticks			= 0
	gb_timing_fps_value			= 0
	
	gb_timing_deltatime_mdiff = 0
	gb_timing_deltatime_sdiff = 0.0

	gb_timing_stopwatch				= new_stopwatch()
endfunction

function gb_timing_update()
	local d:long = sdlgetticks()
	
	gb_timing_old 		= gb_timing_curr
	gb_timing_curr 		= d
	
	gb_timing_fps_ticks = gb_timing_fps_ticks + 1
	if (gb_timing_curr - gb_timing_fps_old >= 1000)
		gb_timing_fps_value = gb_timing_fps_ticks
		gb_timing_fps_ticks = 0
		gb_timing_fps_old   = gb_timing_curr
	endif
	
	gb_timing_deltatime_mdiff = gb_timing_curr - gb_timing_old
	gb_timing_deltatime_sdiff = float(gb_timing_deltatime_mdiff)/1000
	
	stopwatch_update(gb_timing_stopwatch,gb_timing_deltatime_sdiff)
endfunction