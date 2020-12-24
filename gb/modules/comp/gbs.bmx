

include "gbs/cmd.bmx"
include "gbs/var.bmx"
include "gbs/node.bmx"

''''''''''''
'' consts ''
''''''''''''

const gbs_max_cmds :int = 100

''''''''''
'' type ''
''''''''''

type t_gbs
  field data    :t_dict
  field input   :string[]
  field cmds    :t_gbscmd[]
endtype

function new_gbs:t_gbs ()
  local r:t_gbs = new t_gbs
  r.data  = new_dict()
  r.input = null
  r.cmds  = new t_gbscmd[ gbs_max_cmds ]
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gbs_init()
  gb.gbs = new_gbs()

	local u:t_counter = new_counter()
  gb.gbs.cmds[counter_inc(u)] = new_gbscmd_clear()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_set()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_add()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_mul()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_div()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_write()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_read()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_save()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_load()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_reloadimages()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_reloadsounds()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_listimages()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_listsounds()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_data()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_sdata()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_playsound()
	gb.gbs.cmds[counter_inc(u)] = new_gbscmd_help()
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gbs_interpret(s:string, d:t_dict=null)
	gb.gbs.input = tokenize(s)
	local l:int = len(gb.gbs.input)
	local i0:string
	local iu:byte
	
	if l > 0
		i0 = lower(gb.gbs.input[0])
		iu = false
		for local i:int = 0 to gbs_max_cmds-1
			if gb.gbs.cmds[i]
				if gb.gbs.cmds[i].cmd = i0
					iu = true
				else
					for local s:string = eachin gb.gbs.cmds[i].aliases
						if s = i0
							iu = true
						endif
					next
				endif
				if iu = true
					gbscmd_execute( gb.gbs.cmds[i], cdr(gb.gbs.input) )
					return
				endif
			endif
		next
	endif
endfunction

''''''''''''
'' events ''
''''''''''''