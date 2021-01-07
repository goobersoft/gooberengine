
''''''''''
'' type ''
''''''''''

type t_gb_testing
  field lightcube :t_lightcube
  field timer     :t_timer
  field list      :t_list
  field jake      :t_jake
  field model     :t_model
endtype

function new_gb_testing:t_gb_testing ()
  local r:t_gb_testing = new t_gb_testing

  r.lightcube = new_lightcube()
  r.timer     = new_timer(30)
  r.list      = new_list()
  r.jake      = new_jake()
  r.model     = new_model_cube(20,12,0,5)
  

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
endfunction

function gb_testing_update()
  lightcube_update(gb.testing.lightcube)
  timer_update(gb.testing.timer)
  jake_update(gb.testing.jake)
  if timer_finished(gb.testing.timer) or keyhit(key_space)
    timer_reset(gb.testing.timer)
    lightcube_reset(gb.testing.lightcube)
    gb.testing.model = new_model_cube(17.5, 9.5, 0, 5)
  endif
  point3_add(gb.testing.model.tris[rand(0,11)].points[rand(0,2)], frand(-0.1,0.1), frand(-0.1,0.1), frand(-0.1,0.1))


endfunction

function gb_testing_draw()
  lightcube_draw(gb.testing.lightcube, 20, 12)
  gb_graph_draw_tile_text(39,23,1,int(timer_value(gb.testing.timer)))
  jake_draw(gb.testing.jake)
  model_draw_wire3d(gb.testing.model)
endfunction

function gb_testing_end()
endfunction