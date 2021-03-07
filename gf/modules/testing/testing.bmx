

'''''''''''''
'' globals ''
'''''''''''''

global gf_testing_gball       :t_gball
global gf_testing_redskull    :t_redskull
global gf_testing_lightcube   :t_lightcube

''''''''''''
'' events ''
''''''''''''

function gf_testing_init()
  gf_testing_gball = new_gball()
  gf_testing_redskull = new_redskull()
  gf_testing_lightcube = new_lightcube()
endfunction

function gf_testing_load()
endfunction

function gf_testing_start()
  point_set(gf_testing_redskull.pos, 30, 15)
endfunction

function gf_testing_update()
  gball_update( gf_testing_gball )
  redskull_update( gf_testing_redskull )
  lightcube_update( gf_testing_lightcube )
endfunction

function gf_testing_draw()
  lightcube_draw( gf_testing_lightcube, 20, 12 )
  gball_draw( gf_testing_gball, 10, 10 )
  gb_graph_draw_tile_text(0,23,1,"Make sure you shake it up real good with spacebar!");
  redskull_draw( gf_testing_redskull )
endfunction

function gf_testing_end()
endfunction