
type t_gb_console_cursor
  field flash   :t_bool
  field timer   :t_timer
  field pos     :t_int
endtype

function new_gb_console_cursor:t_gb_console_cursor ()
  local r:t_gb_console_cursor = new t_gb_console_cursor
  r.flash     = new_bool(true)
  r.timer     = new_timer(0.5)
  r.pos       = new_int(0)
  return r
endfunction