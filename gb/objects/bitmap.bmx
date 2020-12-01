
''  depth:
''    if 0, colors in the bitmap are not rounded.
''    if 2 or more, colors are rounded based on intervals
''      (4 = 4-tone color, 16 'bits' total) 

type t_bitmap
	field width		  :t_int
	field height	  :t_int
	field data		  :t_color[,]
  field coldepth  :t_int
  field palette   :t_palette
	field image		  :timage
endtype

function new_bitmap:t_bitmap(x:int, y:int, rr:float=0, gg:float=0, bb:float=0, aa:float=0)
	local r:t_bitmap = new t_bitmap
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

function new_bitmap_from_image:t_bitmap( u:t_image )
	local px :int     = image_width(u)
	local py :int     = image_height(u)
  local pp :tpixmap = lockimage(u.image)
  local ii :int     = 0
  local aa :float   = 0
  local rr :float   = 0
  local gg :float   = 0
  local bb :float   = 0
	
	local r:t_bitmap = new_bitmap(px,py)
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
  bitmap_sync(r)
	return r
endfunction

function new_bitmap_from_path:t_bitmap ( p:string )
  return new_bitmap_from_image(new_image(p))
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function bitmap_sync(c:t_bitmap)
	local u:tpixmap = lockimage(c.image)
	local x:int = 0
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			x = color_to_argb(c.data[i,j])
			writepixel(u,i,j,x)
		next
	next
endfunction

function bitmap_cls(c:t_bitmap)
	for local i:int = 0 to c.width.value-1
		for local j:int = 0 to c.height.value-1
			color_set(c.data[i,j], 0, 0, 0, 0)
		next
	next
endfunction

'''''''''''''''''''''''
'' drawing functions ''
'''''''''''''''''''''''

function bitmap_draw_dot(c:t_bitmap, x:int, y:int, rr:float, gg:float, bb:float, aa:float=3)
	x = iwrap(x,0,c.width.value-1)
	y = iwrap(y,0,c.height.value-1)
  if in_irange( x, 0, c.width.value ) and in_irange( y, 0, c.height.value )
    color_set(c.data[x,y], rr, gg, bb, aa)
  endif
endfunction

function bitmap_draw_dot_c(c:t_bitmap, x:int, y:int, d:t_color)
	x = iwrap(x,0,c.width.value-1)
	y = iwrap(y,0,c.height.value-1)
  if in_irange( x, 0, c.width.value ) and in_irange( y, 0, c.height.value )
	  color_set_c(c.data[x,y], d)
  endif
endfunction

function bitmap_draw_rect_c( c:t_bitmap, x:int, y:int, w:int, h:int, d:t_color)
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

function bitmap_blend( c:t_bitmap, x:int, y:int, d:t_bitmap )
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

function bitmap_blend_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width.value, c.height.value )
				color_set_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_alpha( c:t_bitmap, x:int, y:int, d:t_bitmap )
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

function bitmap_blend_alpha_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
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

function bitmap_blend_add( c:t_bitmap, x:int, y:int, d:t_bitmap )
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

function bitmap_blend_add_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
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

function bitmap_blend_multiply( c:t_bitmap, x:int, y:int, d:t_bitmap )
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

function bitmap_blend_multiply_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
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

function bitmap_blend_sub( c:t_bitmap, x:int, y:int, d:t_bitmap )
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

function bitmap_blend_sub_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
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

function bitmap_blend_custom( c:t_bitmap, x:int, y:int, d:t_bitmap,
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

function bitmap_draw( c:t_bitmap, x:float=0, y:float=0 )
	gb_graph_draw_image( x, y, c.image )
endfunction