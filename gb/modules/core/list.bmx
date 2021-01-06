
include "list/node.bmx"
include "list/iterator.bmx"

''''''''''''
'' consts ''
''''''''''''

const list_sortmode_descending  :int = -1
const list_sortmode_off         :int =  0
const list_sortmode_ascending   :int =  1

''''''''''
'' list ''
''''''''''

type t_list	
	field first 	    :t_list_node
	field last		    :t_list_node
	field count		    :t_int
  field sortmode    :t_int
  field iterator    :t_list_iterator
endtype

function new_list:t_list()
	local r:t_list = new t_list
	r.first 	    = null
	r.last 		    = null
	r.count 	    = new_int(0)
  r.sortmode    = new_int(list_sortmode_off)
  r.iterator    = null
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function list_add_first:int(l:t_list, t:string="list_node", o:object, d:float=0.0)
  local u:t_list_node = new_list_node(o,d,t)

  if l.count.value = 0
    l.first   = u
    l.last    = u
    int_set   (l.count, 1)
  else
    local w:t_list_node = l.first
    l.first   = u
    w.left    = u
    u.right   = w
    int_add   (l.count, 1)
  endif

  return l.count.value
endfunction

function list_add_last:int (l:t_list, t:string="list_node", o:object, d:float=0.0)
  local u:t_list_node = new_list_node(o,d,t)
  if l.count.value = 0
    l.first   = u
    l.last    = u
    int_set   (l.count, 1)
  else
    local w:t_list_node = l.last
    l.last    = u
    w.right   = u
    u.left    = w
    int_add   (l.count, 1)
  endif

  return l.count.value
endfunction

function list_remove_first:t_datum(l:t_list)
  local r:t_datum
  if l.count.value = 1
    r = list_node_data(l.first)
    l.first = null
    l.last  = null
    int_set(l.count, 0)
  else
    r = list_node_data(l.first)
    l.first        = l.first.right
    l.first.left   = null
    int_add(l.count, -1)
  endif
  return r
endfunction

function list_remove_last:t_datum(l:t_list)
  local r:t_datum
  if l.count.value = 1
    r = list_node_data(l.last)
    l.first = null
    l.last = null
    int_set(l.count, 0)
  else
    r = list_node_data(l.last)
    l.last        = l.last.left
    l.last.right  = null
    int_add       (l.count, -1)
  endif
  return r
endfunction

'''''''''''''

function list_clear(l:t_list)
  l.count.value   = 0
  l.first         = null
  l.last          = null
endfunction

function list_count:int(l:t_list)
  return l.count.value
endfunction

function list_to_string:string(l:t_list)
  local r:string = "["

  if l.count.value > 0
    local u:int = 0
    local v:t_list_node = l.first
    local w:t_datum
    while u < l.count.value
      r = r + datum_to_string(v.data)
      v = v.right
      u = u + 1
      if u < l.count.value
        r = r + ", "
      endif
    wend
  endif
  r = r + "]"
  return r
endfunction

function list_iterate_datum:t_datum(l:t_list)
  if not l.iterator
    if l.count.value > 0
      l.iterator = new_list_iterator(l)
      return list_iterator_curr(l.iterator)
    endif
  else
    list_iterator_right(l.iterator)
    if l.iterator.curr
      return list_iterator_curr(l.iterator)
    else
      l.iterator = null
    endif
  endif
endfunction

function list_iterate:object(l:t_list)
  local d:t_datum = list_iterate_datum(l)
  if d then return d.value
endfunction

function list_is_iterating:int(l:t_list)
  if (l.iterator) then return true
  return false
endfunction