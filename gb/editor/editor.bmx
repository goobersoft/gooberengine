
'''''''''''''
'' globals ''
'''''''''''''

global gb_editor_active :t_bool

''''''''''''
'' events ''
''''''''''''

function gb_editor_init()
	gb_editor_active = new_bool(false)
endfunction