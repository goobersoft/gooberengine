
''''''''''
'' type ''
''''''''''

type t_gb_testing
endtype

''''''''''''
'' global ''
''''''''''''

global gb_testing :t_gb_testing

''''''''''''
'' Events ''
''''''''''''

function gb_testing_init()
	gb_testing              = new t_gb_testing
endfunction

function gb_testing_load()
endfunction

function gb_testing_start()
endfunction

function gb_testing_update(d:float)
endfunction

function gb_testing_draw(x:float=0, y:float=0)
  gb_graph_draw_image(0,0,1)
endfunction

function gb_testing_end()
endfunction