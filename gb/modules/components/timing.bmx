
''''''''''''
'' global ''
''''''''''''

global gb_timing_time_curr          :t_long
global gb_timing_time_old           :t_long
global gb_timing_stopwatch          :t_stopwatch
global gb_timing_uptime             :t_float
global gb_timing_speed              :t_number

global gb_timing_deltatime_mdiff    :t_long
global gb_timing_deltatime_sdiff    :t_float

global gb_timing_fps_ticks          :t_long
global gb_timing_fps_value          :t_long
global gb_timing_fps_time           :t_long

''''''''''''
'' Events ''
''''''''''''

Function gb_timing_init()
  gb_timing_time_curr     = new_long(0)
  gb_timing_time_old      = new_long(0)
  gb_timing_stopwatch     = new_stopwatch()
  gb_timing_uptime        = new_float(0)
  gb_timing_speed         = new_number(1,0,10)

  gb_timing_deltatime_mdiff = new_long(0)
  gb_timing_deltatime_sdiff = new_float(0)

  gb_timing_fps_ticks = new_long(0)
  gb_timing_fps_value = new_long(0)
  gb_timing_fps_time = new_long(0)

  
	Local d:Long = SDLGetTicks()
	long_set( gb_timing_time_curr, d )
	long_set( gb_timing_time_old, d )
	long_set( gb_timing_fps_time, d )
EndFunction

Function gb_timing_update()
	Local d:Long = SDLGetTicks()
	
	long_set( gb_timing_time_old, gb_timing_time_curr )
	long_set( gb_timing_time_curr, d )
	
  long_add( gb_timing_fps_ticks, 1 )

	If (gb_timing_time_curr.value - gb_timing_fps_time.value >= 1000 )
		long_set( gb_timing_fps_value, gb_timing_fps_ticks )
		long_set( gb_timing_fps_ticks, 0 )
		long_set( gb_timing_fps_time, gb_timing_time_curr )
	EndIf
	
	long_set(gb_timing_deltatime_mdiff, long_get(gb_timing_time_curr) - long_get(gb_timing_time_old))
	float_set(gb_timing_deltatime_sdiff, Float(long_get(gb_timing_deltatime_mdiff))/1000)
	
	stopwatch_update(gb_timing_stopwatch)
  float_add(gb_timing_uptime, gb_delta())
EndFunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_uptime:float()
  return gb_timing_uptime.get()
endfunction

Function gb_delta:Float(n:float=1)
  Return gb_timing_deltatime_sdiff.value * n * gb_timing_speed.value
EndFunction

function gb_cpu:float()
  return low((1-(gb_timing_deltatime_sdiff.value / 0.01666667)) * 100,0)
endfunction
function gb_icpu:int()
  return int(gb_cpu())
endfunction

function gb_cpu_string:string(t:int)
  if t
    local u:int = iclamp(gb_icpu(),0,100)/4
    return "[" + rep("*",u) + rep(" ",20-u) + "]"
  else
    return strdec(gb_cpu(),0) + "%"
  endif
endfunction



