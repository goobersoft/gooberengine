
Include "timing/fps.bmx"
Include "timing/deltatime.bmx"

''''''''''
'' type ''
''''''''''

Type t_gb_timing
  Field time_curr     :t_long
  Field time_old      :t_long
  Field fps           :t_gb_timing_fps
  Field deltatime     :t_gb_timing_deltatime
  Field stopwatch     :t_stopwatch
  field uptime        :t_float
  field speed         :t_number
EndType

Function new_gb_timing:t_gb_timing ()
  Local r:t_gb_timing = New t_gb_timing
  r.time_curr   = new_long(0)
  r.time_old    = new_long(0)
  r.fps         = new_gb_timing_fps()
  r.deltatime   = new_gb_timing_deltatime()
  r.stopwatch   = new_stopwatch()
  r.uptime      = new_float(0)
  r.speed       = new_number(1,0,10)
  Return r
EndFunction

''''''''''''
'' Events ''
''''''''''''

Function gb_timing_init()
  gb.timing = new_gb_timing()
	Local d:Long = SDLGetTicks()
	long_set( gb.timing.time_curr, d )
	long_set( gb.timing.time_old, d )
	long_set( gb.timing.fps.time, d )
EndFunction

Function gb_timing_update()
	Local d:Long = SDLGetTicks()
	
	long_set( gb.timing.time_old, gb.timing.time_curr )
	long_set( gb.timing.time_curr, d )
	
  long_add( gb.timing.fps.ticks, 1 )

	If (gb.timing.time_curr.value - gb.timing.fps.time.value >= 1000 )
		long_set( gb.timing.fps.value, gb.timing.fps.ticks )
		long_set( gb.timing.fps.ticks, 0 )
		long_set( gb.timing.fps.time, gb.timing.time_curr )
	EndIf
	
	long_set(gb.timing.deltatime.mdiff, long_get(gb.timing.time_curr) - long_get(gb.timing.time_old))
	float_set(gb.timing.deltatime.sdiff, Float(long_get(gb.timing.deltatime.mdiff))/1000)
	
	stopwatch_update(gb.timing.stopwatch)
  float_add(gb.timing.uptime, gb_delta())
EndFunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_uptime:float()
  return gb.timing.uptime.get()
endfunction

Function gb_delta:Float(n:float=1)
  Return gb.timing.deltatime.sdiff.value * n * gb.timing.speed.value
EndFunction

function gb_cpu:float()
  return low((1-(gb.timing.deltatime.sdiff.value / 0.01666667)) * 100,0)
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



