
const gb_debug_max_logs :int = 1000

'''''''''''''
'' globals ''
'''''''''''''

global gb_debug_logpos 	:t_int
global gb_debug_log			:string[]
global gb_debug_panel		:t_bool

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_debug_out( s:string )
	int_add(gb_debug_logpos,1)
	if int_eq(gb_debug_logpos,gb_debug_log.length)
		gb_debug_log = gb_debug_log[..gb_debug_log.length+1000]
	endif
	gb_debug_log[int_get(gb_debug_logpos)] = s
  SDL_Log(s)
endfunction

function gb_debug_draw_panel()
	gb_graph_enable_camera(false)
	gb_graph_draw_tile_rect(0,0,0,27,23,40,1)
	gb_graph_draw_tile_text(0,0,1,"M:"+gcmemalloced())
	gb_graph_draw_tile_text(6,0,1,int(gb_mouse_virtual_tile_x())+","+int(gb_mouse_virtual_tile_y()))
	gb_graph_draw_tile_text(9,0,1,"P:"+gb_graph_pixelmode.value)
	gb_graph_draw_tile_text(11,0,1,"W:"+int(gb_visual_windowscale.value))
	gb_graph_draw_tile_text(13,0,1,"F:"+gb_timing_fps_value)
	gb_graph_draw_tile_text(16,0,1,stopwatch_tostring(gb_timing_stopwatch))
	local u:float = 0
	for local i:float = 0 to 11
		u = gb_controller_buttons[i]
		gb_graph_draw_tile_box(22+(0.2*i), 0, 0, 28+(0.2*u), 21, 0.2, 1)
	next
	for local i:float = 0 to 10
		if i = 0
			gb_graph_draw_tile(25+i,0,0,21,2)
		elseif i = 1
			gb_graph_draw_tile(25+i,0,0,22,3)
		elseif i = 10
			gb_graph_draw_tile(25+i,0,0,23,2)
		else
			gb_graph_draw_tile(25+i,0,0,22,2)
		endif
	next
	gb_graph_draw_tile(25+gb_speed.value,0,0,23,3)
	gb_graph_draw_tile_text(36,0,1,strdec(gb_speed.value))
	gb_graph_draw_tile(39,0,0,28,2)
	gb_graph_enable_camera(true)
endfunction

function gb_debug_draw_grid_bg()
endfunction

function gb_debug_draw_grid_fg()
	gb_graph_enable_camera(false)
	gb_graph_set_mode(gb_graph_mode_multiply)
	for local i:float = 0 to 40
		for local j:float = 0 to 24
			if (i = 19) and (j = 12)
				gb_graph_draw_tile(i,j,0,26,3)
			elseif (i = 19)
				gb_graph_draw_tile(i,j,0,27,3)
			elseif (j = 12)
				gb_graph_draw_tile(i,j,0,27,2)
			else
				gb_graph_draw_tile(i,j,0,26,2)
			endif
		next
	next
	gb_graph_set_mode(gb_graph_mode_normal)
	gb_graph_enable_camera(true)
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_debug_init()
	gb_debug_logpos 	= new_int(0)
	gb_debug_log 			= new string[gb_debug_max_logs]
	gb_debug_panel		= new_bool(false)
endfunction

function gb_debug_load()
endfunction

function gb_debug_start()
endfunction

function gb_debug_update(d:float)
	if keyhit(key_f1)
		bool_toggle(gb_debug_panel)
	endif
	if keyhit(key_f6)
		bool_toggle(gb_graph_pixelmode)
	endif
	if keydown(key_f7)
		number_add(gb_speed,-0.05)
	endif
	if keydown(key_f8)
		number_add(gb_speed,0.05)
	endif
	if keyhit(key_f5)
		bool_toggle(gb_visual_use_scanlines)
	endif
	if keyhit(key_f9)
		number_add(gb_visual_scanlines_style,1)
	endif
	if keyhit(key_f10)
		number_add(gb_visual_scanlines_thick,1)
	endif
endfunction

function gb_debug_draw_bg(x:float=0, y:float=0)
	if bool_eq(gb_debug_panel)
		gb_debug_draw_grid_bg()
	endif
endfunction

function gb_debug_draw_fg(x:float=0, y:float=0)
	if bool_eq(gb_debug_panel)
		gb_debug_draw_grid_fg()
		gb_debug_draw_panel()
	endif
endfunction

function gb_debug_end()
endfunction
