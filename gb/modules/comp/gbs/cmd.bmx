
''''''''''
'' type ''
''''''''''

type t_gbscmd
	field name      :string
	field cmd       :string
	field aliases   :string[]
	field desc      :string
	field func      (args:string[])
  field cmd_true  :t_gbscmd
  field cmd_false :t_gbscmd
endtype

function new_gbscmd:t_gbscmd()
	local r:t_gbscmd = new t_gbscmd
	r.name 			= ""
	r.cmd 			= ""
	r.aliases 	= new string[0]
	r.desc			= ""
	r.func 			= null
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gbscmd_set_name(c:t_gbscmd, s:string)
	c.name = s
endfunction

function gbscmd_set_cmd(c:t_gbscmd, s:string)
	c.cmd = s
endfunction

function gbscmd_set_aliases(c:t_gbscmd, s:string[])
	c.aliases = s
endfunction

function gbscmd_set_desc(c:t_gbscmd, s:string)
	c.desc = s
endfunction

function gbscmd_set_func(c:t_gbscmd, f(args:string[]))
	c.func = f
endfunction

function gbscmd_execute(c:t_gbscmd, s:string[])
	if c.func then c.func(s)
endfunction

''''''''''''''
'' commands ''
''''''''''''''

function new_gbscmd_clear:t_gbscmd()
  local r:t_gbscmd = new_gbscmd()
  r.name = "clear"
  r.cmd = "clear"
  r.aliases = ["cls"]
  r.desc = "clear the console"
  r.func = gbs_func_clear
  return r
endfunction

function gbs_func_clear(s:string[])
  gb.console.lines			= new t_string[23]
  for local i:int = 0 to 22
		gb.console.lines[i] = new_string("")
	next
endfunction

'''

function new_gbscmd_set:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "set"
	r.cmd = "set"
	r.aliases = []
	r.desc = "set var [0] to [1]"
	r.func = gbs_func_set
	return r
endfunction

function gbs_func_set(s:string[])
	local u:t_dictval = new_dictval()
	dictval_eval(u,sind(s,1))
	dict_set_val(gb.gbs.data,sind(s,0),u)
endfunction

'''

function new_gbscmd_add:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "add"
	r.cmd = "add"
	r.aliases = []
	r.desc = "add [1] to var [0]"
	r.func = gbs_func_add
	return r
endfunction

function gbs_func_add(s:string[])
endfunction

'''

function new_gbscmd_mul:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "mul"
	r.cmd = "mul"
	r.aliases = []
	r.desc = "multiply var [0] by [1]"
	r.func = gbs_func_mul
	return r
endfunction

function gbs_func_mul(s:string[])
endfunction

'''

function new_gbscmd_div:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "div"
	r.cmd = "div"
	r.aliases = []
	r.desc = "divide var [0] by [1]"
	r.func = gbs_func_div
	return r
endfunction

function gbs_func_div(s:string[])
endfunction

'''

function new_gbscmd_write:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "write"
	r.cmd = "write"
	r.aliases = []
	r.desc = "write [1] to path [0]"
	r.func = gbs_func_write
	return r
endfunction

function gbs_func_write(s:string[])
	local u:t_dictval = new_dictval()
	dictval_eval(u,sind(s,1))
	dict_write_val(gb.gbs.data,sind(s,0),u)
endfunction

'''

function new_gbscmd_read:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "read"
	r.cmd = "read"
	r.aliases = []
	r.desc = "read from path [0] into [1]"
	r.func = gbs_func_read
	return r
endfunction

function gbs_func_read(s:string[])
endfunction

'''

function new_gbscmd_save:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "save"
	r.cmd = "save"
	r.aliases = []
	r.desc = "save gbs session data"
	r.func = gbs_func_save
	return r
endfunction

function gbs_func_save(s:string[])
	gbml_save(gb.gbs.data,"gbs")
endfunction

'''

function new_gbscmd_load:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "load"
	r.cmd = "load"
	r.aliases = []
	r.desc = "load gbs session data"
	r.func = gbs_func_load
	return r
endfunction

function gbs_func_load(s:string[])
	gb.gbs.data = gbml_load("gbs")
endfunction

'''

function new_gbscmd_reloadimages:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "reloadimages"
	r.cmd = "reloadimages"
	r.aliases = ["rli"]
	r.desc = "reload images from files"
	r.func = gbs_func_reloadimages
	return r
endfunction

function gbs_func_reloadimages(s:string[])
	gb_assets_reload_images()
endfunction

'''

function new_gbscmd_reloadsounds:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "reloadsounds"
	r.cmd = "reloadsounds"
	r.aliases = ["rls"]
	r.desc = "reload sounds from files"
	r.func = gbs_func_reloadsounds
	return r
endfunction

function gbs_func_reloadsounds(s:string[])
	gb_assets_reload_sounds()
endfunction

'''

function new_gbscmd_listimages:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "listimages"
	r.cmd = "listimages"
	r.aliases = ["li"]
	r.desc = "list loaded images by index"
	r.func = gbs_func_listimages
	return r
endfunction

function gbs_func_listimages(s:string[])
	for local i:int = 0 to gb_assets_max_images-1
		if gb.assets.images[i]
			gb_console_out(i + ": " + gb.assets.images[i].path)
		endif
	next
endfunction

'''

function new_gbscmd_listsounds:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "listsounds"
	r.cmd = "listsounds"
	r.aliases = ["ls"]
	r.desc = "list loaded sounds by index"
	r.func = gbs_func_listsounds
	return r
endfunction

function gbs_func_listsounds(s:string[])
	for local i:int = 0 to gb_assets_max_sounds-1
		if gb.assets.sounds[i]
			gb_console_out(i + ": " + gb.assets.sounds[i].path)
		endif
	next
endfunction

'''

function new_gbscmd_data:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "data"
	r.cmd = "data"
	r.aliases = []
	r.desc = "output GBML-formatted session data"
	r.func = gbs_func_data
	return r
endfunction

function gbs_func_data(s:string[])
	gb_console_out(gbml_to_string_compressed(gb.gbs.data))
endfunction

'''

function new_gbscmd_sdata:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "sdata"
	r.cmd = "sdata"
	r.aliases = []
	r.desc = "output GBML-formatted settings data"
	r.func = gbs_func_sdata
	return r
endfunction

function gbs_func_sdata(s:string[])
	gb_console_out(gbml_to_string_compressed(gb.settings))
endfunction

'''

function new_gbscmd_playsound:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "playsound"
	r.cmd = "playsound"
	r.aliases = ["snd"]
	r.desc = "play sound [0]"
	r.func = gbs_func_playsound
	return r
endfunction

function gbs_func_playsound(s:string[])
	local u:int = int(sind(s,0))
	gb_audio_play_sound(u)
	gb_console_out("PLAYING [" + u + "] - " + gb.assets.sounds[u].path)
endfunction

'''

function new_gbscmd_help:t_gbscmd()
	local r:t_gbscmd = new_gbscmd()
	r.name = "help"
	r.cmd = "help"
	r.aliases = ["?"]
	r.desc = "list all loaded commands"
	r.func = gbs_func_help
	return r
endfunction

function gbs_func_help(s:string[])
	local t:string
	for local i:int = 0 to gbs_max_cmds-1
		if gb.gbs.cmds[i]
			t = gb.gbs.cmds[i].cmd
			for local u:string = eachin gb.gbs.cmds[i].aliases
				t = t + "/" + u
			next
			t = t + ": " + gb.gbs.cmds[i].desc
			gb_console_out(t)
		endif
	next
endfunction

