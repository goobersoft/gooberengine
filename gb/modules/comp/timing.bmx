
include "timing/fps.bmx"
include "timing/deltatime.bmx"

''''''''''
'' type ''
''''''''''

type t_gb_timing
  field time_curr     :t_long
  field time_old      :t_long
  field fps           :t_gb_timing_fps
  field deltatime     :t_gb_timing_deltatime
  field stopwatch     :t_stopwatch
endtype

function new_gb_timing:t_gb_timing ()
  local r:t_gb_timing
  r.time_curr   = new_long(0)
  r.time_old    = new_long(0)
  r.fps         = new_gb_timing_fps()
  r.deltatime   = new_gb_timing_deltatime()
  r.stopwatch   = new_stopwatch()
  return r
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_timing_init()
  gb.timing = new_gb_timing()

	local d:long = sdlgetticks()
	long_set( gb.timing.time_curr, d )
	long_set( gb.timing.time_old, d )
	long_set( gb.timing.fps.time, d )
endfunction

function gb_timing_update()
	local d:long = sdlgetticks()
	
	long_set( gb.timing.time_old, gb.timing.time_curr )
	long_set( gb.timing.time_curr, d )
	
  int_add( gb.timing.fps.ticks, 1 )

	if (gb.timing.time_curr.value - gb.timing.fps.time.value >= 1000 )
		int_set( gb.timing.fps.value, gb.timing.fps.ticks )
		int_set( gb.timing.fps.ticks, 0 )
		long_set( gb.timing.fps.time, gb.timing.time_curr )
	endif
	
	int_set(gb.timing.deltatime.mdiff, int(long_get(gb.timing.time_curr) - long_get(gb.timing.time_old)))
	float_set(gb.timing.deltatime.sdiff, float(long_get(gb.timing.deltatime.mdiff))/1000)
	
	stopwatch_update(gb.timing.stopwatch)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_get_deltatime:float()
	return gb.timing.deltatime.sdiff.value
endfunction
function gb_delta:float()
  return gb.timing.deltatime.sdiff.value
endfunction

function gb_get_virtual_deltatime:float()
	return gb.timing.deltatime.sdiff.value * gb.speed.value
endfunction

