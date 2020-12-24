
type t_tag
	field name	:string
	field id		:int
endtype

function new_tag:t_tag(n:string="", id:int=0)
	local r:t_tag = new t_tag
	r.name 	= n
	r.id 		= id
	return r
endfunction

function tag_set_name(t:t_tag, n:string)
	t.name = n
endfunction

function tag_set_id(t:t_tag, id:int)
	t.id = id
endfunction

function tag_get_name:string(t:t_tag)
	return t.name
endfunction

function tag_get_id:int(t:t_tag)
	return t.id
endfunction