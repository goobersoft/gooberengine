
''''''''''
'' type ''
''''''''''

type t_gb_editor
  field active :t_bool
endtype

'''''''''
'' new ''
'''''''''

function new_gb_editor:t_gb_editor ()
  local r:t_gb_editor = new t_gb_editor
  r.active = new_bool(false)
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_editor_init()
  gb.editor = new_gb_editor()
endfunction

function gb_editor_load()
endfunction

function gb_editor_update()
endfunction

function gb_editor_draw()
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_editor_set_active(g:t_gb_editor, n:int)
  bool_set(gb.editor.active, n)
endfunction