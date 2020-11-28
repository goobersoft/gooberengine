
const gbs_max_cmds :int = 100

'''''''''''''
'' globals ''
'''''''''''''

global gbs_data 			:t_dict
global gbs_input 			:string[]
global gbs_cmds				:t_gbscmd[]

'''''''''''''''
'' functions ''
'''''''''''''''

function gbs_interpret(s:string, d:t_dict=null)
	gbs_input = tokenize(s)
	local l:int = len(gbs_input)
	local i0:string
	local iu:byte
	
	if l > 0
		i0 = lower(gbs_input[0])
		iu = false
		for local i:int = 0 to gbs_max_cmds-1
			if gbs_cmds[i]
				if gbs_cmds[i].cmd = i0
					iu = true
				else
					for local s:string = eachin gbs_cmds[i].aliases
						if s = i0
							iu = true
						endif
					next
				endif
				if iu = true
					gbscmd_execute( gbs_cmds[i], cdr(gbs_input) )
					return
				endif
			endif
		next
	endif
	
endfunction

''''''''''''''
'' commands ''
''''''''''''''

function new_gbscmd_set:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "set"
	r.cmd = "set"
	r.aliases = []
	r.desc = "set var [0] to [1]"
	r.func = gbs_func_set
	return r
endfunction

function new_gbscmd_add:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "add"
	r.cmd = "add"
	r.aliases = []
	r.desc = "add [1] to var [0]"
	r.func = gbs_func_add
	return r
endfunction

function new_gbscmd_mul:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "mul"
	r.cmd = "mul"
	r.aliases = []
	r.desc = "multiply var [0] by [1]"
	r.func = gbs_func_mul
	return r
endfunction

function new_gbscmd_div:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "div"
	r.cmd = "div"
	r.aliases = []
	r.desc = "divide var [0] by [1]"
	r.func = gbs_func_div
	return r
endfunction

function new_gbscmd_write:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "write"
	r.cmd = "write"
	r.aliases = []
	r.desc = "write [1] to path [0]"
	r.func = gbs_func_write
	return r
endfunction

function new_gbscmd_read:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "read"
	r.cmd = "read"
	r.aliases = []
	r.desc = "read from path [0] into [1]"
	r.func = gbs_func_read
	return r
endfunction

function new_gbscmd_save:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "save"
	r.cmd = "save"
	r.aliases = []
	r.desc = "save gbs session data"
	r.func = gbs_func_save
	return r
endfunction

function new_gbscmd_load:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "load"
	r.cmd = "load"
	r.aliases = []
	r.desc = "load gbs session data"
	r.func = gbs_func_load
	return r
endfunction

function new_gbscmd_reloadimages:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "reloadimages"
	r.cmd = "reloadimages"
	r.aliases = ["rli"]
	r.desc = "reload images from files"
	r.func = gbs_func_reloadimages
	return r
endfunction

function new_gbscmd_reloadsounds:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "reloadsounds"
	r.cmd = "reloadsounds"
	r.aliases = ["rls"]
	r.desc = "reload sounds from files"
	r.func = gbs_func_reloadsounds
	return r
endfunction

function new_gbscmd_listimages:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "listimages"
	r.cmd = "listimages"
	r.aliases = ["li"]
	r.desc = "list loaded images by index"
	r.func = gbs_func_listimages
	return r
endfunction

function new_gbscmd_listsounds:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "listsounds"
	r.cmd = "listsounds"
	r.aliases = ["ls"]
	r.desc = "list loaded sounds by index"
	r.func = gbs_func_listsounds
	return r
endfunction

function new_gbscmd_data:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "data"
	r.cmd = "data"
	r.aliases = []
	r.desc = "output GBML-formatted session data"
	r.func = gbs_func_data
	return r
endfunction

function new_gbscmd_sdata:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "sdata"
	r.cmd = "sdata"
	r.aliases = []
	r.desc = "output GBML-formatted settings data"
	r.func = gbs_func_sdata
	return r
endfunction

