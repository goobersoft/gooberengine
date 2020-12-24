
include "console/cursor.bmx"

''''''''''''
'' consts ''
''''''''''''

const gb_console_chars_x    :int = 80
const gb_console_chars_y    :int = 24
const gb_console_max_lines  :int = 1000

'''''''''''''
'' globals ''
'''''''''''''

''''''''''
'' type ''
''''''''''

type t_gb_console
  field active    :t_bool
  field input     :t_string
  field lines     :t_string[]
  field cursor    :t_gb_console_cursor
  field data      :t_dict
  field logchain  :t_logchain
endtype

function new_gb_console:t_gb_console ()
  local r:t_gb_console = new t_gb_console
  r.active    = new_bool(false)
  r.input     = new_string("")
  r.lines     = new_string_v(23,"")
  r.data      = new_dict()
  r.logchain  = new_logchain( gb_console_max_lines )
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_console_init()
  gb.console = new_gb_console()
endfunction

function gb_console_load()
endfunction

function gb_console_update()
  local d:float   = gb_delta()
	if keyhit(key_tilde)
		bool_toggle(gb.console.active)
		flushkeys()
	endif
	
	if bool_eq(gb.console.active)
		local u:byte = getchar()
		while u <> 0
			select u
				case key_backspace
					string_del(gb.console.input)
				case key_return, key_enter
					gbs_interpret(string_get(gb.console.input))
					string_set(gb.console.input,"")
				case key_tab
					gb_console_add_input_char("  ")
				default
					gb_console_add_input_char(chr(u))
			endselect
			timer_reset(gb.console.cursor.timer)
			bool_set(gb.console.cursor.flash,true)
			u = getchar()
		wend
	
		timer_update(gb.console.cursor.timer)
		if timer_finished(gb.console.cursor.timer)
			timer_reset(gb.console.cursor.timer)
			bool_toggle(gb.console.cursor.flash)
		endif
	endif
endfunction

function gb_console_draw(x:float=0, y:float=0)
	if bool_eq(gb.console.active)
		gb_graph_enable_camera(false)
		gb_graph_set_mode(gb_graph_mode_multiply)
		gb_graph_draw_tile_rect(0,0,0,27,22,40,23)
		gb_graph_draw_tile_rect(0,23,0,27,21,40,1)
		gb_graph_set_mode(gb_graph_mode_normal)
		
		for local i:int = 0 to 22
			gb_graph_draw_tile_text(0.25,i,1,gb.console.lines[i].value)
		next
		gb_graph_draw_tile_text(0,23,1,right(gb.console.input.value,79))
		
		gb_graph_draw_tile_box( ..
			high(string_length(gb.console.input)/2.0,39.5),
			23,0,28+(0.5*gb.console.cursor.flash.value),22,0.5,1)
		gb_graph_enable_camera(true)
	endif
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_console_refresh_lines()
endfunction

function gb_console_add_input_char(s:string)
	string_add(gb.console.input,s)
endfunction

function gb_console_print_line(s:string)
	'' s should be 80 characters or less.
	for local i:int = 1 to 22
		gb.console.lines[i-1] = gb.console.lines[i]
	next
	gb.console.lines[22] = new_string(s)
endfunction

'' input as string
function gb_console_out(s:string)
	local u:string[] = cut(s)
	for local i:int = 0 to len(u)-1
		gb_console_print_line(u[i])
	next

  logchain_set    ( gb.console.logchain, s )
  logchain_next   ( gb.console.logchain )
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

function gb_console_log(t:string)
  print(t)
endfunction

'' writes the current console output session to a file.
'' this is automatically called when the program ends.
function gb_console_to_file(s:string="log.txt")
  
endfunction
