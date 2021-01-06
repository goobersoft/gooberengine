include "dict/dictval.bmx"

type t_dict
	field length 		:int
	field	pos 			:int
	field keys			:string[]
	field values		:t_dictval[]
endtype

function new_dict:t_dict()
	local r:t_dict = new t_dict
	r.length 		= 0
	r.pos 			= undefined
	r.keys			= new string[0]
	r.values		= new t_dictval[0]
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function dict_lencheck(d:t_dict)
	local u:int = dict_find_pos(d,null)
	if (u = undefined) then dict_expand(d)
endfunction

function dict_expand(d:t_dict)
	if (d.length = 0)
		d.length = 1
		d.pos = 0
	elseif (d.length > 0)
		d.length = d.length * 2
	endif
	d.keys 		= d.keys[..d.length]
	d.values	= d.values[..d.length]
endfunction

function dict_find:t_dictval(d:t_dict, k:string=null)
	if (d.length = 0)
		return null
	else
		local u:int = d.length
		while u > 0
			if (d.keys[d.pos] = k)
				return d.values[d.pos]
			endif
			d.pos = (d.pos + 1) mod d.length
			u = u - 1
		wend
	endif
	return null
endfunction

function dict_find_pos:int(d:t_dict, k:string=null)
	if (d.length = 0)
		return undefined
	else
		local u:int = d.length
		while u > 0
			if (d.keys[d.pos] = k)
				return d.pos
			endif
			d.pos = (d.pos + 1) mod d.length
			u = u - 1
		wend
	endif
	return undefined
endfunction



function dict_load:int( d:t_dict, t:string )
	local r:t_dict = gbml_load(t)
	if (r)
		d.length 		= r.length
		d.pos 			= r.pos
		d.keys 			= r.keys
		d.values 		= r.values
		return true
	endif
	return false
endfunction

function dict_save(d:t_dict, t:string)
	gbml_save(d,t)
endfunction

'''''''''''''
'' setters ''
'''''''''''''

function dict_set_val:t_dictval(d:t_dict, k:string, v:t_dictval)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = v
	return d.values[u]
endfunction

function dict_set_byte:t_dictval(d:t_dict, k:string, v:byte)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_byte(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_short:t_dictval(d:t_dict, k:string, v:short)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_short(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_int:t_dictval(d:t_dict, k:string, v:int)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_int(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_long:t_dictval(d:t_dict, k:string, v:long)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_long(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_float:t_dictval(d:t_dict, k:string, v:float)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_float(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_double:t_dictval(d:t_dict, k:string, v:double)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_double(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_string:t_dictval(d:t_dict, k:string, v:string)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_string(d.values[u], v)
	return d.values[u]
endfunction

function dict_set_dir:t_dictval(d:t_dict, k:string, t:t_dict=null)
	dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_dir(d.values[u],t)
	return d.values[u]
endfunction

function dict_set_data:t_dictval ( d:t_dict, k:string, v:object )
  dict_lencheck(d)
	local u:int = dict_find_pos(d,k)
	if (u = undefined) then u = dict_find_pos(d,null)
	d.keys[u] = k
	d.values[u] = new_dictval()
	dictval_set_data(d.values[u], v)
	return d.values[u]
endfunction

'''''''''''''
'' setters ''
'''''''''''''

function dict_get_byte:byte(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_byte(d.values[u])
	endif
	return 0
endfunction

function dict_get_short:short(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_short(d.values[u])
	endif
	return 0
endfunction

function dict_get_int:int(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_int(d.values[u])
	endif
	return 0
endfunction

function dict_get_long:long(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_long(d.values[u])
	endif
	return 0
endfunction

function dict_get_float:float(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_float(d.values[u])
	endif
	return 0
endfunction

function dict_get_double:double(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_double(d.values[u])
	endif
	return 0
endfunction

function dict_get_string:string(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_string(d.values[u])
	endif
	return 0
endfunction

function dict_get_dir:t_dict(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_dir(d.values[u])
	endif
	return null
endfunction

function dict_get_data:object(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_data(d.values[u])
	endif
	return null
endfunction

function dict_get_id:int(d:t_dict, k:string)
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		return dictval_get_id(d.values[u])
	endif
	return undefined
endfunction

function dict_get_type:string( d:t_dict, k:string )
	local u:int = dict_find_pos(d,k)
	if (u <> undefined)
		select d.values[u].id
			case dictval_id_byte
				return "byte"
			case dictval_id_short
				return "short"
			case dictval_id_int
				return "int"
			case dictval_id_long
				return "long"
			case dictval_id_float
				return "float"
			case dictval_id_double
				return "double"
			case dictval_id_string
				return "string"
			case dictval_id_dir
				return "dir"
      case dictval_id_data
        return "data"
      default
        return "null"
		endselect
	endif
	return null
endfunction

'''''''''''''
'' writing ''
'''''''''''''

function dict_write_val:int( d:t_dict, s:string, v:t_dictval )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_val(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_val(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_byte:int( d:t_dict, s:string, v:byte )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_byte(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_byte(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_short:int( d:t_dict, s:string, v:short )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_short(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_short(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_int:int( d:t_dict, s:string, v:int )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_int(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_int(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_long:int( d:t_dict, s:string, v:long )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_long(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_long(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_float:int( d:t_dict, s:string, v:float )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_float(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_float(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_double:int( d:t_dict, s:string, v:double )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_double(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_double(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_string:int( d:t_dict, s:string, v:string )
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_string(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_string(g, w[w.length-1], v)
		return true
	endif
endfunction

function dict_write_dir:int( d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_dir(d,w[0])
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_dir(g, w[w.length-1])
		return true
	endif
endfunction

function dict_write_data:int (d:t_dict, s:string, v:object)
  local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return false
	elseif (w.length = 1)
		dict_set_data(d,w[0],v)
		return true
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length])
		dict_set_data(g, w[w.length-1], v)
		return true
	endif
endfunction
global dict_write:int (d:t_dict, s:string, v:object) = dict_write_data

'''''''''''''
'' Reading ''
'''''''''''''

function dict_read_byte:byte(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_byte(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_byte(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_short:short(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_short(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_short(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_int:int(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_int(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_int(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_long:long(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_long(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_long(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_float:float(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_float(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_float(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_double:double(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_double(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_double(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_string:string(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_string(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_string(g,w[w.length-1])
		else
			return 0
		endif
	endif
endfunction

function dict_read_dir:t_dict(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_dir(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_dir(g,w[w.length-1])
		else
			return null
		endif
	endif
endfunction

function dict_read_data:object(d:t_dict, s:string)
	local w:string[] = tokenize(s,"/",false)
	local g:t_dict = null
	if (w.length = 0)
		return null
	elseif (w.length = 1)
		return dict_get_data(d, w[0])
	elseif (w.length > 1)
		g = dict_goto_dir(d, w[..w.length], false)
		if (g)
			return dict_get_data(g,w[w.length-1])
		else
			return null
		endif
	endif
endfunction
global dict_read:object(d:t_dict, s:string) = dict_read_data

''''''''''
'' goto ''
''''''''''

function dict_goto_dir:t_dict( d:t_dict, s:string[], wd:int=true)
	local q:t_dict = d
	local g:t_dict = null
	if (s.length > 0)
		local i:int = 0
		local u:int = 0
		while i < s.length-1
			g = dict_get_dir(q,s[i])
			if (g)
				q = g
			elseif (wd)
				dict_set_dir(q,s[i])
				q = dict_get_dir(q,s[i])
			else
				return null
			endif
			i = i + 1
		wend
	endif
	return q
endfunction