
''''''''''''
'' consts ''
''''''''''''

const gb_console_chars_x    :int = 80
const gb_console_chars_y    :int = 24
const gb_console_max_lines  :int = 1000

'''''''''''''
'' globals ''
'''''''''''''

global gb_console_input					:t_string
global gb_console_lines 				:t_string[]
global gb_console_enabled 			:t_bool
global gb_console_cursor_flash	:t_bool
global gb_console_cursor_timer	:t_timer
global gb_console_cursor_pos		:t_int
global gb_console_data					:t_dict       '' gbml tree
global gb_console_logchain      :t_logchain

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_console_refresh_lines()
endfunction

function gb_console_add_input_char(s:string)
	string_add(gb_console_input,s)
endfunction

function gb_console_print_line(s:string)
	'' s should be 80 characters or less.
	for local i:int = 1 to 22
		gb_console_lines[i-1] = gb_console_lines[i]
	next
	gb_console_lines[22] = new_string(s)
endfunction

'' input as string
function gb_console_out(s:string)
	local u:string[] = cut(s)
	for local i:int = 0 to len(u)-1
		gb_console_print_line(u[i])
	next
endfunction

'' input as integer value
function gb_console_out(v:int)
  local s:string = string(v)
	local u:string[] = cut(s)
	for local i:int = 0 to len(u)-1
		gb_console_print_line(u[i])
	next
endfunction

'' input as a floating point value
function gb_console_out(v:float)
  local s:string = string(strdec(v))
	local u:string[] = cut(s)
	for local i:int = 0 to len(u)-1
		gb_console_print_line(u[i])
	next
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_console_init()
	gb_console_enabled 		= new_bool(false)
	gb_console_input			= new_string("")
	gb_console_lines			= new t_string[23]
	for local i:int = 0 to 22
		gb_console_lines[i] = new_string("")
	next
	gb_console_cursor_flash = new_bool(true)
	gb_console_cursor_timer = new_timer(0.5)
	gb_console_cursor_pos		= new_int(0)
	gb_console_data					= new_dict()
endfunction

function gb_console_load()
endfunction

function gb_console_update(d:float)
	if keyhit(key_tilde)
		bool_toggle(gb_console_enabled)
		flushkeys()
	endif
	
	if bool_eq(gb_console_enabled)
	
		local u:byte = getchar()
		while u <> 0
			select u
				case key_backspace
					string_del(gb_console_input)
				case key_return, key_enter
					gbs_interpret(string_get(gb_console_input))
					string_set(gb_console_input,"")
				case key_tab
					gb_console_add_input_char("  ")
				default
					gb_console_add_input_char(chr(u))
			endselect
			timer_reset(gb_console_cursor_timer)
			bool_set(gb_console_cursor_flash,true)
			u = getchar()
		wend
	
		timer_update(gb_console_cursor_timer,d)
		if timer_finished(gb_console_cursor_timer)
			timer_reset(gb_console_cursor_timer)
			bool_toggle(gb_console_cursor_flash)
		endif
	endif
endfunction

function gb_console_draw(x:float=0, y:float=0)
	if bool_eq(gb_console_enabled)
		gb_graph_enable_camera(false)
		gb_graph_set_mode(gb_graph_mode_multiply)
		gb_graph_draw_tile_rect(0,0,0,27,22,40,23)
		gb_graph_draw_tile_rect(0,23,0,27,21,40,1)
		gb_graph_set_mode(gb_graph_mode_normal)
		
		for local i:int = 0 to 22
			gb_graph_draw_tile_text(0.25,i,1,gb_console_lines[i].value)
		next
		gb_graph_draw_tile_text(0,23,1,right(gb_console_input.value,79))
		
		gb_graph_draw_tile_box( ..
			high(string_length(gb_console_input)/2.0,39.5),
			23,0,28+(0.5*gb_console_cursor_flash.value),22,0.5,1)
		gb_graph_enable_camera(true)


	endif
endfunction