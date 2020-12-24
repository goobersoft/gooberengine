
'''''''''''''''
'' list node ''
'''''''''''''''

type t_listnode
	field left 			:t_listnode
	field right 		:t_listnode
	field depth     :t_float
  field data      :object
endtype

function new_listnode:t_listnode(i:int=0, f:float=0)
	local r:t_listnode = new t_listnode
	r.left 		= null
	r.right 	= null
	r.depth   = new_float(0.0)
  r.data    = null
	return r
endfunction

function listnode_set_data(l:t_listnode, d:object)
  l.data = d
endfunction
function listnode_set_depth(l:t_listnode, d:float)
  float_set(l.depth, d)
endfunction

function listnode_depth(l:t_listnode)
  return l.depth
endfunction
function listnode_left:t_listnode (l:t_listnode)
  return l.left
endfunction
function listnode_right:t_listnode (l:t_listnode)
  return l.right
endfunction
function listnode_data:object(l:t_listnode)
  return l.data
endfunction