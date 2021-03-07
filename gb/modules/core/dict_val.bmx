
''''''''''''
'' consts ''
''''''''''''

Const dict_val_id_none		:Int = 0
Const dict_val_id_byte		:Int = 1
Const dict_val_id_short 	:Int = 2 
Const dict_val_id_int		  :Int = 3
Const dict_val_id_long		:Int = 4
Const dict_val_id_float	  :Int = 5
Const dict_val_id_double  :Int = 6
Const dict_val_id_string	:Int = 7
Const dict_val_id_dir		  :Int = 8
const dict_val_id_data    :int = 9

''''''''''
'' type ''
''''''''''

type t_dict_val
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

function new_dict_val:t_dict_val()
	local d:t_dict_val = new t_dict_val
	return d
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function dict_val_clear(d:t_dict_val)
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

function dict_val_eval(d:t_dict_val, s:string)
	if len(s) >= 2
		select chr(s[0])
			case "@"
				dict_val_set_byte(d,byte(s[1..]))
			case "&"
				dict_val_set_short(d,short(s[1..]))
			case "%"
				dict_val_set_int(d,int(s[1..]))
			case "^"
				dict_val_set_long(d,long(s[1..]))
			case "#"
				dict_val_set_float(d,float(s[1..]))
			case "!"
				dict_val_set_double(d,double(s[1..]))
			case "$"
				dict_val_set_string(d,string(s[1..]))
			case "~q"
				if s[s.length-1] = "~q"
					dict_val_set_string(d,s[1..s.length-1])
				else
					dict_val_set_string(d,s)
				endif
			default
				dict_val_set_string(d,s)
		endselect
	elseif len(s) = 1
		dict_val_set_string(d,s)
	endif
endfunction

function dict_val_set_byte(d:t_dict_val, v:byte)
	dict_val_clear(d)
	d.id 			= dict_val_id_byte
	d.tbyte 	= new_byte(v)
endfunction

function dict_val_set_short(d:t_dict_val, v:short)
	dict_val_clear(d)
	d.id 			= dict_val_id_short
	d.tshort 	= new_short(v)
endfunction

function dict_val_set_int(d:t_dict_val, v:int)
	dict_val_clear(d)
	d.id 			= dict_val_id_int
	d.tint 		= new_int(v)
endfunction

function dict_val_set_long(d:t_dict_val, v:long)
	dict_val_clear(d)
	d.id 			= dict_val_id_long
	d.tlong 	= new_long(v)
endfunction

function dict_val_set_float(d:t_dict_val, v:float)
	dict_val_clear(d)
	d.id 			= dict_val_id_float
	d.tfloat 	= new_float(v)
endfunction

function dict_val_set_double(d:t_dict_val, v:double)
	dict_val_clear(d)
	d.id 				= dict_val_id_double
	d.tdouble 	= new_double(v)
endfunction

function dict_val_set_string(d:t_dict_val, v:string)
	dict_val_clear(d)
	d.id 				= dict_val_id_string
	d.tstring 	= new_string(v)
endfunction

function dict_val_set_dir(d:t_dict_val, t:t_dict=null)
	dict_val_clear(d)
	d.id 			= dict_val_id_dir
	if (t)
		d.tdir		= t
	else
		d.tdir 		= new_dict()
	endif
endfunction

function dict_val_set_data(d:t_dict_val, o:object)
  dict_val_clear(d)
	d.id 				= dict_val_id_data
	d.tdata     = o
endfunction

function dict_val_get_byte:byte(d:t_dict_val)
	if (d.id = dict_val_id_byte) then return d.tbyte.value else return 0
endfunction

function dict_val_get_short:short(d:t_dict_val)
	if (d.id = dict_val_id_short) then return d.tshort.value else return 0
endfunction

function dict_val_get_int:int(d:t_dict_val)
	if (d.id = dict_val_id_int) then return d.tint.value else return 0
endfunction

function dict_val_get_long:long(d:t_dict_val)
	if (d.id = dict_val_id_long) then return d.tlong.value else return 0
endfunction

function dict_val_get_float:float(d:t_dict_val)
	if (d.id = dict_val_id_float) then return d.tfloat.value else return 0
endfunction

function dict_val_get_double:double(d:t_dict_val)
	if (d.id = dict_val_id_double) then return d.tdouble.value else return 0
endfunction

function dict_val_get_string:string(d:t_dict_val)
	if (d.id = dict_val_id_string) then return d.tstring.value else return 0
endfunction

function dict_val_get_dir:t_dict(d:t_dict_val)
	if (d.id = dict_val_id_dir) then return d.tdir else return null
endfunction

function dict_val_get_data:object(d:t_dict_val)
  if (d.id = dict_val_id_data) then return d.tdata else return null
endfunction

function dict_val_get_id:int(d:t_dict_val)
	return d.id
endfunction