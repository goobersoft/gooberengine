

type t_script_node
  field label     :t_string
  field keyword   :t_string
  field child     :object
  method get:string(); return string_get(label); endmethod
endtype

function new_script_node:t_script_node (k:string="", l:string="")
  local r:t_script_node = new t_script_node
  r.label = new_string("")
  r.keyword = new_string("")
  r.child = null
  return r
endfunction

'''''''''
'' set ''
'''''''''

type t_script_node_set
  field destvar :t_string
  field destval :t_datum
endtype

function new_script_node_set:t_script_node(l:string="")
  local r:t_script_node = new_script_node("set",l)
  local c:t_script_node_set = new t_script_node_set
  c.destvar = new_string("")
  c.destval = new_datum()
  r.child = c
  return r
endfunction

'''''''''
'' get ''
'''''''''

