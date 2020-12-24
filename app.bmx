
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

function event_update()
	gb_update()
endfunction

function event_draw()
	gb_draw()
endfunction

function event_end()
	gb_end()
endfunction

function event_main()
	event_init()
	event_load()
	event_start()
	while bool_eq(gb.running,true) and appterminate()=false
		cls()
		event_update()
		event_draw()
		flip(1)
	wend
	event_end()
endfunction

''''''''''
'' main ''
''''''''''

event_main()