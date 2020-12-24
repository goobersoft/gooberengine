
type t_gb_timing_deltatime
  field mdiff   :t_int
  field sdiff   :t_float
endtype

function new_gb_timing_deltatime:t_gb_timing_deltatime ()
  local r:t_gb_timing_deltatime = new t_gb_timing_deltatime
  r.mdiff = new_int(0)
  r.sdiff = new_float(0)
  return r
endfunction