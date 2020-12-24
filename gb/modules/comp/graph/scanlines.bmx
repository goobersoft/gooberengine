
type t_gb_graph_scanlines
  field active  :t_bool
  field style   :t_int
  field thick   :t_int
endtype

function new_gb_graph_scanlines:t_gb_graph_scanlines ()
  local r:t_gb_graph_scanlines = new t_gb_graph_scanlines
  r.active    = new_bool(false)
  r.style     = new_int(0)
  r.thick     = new_int(0)
  return r
endfunction