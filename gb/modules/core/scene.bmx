
''''''''''
'' type ''
''''''''''

type t_scene
  field uptime      :t_float
  field fn_init     ()
  field fn_update   ()
  field fn_draw     ()
  field fn_end      ()
  field data        :object
endtype

function new_scene:t_scene ()
  local r:t_scene = new t_scene
  r.uptime      = new_float(0.0)
  r.fn_init     = empty
  r.fn_update   = empty
  r.fn_draw     = empty
  r.fn_end      = empty
  r.data        = null
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function scene_init(s:t_scene)
endfunction

function scene_update(s:t_scene)
  local d:float   = gb_delta()
endfunction

function scene_draw(s:t_scene)
endfunction

function scene_end(s:t_scene)
endfunction

function scene_data:object (s:t_scene)
  return s.data
endfunction