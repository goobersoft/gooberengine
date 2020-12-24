
''''''''''
'' type ''
''''''''''

type t_gb_testing

endtype

function new_gb_testing:t_gb_testing ()
  local r:t_gb_testing = new t_gb_testing

  return r
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_testing_init()
	gb.testing              = new_gb_testing()
endfunction

function gb_testing_load()
endfunction

function gb_testing_start()
endfunction

function gb_testing_update()
endfunction

function gb_testing_draw()
  gb_graph_draw_image(0,0,1)
endfunction

function gb_testing_end()
endfunction