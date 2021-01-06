
''  depth:
''    if 0, colors in the canvas are not rounded.
''    if 2 or more, colors are rounded based on intervals
''      (4 = 4-tone color, 16 'bits' total) 

type t_canvas
	field width		  :t_int
	field height	  :t_int
	field data		  :t_color[,]
  field coldepth  :t_int
  field palette   :t_palette
	field image		  :timage
endtype

function new_canvas:t_canvas(x:int, y:int, rr:float=0, gg:float=0, bb:float=0, aa:float=0)
	local r:t_canvas = new t_canvas
	r.width 		= new_int(ilow(x,10))
	r.height 		= new_int(ilow(y,10))
	r.data 			= new t_color[r.width.value, r.height.value]
	for local i:int = 0 to r.width.value-1
		for local j:int = 0 to r.height.value-1
			r.data[i,j] = new_color(rr,gg,bb,aa)
		next
	next
	r.image     = createimage(r.width.value, r.height.value, 1, DYNAMICIMAGE)
  r.palette   = new_palette()
  r.coldepth  = new_int(0)
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
      r.data[i,j] = new_color(rr,gg,bb,aa)
		next
	next
	r.image     = createimage(r.width.value, r.height.value, 1, DYNAMICIMAGE)
  r.palette   = new_palette()
  canvas_sync(r)
	return r
endfunction

function new_canvas_from_path:t_canvas ( p:string )
  return new_canvas_from_image(new_image(p))
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function canvas_sync(c:t_canvas)
	local u:tpixmap = lockimage(c.image)
	local x:int = 0
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			x = color_to_argb(c.data[i,j])
			writepixel(u,i,j,x)
		next
	next
endfunction

function canvas_cls(c:t_canvas)
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			color_set(c.data[i,j], 0, 0, 0, 0)
		next
	next
endfunction

'''''''''''''''''''''''
'' drawing functions ''
'''''''''''''''''''''''

function canvas_draw_dot(c:t_canvas, x:int, y:int, rr:float, gg:float, bb:float, aa:float=3)
	x = iwrap(x,0,c.width.value-1)
	y = iwrap(y,0,c.height.value-1)
  if in_irange( x, 0, c.width.value ) and in_irange( y, 0, c.height.value )
    color_set(c.data[x,y], rr, gg, bb, aa)
  endif
endfunction

function canvas_draw_dot_c(c:t_canvas, x:int, y:int, d:t_color)
	x = iwrap(x,0,c.width.value-1)
	y = iwrap(y,0,c.height.value-1)
  if in_irange( x, 0, c.width.value ) and in_irange( y, 0, c.height.value )
	  color_set_c(c.data[x,y], d)
  endif
endfunction

function canvas_draw_rect_c( c:t_canvas, x:int, y:int, w:int, h:int, d:t_color)
	x = iwrap(x,0,c.width.value-1)
	y = iwrap(y,0,c.height.value-1)
	w = iclamp(w,0,c.width.value-x)
	h = iclamp(h,0,c.height.value-y)
	for local i:int = 0 to w-1
		for local j:int = 0 to h-1
      if in_irange( x+i, 0, c.width.value ) and in_irange( y+j, 0, c.height.value )
			  color_set_c( c.data[x+i,y+j], d )
      endif
		next
	next
endfunction

'''''''''''''''''''''''
'' blending commands ''
'''''''''''''''''''''''

function canvas_blend( c:t_canvas, x:int, y:int, d:t_canvas )
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_set_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function canvas_blend_part( c:t_canvas, x:int, y:int, d:t_canvas,
dx:int, dy:int, dw:int, dh:int )
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_set_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function canvas_blend_alpha( c:t_canvas, x:int, y:int, d:t_canvas )
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_blend_alpha_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function canvas_blend_alpha_part( c:t_canvas, x:int, y:int, d:t_canvas,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width.value-1)
	dy = iclamp(dy,0,d.height.value-1)
	dw = iclamp(dw,1,d.width.value-dx)
	dh = iclamp(dh,1,d.height.value-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_blend_alpha_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function canvas_blend_add( c:t_canvas, x:int, y:int, d:t_canvas )
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_blend_add_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function canvas_blend_add_part( c:t_canvas, x:int, y:int, d:t_canvas,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width.value-1)
	dy = iclamp(dy,0,d.height.value-1)
	dw = iclamp(dw,1,d.width.value-dx)
	dh = iclamp(dh,1,d.height.value-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_blend_add_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function canvas_blend_multiply( c:t_canvas, x:int, y:int, d:t_canvas )
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_blend_multiply_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function canvas_blend_multiply_part( c:t_canvas, x:int, y:int, d:t_canvas,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width.value-1)
	dy = iclamp(dy,0,d.height.value-1)
	dw = iclamp(dw,1,d.width.value-dx)
	dh = iclamp(dh,1,d.height.value-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_blend_multiply_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function canvas_blend_sub( c:t_canvas, x:int, y:int, d:t_canvas )
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_blend_sub_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function canvas_blend_sub_part( c:t_canvas, x:int, y:int, d:t_canvas,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width.value-1)
	dy = iclamp(dy,0,d.height.value-1)
	dw = iclamp(dw,1,d.width.value-dx)
	dh = iclamp(dh,1,d.height.value-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_blend_sub_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function canvas_blend_custom( c:t_canvas, x:int, y:int, d:t_canvas,
f(c:t_color, d:t_color))
	local ux:int = d.width.value
	local uy:int = d.height.value
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width.value, c.height.value)
				color_blend_custom( c.data[x+i, y+j], d.data[i,j], f)
			endif
		next
	next
endfunction


''''''''''''
'' events ''
''''''''''''

function canvas_draw( c:t_canvas, x:float=0, y:float=0 )
	gb_graph_draw_image( x, y, c.image )
endfunction