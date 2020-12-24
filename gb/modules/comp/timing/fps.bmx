
type t_gb_timing_fps
  field ticks   :t_int
  field value   :t_int
  field time    :t_long
endtype

function new_gb_timing_fps:t_gb_timing_fps ()
  local r:t_gb_timing_fps = new t_gb_timing_fps
  r.ticks   = new_int(0)
  r.value   = new_int(0)
  r.time    = new_long(0)
  return r
endfunction