
''''''''''''
'' consts ''
''''''''''''

const list_null_float :float = -3e37

'''''''''''''''
'' list node ''
'''''''''''''''

type t_listnode
	field left 			:t_listnode
	field right 		:t_listnode
	field ival			:int
	field fval			:float
endtype

function new_listnode:t_listnode(i:int=0, f:float=0)
	local r:t_listnode = new t_listnode
	r.left 		= null
	r.right 	= null
	r.ival 		= i
	r.fval 		= f
	return r
endfunction

''''''''''
'' list ''
''''''''''

type t_list	
	field first 	:t_listnode
	field last		:t_listnode
	field count		:t_int
endtype

function new_list:t_list()
	local r:t_list = new t_list
	r.first 	= null
	r.last 		= null
	r.count 	= new_int(0)
	return r
endfunction

function list_add_first(l:t_list, i:int, f:float)
	local r :t_listnode = new_listnode(i,f)
	
	if l.count.value = 0
		l.first 	= r
		l.last 		= r
	else
		l.first.left 	= r
		r.right 			= l.first
		l.first 			= r
	endif
	
	iadd(l.count,1)
endfunction

function list_add_last(l:t_list, i:int, f:float)
	local r :t_listnode = new_listnode(i,f)
	
	if l.count.value = 0
		l.first = r
		l.last = r
	else
		l.last.right 	= r
		r.left 				= l.last
		l.last 				= r
	endif
	
	iadd(l.count,1)
endfunction

function list_remove_first(l:t_list)
	if l.count.value = 1
		l.count.value = 0
		l.first 	= null
		l.last 		= null
	elseif l.count.value > 1
		l.first 			= l.first.right
		l.first.left 	= null
	endif
	iadd(l.count,-1)
endfunction

function list_remove_last(l:t_list)
	if l.count.value = 1
		l.count.value = 0
		l.first 	= null
		l.last 		= null
	elseif l.count.value > 1
		l.last 				= l.last.left
		l.last.right 	= null
	endif
	iadd(l.count,-1)
endfunction

function list_add_list_first(l:t_list, l2:t_list)
endfunction

function list_add_list_last(l:t_list, l2:t_list)
endfunction

function list_first_index:int(l:t_list)
	if l.count.value > 0
		return l.first.ival
	endif
	return -1
endfunction

function list_first_depth:float(l:t_list)
	if l.count.value > 0
		return l.first.fval
	endif
	return list_null_float
endfunction

function list_last_index:int(l:t_list)
	if l.count.value > 0
		return l.last.ival
	endif
	return -1
endfunction

function list_last_depth:float(l:t_list)
	if l.count.value > 0
		return l.last.fval
	endif
	return list_null_float
endfunction

function list_to_string:string(l:t_list)
	local r:string 			= "["
	local n:t_listnode 	= null
	
	if l.count.value > 0
		print("yes")
		n = l.first
		while n
			r = r + n.ival + ":" + strdec(n.fval)
			if n.right
				r = r + " "
			endif
			n = n.right
		wend
	endif
	r = r + "]"
	return r
endfunction

function list_sort_by_index(l:t_list)
	if l.count.value > 1
	endif
endfunction

function list_sort_by_depth(l:t_list)
	if l.count.value > 1
	endif
endfunction