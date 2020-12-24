
include "list/listnode.bmx"

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
	field first 	    :t_listnode
	field last		    :t_listnode
	field count		    :t_int
  field ascending   :t_bool
endtype

function new_list:t_list()
	local r:t_list = new t_list
	r.first 	    = null
	r.last 		    = null
	r.count 	    = new_int(0)
  r.sortmode    = new_int(list_sortmode_off)
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function list_