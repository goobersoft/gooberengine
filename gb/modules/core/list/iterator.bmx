
type t_list_iterator
  field curr:t_list_node
endtype

function new_list_iterator:t_list_iterator ( l:t_list )
  local r:t_list_iterator = new t_list_iterator
  r.curr = l.first
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function list_iterator_curr:t_datum (l:t_list_iterator)
  if (l.curr) then return l.curr.data
endfunction

function list_iterator_right:t_datum (l:t_list_iterator)
  if (l.curr) then l.curr = l.curr.right
  return list_iterator_curr(l)
endfunction

function list_iterator_left:t_datum (l:t_list_iterator)
  if (l.curr) then l.curr = l.curr.right
  return list_iterator_curr(l)
endfunction