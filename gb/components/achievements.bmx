
global gb_achievements :t_dict

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_achievements_add_achievement(id:string, name:string)
	local u:t_dict = new_dict()
	dict_set_string(u,"id",id)
	dict_set_string(u,"name",name)
	dict_set_string(u,"desc","No Desc.")
	dict_set_byte(u,"fulfilled",false)
	dict_set_dir(gb_achievements,id,u)
endfunction

function gb_achievements_fulfill(id:string)
	local u:t_dictval = dict_find(gb_achievements,id)
	if (u)
		local v:t_dict = dictval_get_dir(u)
		dict_set_byte(v,"fulfilled",true)
	endif
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_achievements_init()
endfunction

function gb_achievements_load()
	gb_achievements = gbml_load("achievements")
endfunction

function gb_achievements_end()
	gbml_save(gb_achievements,"achievements")
endfunction