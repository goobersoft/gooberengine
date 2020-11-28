
type t_bitmap
	field width		:int
	field height	:int
	field data		:t_color[,]
	field depth		:float[,]
	field image		:timage
endtype

function new_bitmap:t_bitmap(x:int, y:int, rr:float=0, gg:float=0, bb:float=0, aa:float=0)
	local r:t_bitmap = new t_bitmap
	r.width 		= low(x,10)
	r.height 		= low(y,10)
	r.data 			= new t_color[r.width, r.height]
	r.depth			= new float[r.width, r.height]
	for local i:int = 0 to r.width-1
		for local j:int = 0 to r.height-1
			r.data[i,j] = new_color(rr,gg,bb,aa)
		next
	next
	r.image = createimage(r.width, r.height, 1, DYNAMICIMAGE)
	return r
endfunction

function bitmap_from_image:t_bitmap( u:t_image )
	local px :int = u.size.x
	local py :int = u.size.y
	
	local r:t_bitmap = new_bitmap(px,py)
	for local i:int = 0 to px-1
		for local j:int = 0 to py-1
      r.data[i,j] = new_color()
		next
	next
	
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function bitmap_sync(c:t_bitmap)
	local u:tpixmap = lockimage(c.image)
	local x:int = 0
	for local i:int = 0 to c.width-1
		for local j:int = 0 to c.height-1
			x = color_to_argb(c.data[i,j])
			writepixel(u,i,j,x)
		next
	next
endfunction

function bitmap_cls(c:t_bitmap)
	for local i:int = 0 to c.width-1
		for local j:int = 0 to c.height-1
			color_set(c.data[i,j], 0, 0, 0, 0)
		next
	next
endfunction

function bitmap_draw_dot(c:t_bitmap, x:int, y:int, rr:float, gg:float, bb:float, aa:float=3)
	x = iclamp(x,0,c.width-1)
	y = iclamp(y,0,c.height-1)
	rr = clamp(rr,0,3)
	gg = clamp(gg,0,3)
	bb = clamp(bb,0,3)
	aa = clamp(aa,0,3)
	color_set(c.data[x,y], rr, gg, bb, aa)
endfunction

function bitmap_draw_dot(c:t_bitmap, x:int, y:int, d:t_color)
	x = iclamp(x,0,c.width-1)
	y = iclamp(y,0,c.height-1)
	color_set(c.data[x,y], d)
endfunction

function bitmap_draw_rect( c:t_bitmap, x:int, y:int, w:int, h:int, d:t_color)
	x = iclamp(x,0,c.width-1)
	y = iclamp(y,0,c.height-1)
	w = iclamp(w,0,c.width-x)
	h = iclamp(h,0,c.height-y)
	for local i:int = 0 to w-1
		for local j:int = 0 to h-1
			color_set( c.data[i,j], d )
		next
	next
endfunction

function bitmap_paste( c:t_bitmap, x:int, y:int, d:t_bitmap )
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
				color_set( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function bitmap_paste_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width-1)
	dy = iclamp(dy,0,d.height-1)
	dw = iclamp(dw,1,d.width-dx)
	dh = iclamp(dh,1,d.height-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width, c.height )
				color_set( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_alpha( c:t_bitmap, x:int, y:int, d:t_bitmap )
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
				color_blend_alpha_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function bitmap_blend_alpha_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width-1)
	dy = iclamp(dy,0,d.height-1)
	dw = iclamp(dw,1,d.width-dx)
	dh = iclamp(dh,1,d.height-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width, c.height )
				color_blend_alpha_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_add( c:t_bitmap, x:int, y:int, d:t_bitmap )
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
				color_blend_add_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function bitmap_blend_add_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width-1)
	dy = iclamp(dy,0,d.height-1)
	dw = iclamp(dw,1,d.width-dx)
	dh = iclamp(dh,1,d.height-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width, c.height )
				color_blend_add_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_multiply( c:t_bitmap, x:int, y:int, d:t_bitmap )
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
				color_blend_multiply_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function bitmap_blend_multiply_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width-1)
	dy = iclamp(dy,0,d.height-1)
	dw = iclamp(dw,1,d.width-dx)
	dh = iclamp(dh,1,d.height-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width, c.height )
				color_blend_multiply_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_sub( c:t_bitmap, x:int, y:int, d:t_bitmap )
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
				color_blend_sub_c( c.data[x+i, y+j], d.data[i,j])
			endif
		next
	next
endfunction

function bitmap_blend_sub_part( c:t_bitmap, x:int, y:int, d:t_bitmap,
dx:int, dy:int, dw:int, dh:int )
	dx = iclamp(dx,0,d.width-1)
	dy = iclamp(dy,0,d.height-1)
	dw = iclamp(dw,1,d.width-dx)
	dh = iclamp(dh,1,d.height-dy)
	for local i:int = 0 to dw-1
		for local j:int = 0 to dh-1
			if ipoint_in_irect( x+i, y+j, 0, 0, c.width, c.height )
				color_blend_sub_c( c.data[x+i, y+j], d.data[i+dx, j+dy] )
			endif
		next
	next
endfunction

function bitmap_blend_custom( c:t_bitmap, x:int, y:int, d:t_bitmap,
f(c:t_color, d:t_color))
	local ux:int = d.width
	local uy:int = d.height
	for local i:int = 0 to ux-1
		for local j:int = 0 to uy-1
			if ipoint_in_irect(x+i, y+j, 0, 0, c.width, c.height)
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