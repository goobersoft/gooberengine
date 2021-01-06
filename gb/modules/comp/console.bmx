
include "console/cursor.bmx"
include "console/char.bmx"

''''''''''''
'' consts ''
''''''''''''

const gb_console_max_input  :int = 1000
const gb_console_max_lines  :int = 1000
const gb_console_chars_x    :int = 80
const gb_console_chars_y    :int = 24


'''''''''''''
'' globals ''
'''''''''''''

''''''''''
'' type ''
''''''''''

type t_gb_console
  field active        :t_bool
  field input         :t_mstring
  field blinktimer    :t_timer
endtype

function new_gb_console:t_gb_console ()
  local r:t_gb_console = new t_gb_console
  r.active      = new_bool(false)
  r.input       = new_mstring()
  r.blinktimer  = new_timer(0.5)
  return r
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
  if keyhit(key_tilde)
    bool_toggle(gb.console.active)
    flushkeys()
  endif

  if bool_eq(gb.console.active)
    local c:byte
    repeat
      c = getchar()

      select c
      endselect

    until c = 0
  endif
endfunction

function gb_console_draw(x:float=0, y:float=0)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_console_put_char(n:int)
  
endfunction

function gb_console_refresh_lines()
endfunction

function gb_console_add_input_char(s:string)
endfunction

function gb_console_print_line(s:string)
endfunction

'' input as string
function gb_console_out(s:string)
endfunction

'' input as integer value
function gb_console_out(v:int)
endfunction

'' input as a floating point value
function gb_console_out(v:float)
endfunction

function gb_console_log(t:string)
  print(t)
endfunction

'' writes the current console output session to a file.
'' this is automatically called when the program ends.
function gb_console_to_file(s:string="log.txt")
endfunction
