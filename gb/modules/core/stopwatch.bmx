
type t_stopwatch
	Field days			:Int
	Field hours			:Int
	Field minutes		:Int
	Field seconds		:Int
	Field millis		:Int
endtype

function new_stopwatch:t_stopwatch()
	local r:t_stopwatch = new t_stopwatch
	r.days 		= 0
	r.hours 	= 0
	r.minutes = 0
	r.seconds = 0
	r.millis 	= 0
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function stopwatch_calculate(s:t_stopwatch)
	While s.millis >= 1000
		s.seconds = s.seconds + 1
		s.millis = s.millis - 1000
	EndWhile
	While s.seconds >= 60
		s.seconds = s.seconds - 60
		s.minutes = s.minutes + 1
	EndWhile
	While s.minutes >= 60
		s.minutes = s.minutes - 60
		s.hours = s.hours + 1
	EndWhile
	While s.hours >= 24
		s.hours = s.hours - 24
		s.days = s.days + 1
	EndWhile
endfunction

function stopwatch_add_time(s:t_stopwatch, m:int)
	s.millis = s.millis + m
	stopwatch_calculate(s)
endfunction

function stopwatch_reset(s:t_stopwatch)
	s.days 		= 0
	s.hours 	= 0
	s.minutes = 0
	s.seconds = 0
	s.millis 	= 0
endfunction

function stopwatch_tostring:string(s:t_stopwatch)
	Return s.days + ":" + rpad(s.hours,2,"0") + ":" + rpad(s.minutes,2,"0") + ":" + rpad(s.seconds,2,"0")
endfunction

''''''''''''
'' Events ''
''''''''''''

function stopwatch_update(s:t_stopwatch)
	stopwatch_add_time(s,int(gb_delta()*1000))
endfunction