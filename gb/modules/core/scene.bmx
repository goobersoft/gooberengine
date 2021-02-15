
''''''''''
'' type ''
''''''''''

type t_scene
  field uptime      :t_float
  field fn_init     (s:t_scene)
  field fn_update   (s:t_scene)
  field fn_draw     (s:t_scene)
  field fn_end      (s:t_scene)
  field data        :object
endtype

function new_scene:t_scene ()
  local r:t_scene = new t_scene
  r.uptime      = new_float(0.0)
  r.fn_init     = null
  r.fn_update   = null
  r.fn_draw     = null
  r.fn_end      = null
  r.data        = null
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function scene_switch:t_scene(s:t_scene=null, g:t_scene=null)
  if (not s) and (not g)
    return null
  elseif (not s) and (g)
    scene_init(g)
    return g
  elseif (s) and (not g)
    scene_init(s)
    return s
  elseif (s) and (g)
    float_set(s.uptime, 0)
    scene_end(s)
    s.fn_init   = g.fn_init
    s.fn_update = g.fn_update
    s.fn_draw   = g.fn_draw
    s.fn_end    = g.fn_end
    s.data      = g.data
    scene_init(s)
    return s
  endif
endfunction

function scene_init(s:t_scene)
  if s and s.fn_init
    s.fn_init(s)
  endif
endfunction

function scene_update(s:t_scene)
  if s then float_add( s.uptime, gb_delta() )
  if s and s.fn_update
    s.fn_update(s)
  endif
endfunction

function scene_draw(s:t_scene)
  if s and s.fn_draw
    s.fn_draw(s)
  endif
endfunction

function scene_end(s:t_scene)
  if s and s.fn_end
    s.fn_end(s)
  endif
endfunction

function scene_data:object (s:t_scene)
  return s.data
endfunction