
''''''''''
'' type ''
''''''''''

type t_gb_editor
  field active      :t_bool
  field lightcube   :t_lightcube
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
  if bool_eq(gb.editor.active)
    
  endif
endfunction

function gb_editor_end()
  gb.editor = null
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_editor_set_active(n:int)
  bool_set(gb.editor.active, n)
endfunction