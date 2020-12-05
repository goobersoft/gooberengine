
''''''''''
'' type ''
''''''''''

type t_gbscmd
	field name      :string
	field cmd       :string
	field aliases   :string[]
	field desc      :string
	field func      (args:string[])
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