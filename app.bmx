
SuperStrict
Import sdl.sdl

''''''''''''''
'' Includes ''
''''''''''''''

include "gb/gb.bmx"

''''''''''''
'' Events ''
''''''''''''

function event_init()
	gb_init()
endfunction

function event_load()
	gb_load()
endfunction

function event_start()
	gb_start()
endfunction

function event_update(d:float)
	gb_update(d)
endfunction

function event_draw(x:float=0, y:float=0)
	gb_draw(x,y)
endfunction

function event_end()
	gb_end()
endfunction

function event_main()
	event_init()
	event_load()
	event_start()
	while bool_eq(gb_running,true) and appterminate()=false
		cls()
		event_update(0)
		event_draw(0,0)
		flip(1)
	wend
	event_end()
endfunction

''''''''''
'' main ''
''''''''''

event_main()