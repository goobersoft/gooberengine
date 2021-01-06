
''''''''''''
'' consts ''
''''''''''''

Const dictval_id_none		:Int = 0
Const dictval_id_byte		:Int = 1
Const dictval_id_short 	:Int = 2 
Const dictval_id_int		:Int = 3
Const dictval_id_long		:Int = 4
Const dictval_id_float	:Int = 5
Const dictval_id_double :Int = 6
Const dictval_id_string	:Int = 7
Const dictval_id_dir		:Int = 8
const dictval_id_data   :int = 9

''''''''''
'' type ''
''''''''''

type t_dictval
	field id				:int
	field tbyte			:t_byte
	field tshort		:t_short
	field tint			:t_int
	field tlong			:t_long
	field tfloat		:t_float
	field tdouble		:t_double
	field tstring		:t_string
	field tdir			:t_dict
  field tdata     :object
endtype

function new_dictval:t_dictval()
	local d:t_dictval = new t_dictval
	return d
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function dictval_clear(d:t_dictval)
	d.tbyte 	= null
	d.tshort 	= null
	d.tint 		= null
	d.tlong 	= null
	d.tfloat 	= null
	d.tdouble = null
	d.tstring = null
	d.tdir 		= null
  d.tdata   = null
endfunction

function dictval_eval(d:t_dictval, s:string)
	if len(s) >= 2
		select chr(s[0])
			case "@"
				dictval_set_byte(d,byte(s[1..]))
			case "&"
				dictval_set_short(d,short(s[1..]))
			case "%"
				dictval_set_int(d,int(s[1..]))
			case "^"
				dictval_set_long(d,long(s[1..]))
			case "#"
				dictval_set_float(d,float(s[1..]))
			case "!"
				dictval_set_double(d,double(s[1..]))
			case "$"
				dictval_set_string(d,string(s[1..]))
			case "~q"
				if s[s.length-1] = "~q"
					dictval_set_string(d,s[1..s.length-1])
				else
					dictval_set_string(d,s)
				endif
			default
				dictval_set_string(d,s)
		endselect
	elseif len(s) = 1
		dictval_set_string(d,s)
	endif
endfunction

function dictval_set_byte(d:t_dictval, v:byte)
	dictval_clear(d)
	d.id 			= dictval_id_byte
	d.tbyte 	= new_byte(v)
endfunction

function dictval_set_short(d:t_dictval, v:short)
	dictval_clear(d)
	d.id 			= dictval_id_short
	d.tshort 	= new_short(v)
endfunction

function dictval_set_int(d:t_dictval, v:int)
	dictval_clear(d)
	d.id 			= dictval_id_int
	d.tint 		= new_int(v)
endfunction

function dictval_set_long(d:t_dictval, v:long)
	dictval_clear(d)
	d.id 			= dictval_id_long
	d.tlong 	= new_long(v)
endfunction

function dictval_set_float(d:t_dictval, v:float)
	dictval_clear(d)
	d.id 			= dictval_id_float
	d.tfloat 	= new_float(v)
endfunction

function dictval_set_double(d:t_dictval, v:double)
	dictval_clear(d)
	d.id 				= dictval_id_double
	d.tdouble 	= new_double(v)
endfunction

function dictval_set_string(d:t_dictval, v:string)
	dictval_clear(d)
	d.id 				= dictval_id_string
	d.tstring 	= new_string(v)
endfunction

function dictval_set_dir(d:t_dictval, t:t_dict=null)
	dictval_clear(d)
	d.id 			= dictval_id_dir
	if (t)
		d.tdir		= t
	else
		d.tdir 		= new_dict()
	endif
endfunction

function dictval_set_data(d:t_dictval, o:object)
  dictval_clear(d)
	d.id 				= dictval_id_data
	d.tdata     = o
endfunction

function dictval_get_byte:byte(d:t_dictval)
	if (d.id = dictval_id_byte) then return d.tbyte.value else return 0
endfunction

function dictval_get_short:short(d:t_dictval)
	if (d.id = dictval_id_short) then return d.tshort.value else return 0
endfunction

function dictval_get_int:int(d:t_dictval)
	if (d.id = dictval_id_int) then return d.tint.value else return 0
endfunction

function dictval_get_long:long(d:t_dictval)
	if (d.id = dictval_id_long) then return d.tlong.value else return 0
endfunction

function dictval_get_float:float(d:t_dictval)
	if (d.id = dictval_id_float) then return d.tfloat.value else return 0
endfunction

function dictval_get_double:double(d:t_dictval)
	if (d.id = dictval_id_double) then return d.tdouble.value else return 0
endfunction

function dictval_get_string:string(d:t_dictval)
	if (d.id = dictval_id_string) then return d.tstring.value else return 0
endfunction

function dictval_get_dir:t_dict(d:t_dictval)
	if (d.id = dictval_id_dir) then return d.tdir else return null
endfunction

function dictval_get_data:object(d:t_dictval)
  if (d.id = dictval_id_data) then return d.tdata else return null
endfunction

function dictval_get_id:int(d:t_dictval)
	return d.id
endfunction