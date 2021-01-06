
type t_gb_console_char
  field code    :t_int
  field color   :t_color
endtype

function new_gb_console_char:t_gb_console_char()
  local r:t_gb_console_char = new t_gb_console_char
  r.code    = new_int(0)
  r.color   = new_color(3,3,3,3)
  return r
endfunction

function new_gb_console_char_v:t_gb_console_char[] (n:int)
  n = ilow(n,1)
  local r:t_gb_console_char[] = new t_gb_console_char[n]
  for local i:int = 0 to n-1
    r[i] = new_gb_console_char()
  next
  return r
endfunction

function new_gb_console_char_vv:t_gb_console_char[][] (x:int, y:int)
  x = ilow(x,1)
  local r:t_gb_console_char[][] = new t_gb_console_char[x][]
  for local i:int = 0 to x-1
    r[i] = new_gb_console_char_v(y)
  next
  return r
endfunction

'''''''''''''
'' drawing ''
'''''''''''''

function gb_console_char_draw(c:t_gb_console_char, x:float, y:float)
  
endfunction

function gb_console_char_v_draw(c:t_gb_console_char[], x:float, y:float)
endfunction

function gb_console_char_vv_draw(c:t_gb_console_char[][], x:float, y:float)
endfunction