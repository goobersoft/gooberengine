
type t_gb_graph_perspec
  field value     :t_number
  field active    :t_bool
endtype

function new_gb_graph_perspec:t_gb_graph_perspec ()
  local r:t_gb_graph_perspec = new t_gb_graph_perspec
  r.value     = new_number(24, 1,48)
  r.active    = new_bool(true)
  return r
endfunction