
type t_gb_visual_scanlines
  field active  :t_bool
  field style   :t_number
  field thick   :t_number
endtype

function new_gb_visual_scanlines:t_gb_visual_scanlines ()
  local r:t_gb_visual_scanlines = new t_gb_visual_scanlines
  r.active    = new_bool(false)
  r.style     = new_number(0,0,4,true)
  r.thick     = new_number(0,0,4,true)
  return r
endfunction