function new_gbscmd_playsound:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "playsound"
	r.cmd = "playsound"
	r.aliases = ["snd"]
	r.desc = "play sound [0]"
	r.func = gbs_func_playsound
	return r
endfunction

function new_gbscmd_help:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "help"
	r.cmd = "help"
	r.aliases = ["?"]
	r.desc = "list all loaded commands"
	r.func = gbs_func_help
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gbs_func_set(s:string[])
	local u:t_dictval = new_dictval()
	dictval_eval(u,sind(s,1))
	dict_set_val(gbs_data,sind(s,0),u)
endfunction

function gbs_func_add(s:string[])
endfunction

function gbs_func_mul(s:string[])
endfunction

function gbs_func_div(s:string[])
endfunction

function gbs_func_write(s:string[])
	local u:t_dictval = new_dictval()
	dictval_eval(u,sind(s,1))
	dict_write_val(gbs_data,sind(s,0),u)
endfunction

function gbs_func_read(s:string[])
endfunction

function gbs_func_save(s:string[])
	gbml_save(gbs_data,"gbs")
endfunction

function gbs_func_load(s:string[])
	gbs_data = gbml_load("gbs")
endfunction

function gbs_func_reloadimages(s:string[])
	gb_reload_images()
endfunction

function gbs_func_reloadsounds(s:string[])
	gb_reload_sounds()
endfunction

function gbs_func_listimages(s:string[])
	for local i:int = 0 to gb_max_images-1
		if gb_images[i]
			gb_console_out(i + ": " + gb_image_paths[i])
		endif
	next
endfunction

function gbs_func_listsounds(s:string[])
	for local i:int = 0 to gb_max_sounds-1
		if gb_sounds[i]
			gb_console_out(i + ": " + gb_sound_paths[i])
		endif
	next
endfunction

function gbs_func_data(s:string[])
	gb_console_out(gbml_to_string_compressed(gbs_data))
endfunction

function gbs_func_sdata(s:string[])
	gb_console_out(gbml_to_string_compressed(gb_settings))
endfunction

function gbs_func_playsound(s:string[])
	local u:int = int(sind(s,0))
	gb_audio_play_sound(u)
	gb_console_out("PLAYING [" + u + "] - " + gb_sound_paths[u])
endfunction

function gbs_func_help(s:string[])
	local t:string
	for local i:int = 0 to gbs_max_cmds-1
		if gbs_cmds[i]
			t = gbs_cmds[i].cmd
			for local u:string = eachin gbs_cmds[i].aliases
				t = t + "/" + u
			next
			t = t + ": " + gbs_cmds[i].desc
			gb_console_out(t)
		endif
	next
endfunction

''''''''''''
'' events ''
''''''''''''

function gbs_init()
	gbs_data 			= new_dict()
	gbs_cmds 		= new t_gbscmd[gbs_max_cmds]
	
	local u:t_counter = new_counter()
	gbs_cmds[counter_inc(u)] = new_gbscmd_set()
	gbs_cmds[counter_inc(u)] = new_gbscmd_add()
	gbs_cmds[counter_inc(u)] = new_gbscmd_mul()
	gbs_cmds[counter_inc(u)] = new_gbscmd_div()
	gbs_cmds[counter_inc(u)] = new_gbscmd_write()
	gbs_cmds[counter_inc(u)] = new_gbscmd_read()
	gbs_cmds[counter_inc(u)] = new_gbscmd_save()
	gbs_cmds[counter_inc(u)] = new_gbscmd_load()
	gbs_cmds[counter_inc(u)] = new_gbscmd_reloadimages()
	gbs_cmds[counter_inc(u)] = new_gbscmd_reloadsounds()
	gbs_cmds[counter_inc(u)] = new_gbscmd_listimages()
	gbs_cmds[counter_inc(u)] = new_gbscmd_listsounds()
	gbs_cmds[counter_inc(u)] = new_gbscmd_data()
	gbs_cmds[counter_inc(u)] = new_gbscmd_sdata()
	gbs_cmds[counter_inc(u)] = new_gbscmd_playsound()
	gbs_cmds[counter_inc(u)] = new_gbscmd_help()
endfunction