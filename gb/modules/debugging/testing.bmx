

'''''''''''''
'' globals ''
'''''''''''''

''''''''''''
'' Events ''
''''''''''''

function gb_testing_init()
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
  
endfunction

function gb_testing_draw()
endfunction

function gb_testing_end()
endfunction