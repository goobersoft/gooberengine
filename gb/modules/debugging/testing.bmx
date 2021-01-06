
''''''''''
'' type ''
''''''''''

type t_gb_testing
  field lightcube :t_lightcube
  field timer     :t_timer
  field list      :t_list
  field jake      :t_jake
  field mstring   :t_mstring
endtype

function new_gb_testing:t_gb_testing ()
  local r:t_gb_testing = new t_gb_testing

  r.lightcube = new_lightcube()
  r.timer     = new_timer(30)
  r.list      = new_list()
  r.jake      = new_jake()
  r.mstring   = new_mstring("hello there.")

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
  mstring_add(gb.testing.mstring, " This is another sentence.")
  print(mstring_to_string(gb.testing.mstring))
  print(mstring_length(gb.testing.mstring))

  gb.assets.fonts[-3] = new t_font
endfunction

function gb_testing_update()
  lightcube_update(gb.testing.lightcube)
  timer_update(gb.testing.timer)
  jake_update(gb.testing.jake)
  if timer_finished(gb.testing.timer) or keyhit(key_space)
    timer_reset(gb.testing.timer)
    lightcube_reset(gb.testing.lightcube)
  endif
endfunction

function gb_testing_draw()
  lightcube_draw(gb.testing.lightcube, 20, 12)
  gb_graph_draw_tile_text(39,23,1,int(timer_value(gb.testing.timer)))
  jake_draw(gb.testing.jake)
endfunction

function gb_testing_end()
endfunction