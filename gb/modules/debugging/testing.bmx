

''''''''''
'' type ''
''''''''''

type t_gb_testing
  field gball :t_gball
endtype

function new_gb_testing:t_gb_testing ()
  local r:t_gb_testing = new t_gb_testing

  r.gball = new_gball()

  return r
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_testing_init()
	gb.testing = new_gb_testing()
endfunction

function gb_testing_load()
endfunction

function gb_testing_start()
  local c:t_color = new_color_random()
  for local i:int = 0 to 399
    for local j:int = 0 to 239
      color_set(c, float(j)/240.0*3,0,0,3)
      gb_canvas_plot(i,j,c)
    next
  next
  gb_canvas_sync()
endfunction

function gb_testing_update()
  gball_update( gb.testing.gball )
endfunction

function gb_testing_draw()
  gball_draw( gb.testing.gball, 10, 10 )
  gb_graph_draw_tile_text(0,23,1,"Make sure you shake it up real good with spacebar!");
endfunction

function gb_testing_end()
endfunction