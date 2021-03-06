
'''''''''''''
'' globals ''
'''''''''''''

global gb_debug_panelid :t_number

''''''''''''
'' events ''
''''''''''''

function gb_debug_init()
  gb_debug_panelid       = new_number(0,0,5,true)
endfunction

function gb_debug_load()
endfunction

function gb_debug_start()
endfunction

function gb_debug_update()
  local d:float   = gb_delta()

  if (keydown(key_lcontrol) or keydown(key_rcontrol)) and keydown(key_c)
    end
  endif

	if keyhit(key_f1)
    number_add(gb_debug_panelid,1)
	endif
	if keyhit(key_f6)
		bool_toggle(gb_graph_pixelmode)
	endif
	if keydown(key_f7)
		number_add(gb_timing_speed,-0.05)
	endif
	if keydown(key_f8)
		number_add(gb_timing_speed,0.05)
	endif
	if keyhit(key_f5)
		bool_toggle(gb_visual_scanlines_active)
	endif
	if keyhit(key_f9)
		number_add(gb_visual_scanlines_style,1)
	endif
	if keyhit(key_f10)
		number_add(gb_visual_scanlines_thick,1)
	endif
endfunction

function gb_debug_draw_bg(x:float=0, y:float=0)
	if number_gt(gb_debug_panelid, 0)
		gb_debug_draw_grid_bg()
	endif
endfunction

function gb_debug_draw_fg(x:float=0, y:float=0)
	if number_gt(gb_debug_panelid, 0)
		gb_debug_draw_grid_fg()
		gb_debug_draw_panel()
	endif
endfunction

function gb_debug_end()
endfunction


'''''''''''''''
'' functions ''
'''''''''''''''

function gb_debug_draw_panel()
	gb_graph_enable_camera(false)

  select gb_debug_panelid.value
    case 0
    case 1
      gb_graph_draw_tile_rect(0,0,0,27,23,40,1)
      gb_graph_draw_tile_text(0,0,1,"M:"+gcmemalloced())
      gb_graph_draw_tile_text(6,0,1,int(gb_mouse_tile_x())+","+int(gb_mouse_tile_y()))
      gb_graph_draw_tile_text(9,0,1,"P:"+gb_graph_pixelmode.value)
      gb_graph_draw_tile_text(11,0,1,"W:"+int(gb_visual_windowscale.value))
      gb_graph_draw_tile_text(13,0,1,"F:"+gb_timing_fps_value.value)
      gb_graph_draw_tile_text(16,0,1,stopwatch_tostring(gb_timing_stopwatch))
      local u:float = 0
      for local i:float = 0 to 11
        u = byte_get(gb_controller_buttons[i])
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
      gb_graph_draw_tile(25+gb_timing_speed.value,0,0,23,3)
      gb_graph_draw_tile_text(36,0,1,strdec(gb_timing_speed.value))
      gb_graph_draw_tile(39,0,0,28,2)
    case 2
      gb_graph_draw_tile_rect(0,0,0,27,23,40,1)
      gb_graph_draw_tile_text(0,0,1,"CPU"+gb_cpu_string(true))
      gb_graph_draw_tile(39,0,0,29,2)
    case 3
      gb_graph_draw_tile_rect(0,0,0,27,23,40,1)
      gb_graph_draw_tile(39,0,0,28,3)
    case 4
      gb_graph_draw_tile_rect(0,0,0,27,23,40,1)
      gb_graph_draw_tile(39,0,0,29,3)
  endselect
	
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