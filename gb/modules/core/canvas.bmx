



''''''''''
'' type ''
''''''''''

type t_canvas
	field width		    :t_int
	field height	    :t_int
	field pixels		  :t_color[][]
  field depths      :t_float[][]
  field coldepth    :t_float
  field palette     :t_palette
	field image		    :timage
  method get:timage()
    return image
  endmethod
endtype

'''''''''
'' new ''
'''''''''

function new_canvas:t_canvas(x:int, y:int, rr:float=0, gg:float=0, bb:float=0, aa:float=0)
	local r:t_canvas = new t_canvas
	r.width 		= new_int(ilow(x,10))
	r.height 		= new_int(ilow(y,10))
	r.pixels 		= new t_color[r.width.value][]
  r.depths    = new t_float[r.width.value][]
	for local i:int = 0 to r.width.value-1
    r.pixels[i] = new t_color[r.height.value]
    r.depths[i] = new t_float[r.height.value]
		for local j:int = 0 to r.height.value-1
			r.pixels[i][j] = new_color(rr,gg,bb,aa)
      r.depths[i][j] = new_float(0)
		next
	next
	r.image     = createimage(r.width.value, r.height.value, 1, DYNAMICIMAGE)
  r.palette   = new_palette()
  r.coldepth  = new_float(3)
  canvas_sync(r)
	return r
endfunction

function new_canvas_from_image:t_canvas( u:t_image )
	local px :int     = image_width(u)
	local py :int     = image_height(u)
  local pp :tpixmap = lockimage(u.source)
  local ii :int     = 0
  local aa :float   = 0
  local rr :float   = 0
  local gg :float   = 0
  local bb :float   = 0
	
	local r:t_canvas = new_canvas(px,py)
	for local i:int = 0 to px-1
		for local j:int = 0 to py-1
      ii = readpixel(pp,i,j)
      aa = float(argb_a(ii)) / 255 * 3
      rr = float(argb_r(ii)) / 255 * 3
      gg = float(argb_g(ii)) / 255 * 3
      bb = float(argb_b(ii)) / 255 * 3
      r.pixels[i][j] = new_color(rr,gg,bb,aa)
		next
	next
	r.image     = createimage(r.width.value, r.height.value, 1, DYNAMICIMAGE)
  r.palette   = new_palette()
  
  '' If you don't sync this at least once, the new image will have garbage pixels.
  canvas_sync(r)

	return r
endfunction

function new_canvas_from_path:t_canvas ( p:string )
  return new_canvas_from_image(new_image(p))
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function canvas_get_pixel:t_color(c:t_canvas, x:int, y:int)
  if ipoint_in_irect( x, y, 0, 0, c.width.get(), c.height.get() )
    return c.pixels[x][y]
  endif
endfunction

function canvas_sync(c:t_canvas)
	local u:tpixmap = lockimage(c.image)
	local x:int = 0
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			x = color_to_argb(c.pixels[i][j])
			writepixel(u,i,j,x)
		next
	next
endfunction

function canvas_cls(c:t_canvas)
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			color_set(c.pixels[i][j], 0, 0, 0, 0)
      float_set(c.depths[i][j], 0)
		next
	next
endfunction

function canvas_set_palette()
endfunction


'''''''''''''''''''''''
'' drawing functions ''
'''''''''''''''''''''''

function canvas_plot(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_set(c.pixels[x][y], d)
  endif
endfunction

function canvas_plot_alpha(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_blend_alpha(c.pixels[x][y], d)
  endif
endfunction

function canvas_plot_add(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_blend_add(c.pixels[x][y], d)
  endif
endfunction

function canvas_plot_sub(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_blend_sub(c.pixels[x][y], d)
  endif
endfunction

function canvas_plot_mul(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_blend_mul(c.pixels[x][y], d)
  endif
endfunction

function canvas_plot_div(c:t_canvas, x:int, y:int, d:t_color)
  if ipoint_in_irect(x,y,0,0,c.width.get(),c.height.get())
    color_blend_div(c.pixels[x][y], d)
  endif
endfunction

''''''''''''
'' events ''
''''''''''''

function canvas_draw( c:t_canvas, x:float=0, y:float=0 )
	gb_graph_draw_image( x, y, c.image )
endfunction

''''''''''''''''''''''''
'' saving screenshots ''
''''''''''''''''''''''''

function canvas_save_screenshot()
endfunction