
'''''''''''''
'' globals ''
'''''''''''''

global gb_scene_uptime 			:float
global gb_scene_init_fn			()
global gb_scene_update_fn		(d:float)
global gb_scene_draw_fn			(x:float,y:float)
global gb_scene_end_fn			()

function gb_scene_init()
	gb_scene_uptime = 0
endfunction

function gb_scene_update(d:float)
	if gb_scene_update_fn
		gb_scene_update_fn(d)
	endif
	gb_scene_uptime = gb_scene_uptime + d
endfunction

function gb_scene_draw(x:float, y:float)
	if gb_scene_draw_fn
		gb_scene_draw_fn(x,y)
	endif
endfunction

function gb_scene_end()
endfunction

''''''''''''''

function gb_scene_switch_out()
	if gb_scene_end_fn
		gb_scene_end_fn()
	endif
	gb_scene_uptime = 0
endfunction

function gb_scene_switch_in()
	if gb_scene_init_fn
		gb_scene_init_fn()
	endif
endfunction