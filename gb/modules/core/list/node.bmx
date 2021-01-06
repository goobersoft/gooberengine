
'''''''''''''''
'' list node ''
'''''''''''''''

type t_list_node
	field left 			:t_list_node
	field right 		:t_list_node
	field depth     :t_float
  field data      :t_datum
endtype

function new_list_node:t_list_node(o:object, d:float=0.0, t:string="list_node")
	local r:t_list_node = new t_list_node
	r.left 		= null
	r.right 	= null
	r.depth   = new_float(d)
  r.data    = new_datum(o,t)
	return r
endfunction

function list_node_set_data(l:t_list_node, o:object)
  datum_set(l.data, o, "list_node")
endfunction
function list_node_set_depth(l:t_list_node, d:float)
  float_set(l.depth, d)
endfunction

function list_node_depth:float(l:t_list_node)
  return float_get(l.depth)
endfunction
function list_node_left:t_list_node (l:t_list_node)
  return l.left
endfunction
function list_node_right:t_list_node (l:t_list_node)
  return l.right
endfunction
function list_node_data:t_datum(l:t_list_node)
  return l.data
endfunction