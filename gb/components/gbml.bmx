
''''''''''''
'' consts ''
''''''''''''

Const gbml_ext 									:String = ".gbml"
Const gbml_token_byte						:String = "@"
Const gbml_token_short					:String = "&"
Const gbml_token_int    				:String = "%"
Const gbml_token_long   				:String = "^"
Const gbml_token_float  				:String = "#"
Const gbml_token_double 				:String = "!"
Const gbml_token_string 				:String = "$"
Const gbml_token_dir						:String = "*"
Const gbml_token_bracket_in			:String = "{"
Const gbml_token_bracket_out		:String = "}"
Const gbml_token_assignment			:String = ":"
Const gbml_token_separator			:String = ";"

'''''''''''''
'' globals ''
'''''''''''''

'''''''''''''''
'' functions ''
'''''''''''''''

function gbml_save( d:t_dict, s:string="tmp" )
	local t:string = gbml_to_string(d)
	local ts:tstream = writestream(s+gbml_ext)
	writestring(ts,t)
	closestream(ts)
endfunction

function gbml_load:t_dict( s:string="tmp")
	local u:int = filetype(s+gbml_ext)
	local t:string = ""
	if (u = 1)
		local ts:tstream 	= readstream(s+gbml_ext)
		local b:int 			= streamsize(ts)
		local tss:string 	= readstring(ts,b)
		closestream(ts)
		return gbml_from_string(tss)
	endif
	return null
endfunction
		

function gbml_to_string:string(d:t_dict, s:int=0)
	local sb:tstringbuilder = new tstringbuilder
	If (d.values.length > 0)
			For Local i:Int = 0 To d.values.length-1
				If (d.values[i])
					Select d.values[i].id
						Case dictval_id_byte
							sb.append(rep("  ",s) + d.keys[i] + " : @" + byte_get(d.values[i].tbyte) + ";~n")
						Case dictval_id_short
							sb.append(rep("  ",s) + d.keys[i] + " : &" + short_get(d.values[i].tshort) + ";~n")
						Case dictval_id_int
							sb.append(rep("  ",s) + d.keys[i] + " : %" + int_get(d.values[i].tint) + ";~n")
						Case dictval_id_long
							sb.append(rep("  ",s) + d.keys[i] + " : ^" + long_get(d.values[i].tlong) + ";~n")
						Case dictval_id_float
							sb.append(rep("  ",s) + d.keys[i] + " : #" + float_get(d.values[i].tfloat) + ";~n")
						Case dictval_id_double
							sb.append(rep("  ",s) + d.keys[i] + " : !" + double_get(d.values[i].tdouble) + ";~n")
						Case dictval_id_string
							sb.append(rep("  ",s) + d.keys[i] + " : $" + string_get(d.values[i].tstring) + ";~n")
						Case dictval_id_dir
							Local g:t_dict = d.values[i].tdir
							sb.append(rep("  ",s) + d.keys[i] + " {~n" + gbml_to_string(g,s+1) + rep("  ",s) + "}~n")
					EndSelect
				EndIf
			Next
		EndIf
		
		Return sb.ToString()
endfunction

function gbml_to_string_compressed:string(d:t_dict)
	local sb:tstringbuilder = new tstringbuilder
	If (d.values.length > 0)
			For Local i:Int = 0 To d.values.length-1
				If (d.values[i])
					Select d.values[i].id
						Case dictval_id_byte
							sb.append(d.keys[i] + ":@" + byte_get(d.values[i].tbyte) + ";")
						Case dictval_id_short
							sb.append(d.keys[i] + ":&" + short_get(d.values[i].tshort) + ";")
						Case dictval_id_int
							sb.append(d.keys[i] + ":%" + int_get(d.values[i].tint) + ";")
						Case dictval_id_long
							sb.append(d.keys[i] + ":^" + long_get(d.values[i].tlong) + ";")
						Case dictval_id_float
							sb.append(d.keys[i] + ":#" + float_get(d.values[i].tfloat) + ";")
						Case dictval_id_double
							sb.append(d.keys[i] + ":!" + double_get(d.values[i].tdouble) + ";")
						Case dictval_id_string
							sb.append(d.keys[i] + ":$" + string_get(d.values[i].tstring) + ";")
						Case dictval_id_dir
							Local g:t_dict = d.values[i].tdir
							sb.append(d.keys[i] + "{" + gbml_to_string_compressed(g) + "}")
					EndSelect
				EndIf
			Next
		EndIf
		Return sb.ToString()
endfunction

function gbml_from_string:t_dict(s:string)
	local dt:t_dict = new_dict()
	local l:int     = len(s)
	local c:string  = ""
	local w:string  = ""
	local hk:string	= null
	local ht:string	= null
	local hv:string = null
	local dl:tlist = createlist()
	local cd:t_dict = null
	
	listaddlast(dl,dt)
	
	if (l > 0)
		for local i:int = 0 to l-1
			cd = t_dict( dl.last() )
			c  = chr(s[i])
			
			select c
				case ";"
					if w and hk and ht and (not hv)
						hv = trim(w)
						w = ""
					endif
					if hk and ht and hv
						select ht
							case "@"
								dict_set_byte(cd, hk, byte(hv))
							case "&"
								dict_set_short(cd, hk, short(hv))
							case "%"
								dict_set_int(cd, hk, int(hv))
							case "^"
								dict_set_long(cd, hk, long(hv))
							case "#"
								dict_set_float(cd, hk, float(hv))
							case "!"
								dict_set_double(cd, hk, double(hv))
							case "$"
								dict_set_string(cd, hk, string(hv))
						endselect
					endif
					hk = null
					ht = null
					hv = null
				
				case ":"
					if not hk
						if w
							hk = trim(w)
						endif
						w = ""
					elseif hk and ht
						w = w + c
					endif
				
				case "{"
					if w and (not hk)
						hk = trim(w)
						w = ""
					endif
					if hk
						dict_set_dir( cd, hk )
						local dc:t_dict = dict_get_dir(cd,hk)
						dl.addlast(dc)
					else
						dict_set_dir( cd, "" )
						local dc:t_dict = dict_get_dir(cd,"")
						dl.addlast(dc)
					endif
					hk = null
					ht = null
					hv = null
				
				case "@","&","%","^","#","!","$"
					if hk and (not ht)
						ht = c
					else
						w = w + c
					endif
				
				default
					w = w + c
			endselect 
			
		next
	endif
	return dt
endfunction

