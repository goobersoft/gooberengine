
''''''''''''
'' consts ''
''''''''''''

const gb_graph_color_r :byte = 0
const gb_graph_color_g :byte = 1
const gb_graph_color_b :byte = 2
const gb_graph_color_a :byte = 3

const gb_graph_mode_normal 		:byte = 0
const gb_graph_mode_add 			:byte = 1
const gb_graph_mode_multiply 	:byte = 2

'''''''''''''
'' globals ''
'''''''''''''

global gb_graph_color 				:t_color
global gb_graph_scale 				:t_point
global gb_graph_handle				:t_point
global gb_graph_offset				:t_point
global gb_graph_clip					:t_rect
global gb_graph_angle					:t_number
global gb_graph_perspec				:t_number
global gb_graph_use_perspec		:t_bool
global gb_graph_mode					:t_byte
global gb_graph_pixelmode			:t_bool
global gb_graph_flip					:t_bool[]

global gb_graph_center				:t_point
global gb_graph_zrange				:t_number
global gb_graph_use_camera		:t_bool

global gb_graph_drawscale			:t_point




'' scanlines
global gb_graph_scanline_thickness 	:int
global gb_graph_scanline_style 			:int

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_graph_prepare()
	select(byte_get(gb_graph_mode))
		case gb_graph_mode_normal
			setblend(alphablend)
		case gb_graph_mode_add
			setblend(lightblend)
		case gb_graph_mode_multiply
			setblend(shadeblend)
		default
			setblend(alphablend)
	endselect

	setcolor( roundint(gb_graph_color.r*85), roundint(gb_graph_color.g*85),
		roundint(gb_graph_color.b*85))
	setalpha( gb_graph_color.a/3 )
	'setorigin( gb_graph_offset.x, gb_graph_offset.y )
	setrotation( wrap(gb_graph_angle.value-gb_camera.angle.z,0,1000) / 1000 * 360 )
	gb_graph_calc_viewport()
endfunction

function gb_graph_calc_viewport()
	local cx:float = (gb_graph_clip.w/40*gb_visual_letterbox_rect.w)
	local cy:float = (gb_graph_clip.h/24*gb_visual_letterbox_rect.h)
	setviewport( ..
		roundint(gb_visual_letterbox_rect.x), roundint(gb_visual_letterbox_rect.y),
		roundint(cx), roundint(cy))
endfunction

function gb_graph_calc_perspec:float( z:float )
	if bool_eq(gb_graph_pixelmode) then z = pixel(z)
	local u:float = number_get(gb_graph_perspec)
	if z > -u
		return (u / (z+u))
	endif
	return 0.0
endfunction

function gb_graph_calc_offsets()
	local u:float
	if bool_eq(gb_graph_use_camera)
		u = wrap(number_get(gb_graph_angle) - gb_camera.angle.z,0,1000)
	else
		u = number_get(gb_graph_angle)
	endif
	gb_graph_offset.x = (gb_graph_handle.x * cosf(u)) + (gb_graph_handle.y * -sinf(u))
	gb_graph_offset.y = (gb_graph_handle.y * sinf(u)) + (gb_graph_handle.y * cosf(u))
endfunction

function gb_graph_calc_drawscale()
	gb_graph_drawscale.x = float_get(gb_visual_pixelscale) * float_get(gb_visual_virtualscale)
	gb_graph_drawscale.y = float_get(gb_visual_pixelscale) * float_get(gb_visual_virtualscale)
endfunction

function gb_graph_calc_x:float(x:float)
	if bool_eq(gb_graph_use_camera) then x = x - gb_camera.pos.x
	if bool_eq(gb_graph_pixelmode) then x = pixel(x)
	return gb_visual_letterbox_rect.x + ((x-(gb_graph_offset.x)) * gb_graph_drawscale.x * gb_tilesize)
endfunction

function gb_graph_calc_y:float(y:float)
	if bool_eq(gb_graph_use_camera) then y = y - gb_camera.pos.y
	if bool_eq(gb_graph_pixelmode) then y = pixel(y)
	return gb_visual_letterbox_rect.y + ((y-(gb_graph_offset.y)) * gb_graph_drawscale.y * gb_tilesize)
endfunction

function gb_graph_calc_w:float(w:float)
	if bool_eq(gb_graph_pixelmode) then w = pixel(w)
	return w * gb_graph_drawscale.x * gb_tilesize * gb_graph_scale.x
endfunction

function gb_graph_calc_h:float(h:float)
	if bool_eq(gb_graph_pixelmode) then h = pixel(h)
	return h * gb_graph_drawscale.y * gb_tilesize * gb_graph_scale.y 
endfunction

function gb_graph_calc_x3d:Float( x:Float, z:Float )
	if bool_eq(gb_graph_use_camera)
		x = x - gb_camera.pos.x
		z = z - gb_camera.pos.z
	endif
	if bool_eq(gb_graph_pixelmode)
		x = pixel(x)
		z = pixel(z)
	endif
	Return gb_visual_letterbox_rect.x + ..
		((((x-gb_graph_center.x-gb_graph_offset.x) * ..
		gb_graph_calc_perspec(z))+gb_graph_center.x)* ..
		gb_graph_drawscale.x*gb_tilesize)
endfunction

function gb_graph_calc_y3d:Float( y:Float, z:Float )
	if bool_eq(gb_graph_use_camera)
		y = y - gb_camera.pos.y
		z = z - gb_camera.pos.z
	endif
	if bool_eq(gb_graph_pixelmode)
		y = pixel(y)
		z = pixel(z)
	endif
	Return gb_visual_letterbox_rect.y + ..
		((((y-gb_graph_center.y-gb_graph_offset.y) * ..
		gb_graph_calc_perspec(z))+gb_graph_center.y)* ..
		gb_graph_drawscale.y*gb_tilesize)
endfunction

function gb_graph_calc_w3d:Float( w:Float, z:Float )
	if bool_eq(gb_graph_use_camera) then z = z - gb_camera.pos.z
	if bool_eq(gb_graph_pixelmode)
		w = pixel(w)
		z = pixel(z)
	endif
	Return w * gb_graph_drawscale.x * gb_graph_calc_perspec(z) * ..
		gb_tilesize * gb_graph_scale.x
endfunction

function gb_graph_calc_h3d:Float( h:Float, z:Float )
	if bool_eq(gb_graph_use_camera) then z = z - gb_camera.pos.z
	if bool_eq(gb_graph_pixelmode)
		h = pixel(h)
		z = pixel(z)
	endif
	Return h * gb_graph_drawscale.y * gb_graph_calc_perspec(z) * ..
		gb_tilesize * gb_graph_scale.y
endfunction

function gb_graph_within_zrange:Int(z:Float)
	if bool_eq(gb_graph_use_camera) then z = z - gb_camera.pos.z
	If in_range(z, gb_graph_zrange.low, gb_graph_zrange.high) Then Return True
	Return False
endfunction

''''''''''''''''''

function gb_graph_enable_camera( n:int )
	bool_set( gb_graph_use_camera, n )
endfunction

function gb_graph_set_pixelmode( n:int )
	bool_set( gb_graph_pixelmode, n )
endfunction

function gb_graph_enable_perspec( n:int )
	bool_set( gb_graph_use_perspec, n )
endfunction

function gb_graph_set_perspec( f:float=24.0 )
	number_set(gb_graph_perspec,f)
endfunction

function gb_graph_set_center( x:float=20.0, y:float=12.0 )
	point_set(gb_graph_center, x, y)
endfunction

function gb_graph_set_mode( d:int )
	select(d)
		case gb_graph_mode_normal
			byte_set( gb_graph_mode, gb_graph_mode_normal )
		case gb_graph_mode_add
			byte_set( gb_graph_mode, gb_graph_mode_add )
		case gb_graph_mode_multiply
			byte_set( gb_graph_mode, gb_graph_mode_multiply )
		default
			byte_set( gb_graph_mode, gb_graph_mode_normal )
	endselect
endfunction

function gb_graph_set_color( r:float, g:float, b:float )
	color_set_rgb(gb_graph_color,r,g,b)
endfunction

function gb_graph_set_color( c:t_color )
	gb_graph_color.r = c.r
	gb_graph_color.g = c.g
	gb_graph_color.b = c.b
endfunction

function gb_graph_set_alpha( a:float )
	color_set_alpha( gb_graph_color, a )
endfunction

function gb_graph_set_alpha( c:t_color )
	gb_graph_color.a = c.a
endfunction

function gb_graph_set_rgba( r:float, g:float, b:float, a:float )
	gb_graph_set_color(r,g,b)
	gb_graph_set_alpha(a)
endfunction

function gb_graph_set_angle(f:float)
	number_set(gb_graph_angle,f)
	gb_graph_calc_offsets()
endfunction

function gb_graph_set_scale(x:float, y:float)
	point_set(gb_graph_scale,x,y)
	gb_graph_calc_offsets()
endfunction

function gb_graph_set_flip(x:int, y:int)
	qset( gb_graph_flip[0], x )
	qset( gb_graph_flip[1], y )
endfunction

function gb_graph_set_handle(x:float, y:float)
	point_set(gb_graph_handle,x,y)
	gb_graph_calc_offsets()
endfunction

function gb_graph_set_clip(x:float, y:float, w:float, h:float)
	x = clamp(x,0,40.0)
	y = clamp(y,0,24.0)
	w = clamp(w,0,40.0-x)
	h = clamp(h,0,24.0-y)
	rect_set(gb_graph_clip,x,y,w,h)
endfunction

function gb_graph_reset_clip()
	gb_graph_set_clip(0,0,40,24)
	gb_graph_calc_viewport()
endfunction

function gb_graph_reset()
	gb_graph_set_rgba				(3,3,3,3)
	gb_graph_set_angle			(0)
	gb_graph_set_scale			(1,1)
	gb_graph_set_handle			(0,0)
	gb_graph_set_mode				(gb_graph_mode_normal)
	gb_graph_set_perspec		(24)
	gb_graph_set_center			(20,12)
	gb_graph_set_flip				(0,0)
	gb_graph_reset_clip			()
endfunction

'''''''''''''''''''''''
'' drawing functions ''
'''''''''''''''''''''''

function gb_graph_draw_line(x1:float, y1:float, x2:float, y2:float)
	gb_graph_prepare()
	drawline( gb_graph_calc_x(x1), gb_graph_calc_y(y1),
		gb_graph_calc_x(x2), gb_graph_calc_y(y2))
endfunction

function gb_graph_draw_box(x:float, y:float, w:float, h:float)
	gb_graph_prepare()
	DrawLine( gb_graph_calc_x(x), gb_graph_calc_y(y), gb_graph_calc_x(x+w), gb_graph_calc_y(y) )
	DrawLine( gb_graph_calc_x(x), gb_graph_calc_y(y+h), gb_graph_calc_x(x+w), gb_graph_calc_y(y+h) )
	DrawLine( gb_graph_calc_x(x), gb_graph_calc_y(y), gb_graph_calc_x(x), gb_graph_calc_y(y+h) )
	DrawLine( gb_graph_calc_x(x+w), gb_graph_calc_y(y), gb_graph_calc_x(x+w), gb_graph_calc_y(y+h) )
endfunction

function gb_graph_draw_oval(x:float, y:float, w:float, h:float)
	gb_graph_prepare()
	DrawOval( gb_graph_calc_x(x), gb_graph_calc_y(y), gb_graph_calc_w(w), gb_graph_calc_h(h))
endfunction

function gb_graph_draw_oval3d(x:float, y:float, z:float, w:float, h:float)
	If gb_graph_within_zrange(z)
		gb_graph_prepare()
		DrawOval( gb_graph_calc_x3d(x,z), gb_graph_calc_y3d(y,z), gb_graph_calc_w3d(w,z), gb_graph_calc_h3d(h,z) )
	endif
endfunction

function gb_graph_draw_rect( x:float, y:float, w:float, h:float )
	gb_graph_prepare()
	DrawRect( gb_graph_calc_x(x), gb_graph_calc_y(y), gb_graph_calc_w(w), gb_graph_calc_h(h) )
endfunction

function gb_graph_draw_rect3d( x:float, y:float, z:float, w:float, h:float )
	If gb_graph_within_zrange(z)
		gb_graph_prepare()
		DrawRect( gb_graph_calc_x3d(x,z), gb_graph_calc_y3d(y,z), gb_graph_calc_w3d(w,z), gb_graph_calc_h3d(h,z) )
	endif
endfunction

'' draw an image using an index.
function gb_graph_draw_image( x:float, y:float, im:int )
	gb_graph_prepare()
  local u:timage = gb_images[im]
	DrawSubImageRect( u, gb_graph_calc_x(x), gb_graph_calc_y(y),
		gb_graph_calc_w(u.width/10), gb_graph_calc_h(u.height/10),
		0, 0, u.width, u.height )
endfunction

'' draw an image using a timage object
function gb_graph_draw_image( x:float, y:float, im:timage )
  gb_graph_prepare()
	DrawSubImageRect( im, gb_graph_calc_x(x), gb_graph_calc_y(y),
		gb_graph_calc_w(im.width/10), gb_graph_calc_h(im.height/10),
		0, 0, im.width, im.height )
endfunction

function gb_graph_draw_tile( x:float, y:float, im:int, ix:float, iy:float )
	gb_graph_prepare()
	DrawSubImageRect( gb_images[im], gb_graph_calc_x(x), gb_graph_calc_y(y),
		gb_graph_calc_w(1), gb_graph_calc_h(1),
		ix*gb_tilesize, iy*gb_tilesize, gb_tilesize, gb_tilesize )
endfunction

function gb_graph_draw_tile3d( x:float, y:float, z:float, im:int, ix:float, iy:float )
	if gb_graph_within_zrange(z)
		gb_graph_prepare()
		DrawSubImageRect( gb_images[im], gb_graph_calc_x3d(x,z),
			gb_graph_calc_y3d(y,z), gb_graph_calc_w3d(1,z),
			gb_graph_calc_h3d(1,z), ix*gb_tilesize, iy*gb_tilesize,
			gb_tilesize, gb_tilesize )
	endif
endfunction

function gb_graph_draw_tile_rect( x:float, y:float, im:int, ix:float, iy:float, rx:int, ry:int )
	For Local i:Int = 0 To rx-1
		For Local j:Int = 0 To ry-1
			gb_graph_draw_tile(x+i, y+j, im, ix, iy)
		Next
	Next
endfunction

function gb_graph_draw_tile_rect3d( x:float, y:float, z:float, im:int, ix:float, iy:float, rx:int, ry:int )
	If gb_graph_within_zrange(z)
		For Local i:Int = 0 To rx-1
			For Local j:Int = 0 To ry-1
				gb_graph_draw_tile3d(x+i, y+j, z, im, ix, iy)
			Next
		Next
	EndIf
endfunction

function gb_graph_draw_tile_box( x:float, y:float, im:int, ix:float, iy:float, iw:float, ih:float)
	gb_graph_prepare()
	DrawSubImageRect( gb_images[im],
		gb_graph_calc_x(x), gb_graph_calc_y(y),
		gb_graph_calc_w(iw), gb_graph_calc_h(ih),
		ix*gb_tilesize, iy*gb_tilesize, iw*gb_tilesize, ih*gb_tilesize )
endfunction

function gb_graph_draw_tile_box3d( x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Float, ih:Float)
	If gb_graph_within_zrange(z)
		gb_graph_prepare()
		DrawSubImageRect( gb_images[im],
			gb_graph_calc_x3d(x,z), gb_graph_calc_y3d(y,z),
			gb_graph_calc_w3d(iw,z), gb_graph_calc_h3d(ih,z),
			ix*gb_tilesize, iy*gb_tilesize, iw*gb_tilesize, ih*gb_tilesize )
	EndIf
endfunction

function gb_graph_draw_ibox( x:float, y:float, m:t_ibox)
	gb_graph_draw_tile_box( x, y, m.id, m.x, m.y, m.w, m.h )
endfunction

function gb_graph_draw_ibox3d( x:float, y:float, z:float, m:t_ibox)
	gb_graph_draw_tile_box3d( x, y, z, m.id, m.x, m.y, m.w, m.h )
endfunction

function gb_graph_draw_tile_text(x:Float, y:Float, f:Int, t:String)
	Local ft	:t_font		= gb_fonts[f]
	Local l 	:Int 			= Len(t)
	Local c 	:Byte			= 0
	Local px	:Int			= 0
	Local py	:Int			= 0
	Local ts	:Int			= ft.tiles.x * ft.tiles.y
	Local ppx	:Float		= 0.0
	If (l > 0)
		For Local i:Int = 0 To l-1
			c 	= iwrap(t[i] - ft.offset, 0, ts)
			py	= c / ft.tiles.x
			px	= c Mod ft.tiles.x
			
			gb_graph_draw_tile_box( 											..
				x + ppx, 																		..
				y, 																					..
				ft.image_id, 																..
				ft.origin.x + (ft.tilesize.x*Float(px)), 		..
				ft.origin.y + (ft.tilesize.y*Float(py)), 		..
				ft.tilesize.x, 															..
				ft.tilesize.y 															)
			
			ppx = ppx + ft.spacing[c]
		Next
	EndIf
endfunction

function gb_graph_draw_tile_text3d( x:Float, y:Float, z:Float, f:Int, t:String)
	If gb_graph_within_zrange(z)
		Local ft	:t_font		= gb_fonts[f]
		Local l 	:Int 			= Len(t)
		Local c 	:Byte			= 0
		Local px	:Int			= 0
		Local py	:Int			= 0
		Local ts	:Int			= ft.tiles.x * ft.tiles.y
		Local ppx :Float		= 0
		If (l > 0)
			For Local i:Int = 0 To l-1
				c 	= iwrap(t[i] - ft.offset, 0, ts)
				py	= c / ft.tiles.x
				px	= c Mod ft.tiles.x
				
				gb_graph_draw_tile_box3d( x + ppx, y, z, ..
					ft.image_id, ft.origin.x + (ft.tilesize.x*px), ..
					ft.origin.y + (ft.tilesize.y*py), ..
					ft.tilesize.x, ft.tilesize.y)
				
				ppx = ppx + ft.spacing[c]
			Next
		EndIf
	EndIf
endfunction

function gb_graph_draw_tile_boxrect(x:float, y:float, im:int, ix:float, iy:float, iw:float, ih:float, rx:int, ry:int) 
	Local i:Int, j:Int
	For i = 0 To rx-1
		For j = 0 To ry-1
			gb_graph_draw_tile_box(x + (i*iw), y + (j*ih), im, ix, iy, iw, ih)
		Next
	Next
endfunction

function gb_graph_draw_tile_boxrect3d(x:float, y:float, z:float, im:int, ix:float, iy:float, iw:float, ih:float, rx:int, ry:int) 
	Local i:Int, j:Int
	For i = 0 To rx-1
		For j = 0 To ry-1
			gb_graph_draw_tile_box3d(x + (i*iw), y + (j*ih), z, im, ix, iy, iw, ih)
		Next
	Next
endfunction

function gb_graph_draw_tile_hstrip(x:float, y:float, im:int, ix:float, iy:float, tw:int=2)
	tw = ilow(tw,2)
	For Local i:Int = 0 To tw-1
		If (i = 0)
			gb_graph_draw_tile(x+i,y,im,ix,iy)
		ElseIf (i = tw-1)
			gb_graph_draw_tile(x+i,y,im,ix+2,iy)
		Else
			gb_graph_draw_tile(x+i,y,im,ix+1,iy)
		EndIf
	Next
endfunction

function gb_graph_draw_tile_vstrip( x:float, y:float, im:int, ix:float, iy:float, th:int=2)
	th = ilow(th,2)
	For Local i:Int = 0 To th-1
		If (i = 0)
			gb_graph_draw_tile(x,y+i,im,ix,iy)
		ElseIf (i = th-1)
			gb_graph_draw_tile(x,y+i,im,ix,iy+2)
		Else
			gb_graph_draw_tile(x,y+i,im,ix,iy+1)
		EndIf
	Next
endfunction

function gb_graph_draw_tile_window(x:Float, y:Float, im:Int, ix:Float, iy:Float, wx:Int=2, wy:Int=2, b:Byte=False)
	wx = ilow(wx,2)
	wy = ilow(wy,2)
	For Local i:Int = 0 To wx-1
		For Local j:Int = 0 To wy-1
		
			If (i=0) And (j=0)
				gb_graph_draw_tile( x+i, y+j, im, ix, iy )
			ElseIf (i=wx-1) And (j=0)
				gb_graph_draw_tile( x+i, y+j, im, ix+2, iy )
			ElseIf (i=0) And (j=wy-1)
				gb_graph_draw_tile( x+i, y+j, im, ix, iy+2 )
				If (b = True)
					gb_graph_draw_tile( x+i, y+j+1, im, ix, iy+3)
				EndIf
			ElseIf (i=wx-1) And (j=wy-1)
				gb_graph_draw_tile( x+i, y+j, im, ix+2, iy+2 )
				If (b = True)
					gb_graph_draw_tile( x+i, y+j+1, im, ix+2, iy+3)
				EndIf
			ElseIf (i=0)
				gb_graph_draw_tile( x+i, y+j, im, ix, iy+1 )
			ElseIf (j=0)
				gb_graph_draw_tile( x+i, y+j, im, ix+1, iy )
			ElseIf (i=wx-1)
				gb_graph_draw_tile( x+i, y+j, im, ix+2, iy+1 )
			ElseIf (j=wy-1)
				gb_graph_draw_tile( x+i, y+j, im, ix+1, iy+2 )
				If (b = True)
					gb_graph_draw_tile( x+i, y+j+1, im, ix+1, iy+3)
				EndIf
			Else
				gb_graph_draw_tile( x+i, y+j, im, ix+1, iy+1 )
			EndIf
		
		Next
	Next
endfunction

function gb_graph_draw_tile_window3d(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, wx:Int=2, wy:Int=2)
	If gb_graph_within_zrange(z)
		wx = ilow(wx,2)
		wy = ilow(wy,2)
		For Local i:Int = 0 To wx-1
			For Local j:Int = 0 To wy-1
				If (i=0) And (j=0)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix, iy )
				ElseIf (i=wx-1) And (j=0)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+2, iy )
				ElseIf (i=0) And (j=wy-1)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix, iy+2 )
				ElseIf (i=wx-1) And (j=wy-1)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+2, iy+2 )
				ElseIf (i=0)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix, iy+1 )
				ElseIf (j=0)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+1, iy )
				ElseIf (i=wx-1)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+2, iy+1 )
				ElseIf (j=wy-1)
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+1, iy+2 )
				Else
					gb_graph_draw_tile3d( x+i, y+j, z, im, ix+1, iy+1 )
				EndIf
			Next
		Next
	EndIf
endfunction

function gb_graph_draw_tile_line(x1:float, y1:float, x2:float, y2:float, im:int, ix:float, iy:float, n:int=10)
	n = ilow(n,1)
	If (n = 1)
		gb_graph_draw_tile( (x2+x1)/2, (y2+y1)/2, im, ix, iy )
	ElseIf (n > 1)
		Local dx:Float = (x2-x1)
		Local dy:Float = (y2-y1)
		For Local i:Int = 0 To n-1
			gb_graph_draw_tile( x1 + (dx*i/(n-1)), y1 + (dy*i/(n-1)), im, ix, iy )
		Next
	EndIf
endfunction

function gb_graph_draw_tile_line3d(x1:Float, y1:Float, z1:Float, x2:Float, y2:Float, z2:Float, im:Int, ix:Float, iy:Float, n:Int=10)
	n = ilow(n,1)
	If (n = 1)
		gb_graph_draw_tile3d( (x2+x1)/2, (y2+y1)/2, (z2+z1)/2, im, ix, iy )
	ElseIf (n > 1)
		Local dx:Float = (x2-x1)
		Local dy:Float = (y2-y1)
		Local dz:Float = (z2-z1)
		For Local i:Int = 0 To n-1
			gb_graph_draw_tile3d( x1 + (dx*i/(n-1)), y1 + (dy*i/(n-1)), z1 + (dz*i/(n-1)), im, ix, iy )
		Next
	EndIf
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_graph_init()
	gb_graph_color 			= new_color(3,3,3,3)
	gb_graph_scale			= new_point(1,1)
	gb_graph_handle			= new_point(0,0)
	gb_graph_offset			= new_point(0,0)
	gb_graph_clip				= new_rect(0,0,40,24)
	gb_graph_angle			= new_number(0,0,1000,true)
	gb_graph_perspec		= new_number(24,1,1000)
	gb_graph_zrange			= new_number(0,-23.99,10000)
	gb_graph_mode				= new_byte(gb_graph_mode_normal)
	gb_graph_center			= new_point(20,12)
	gb_graph_drawscale	= new_point(1,1)
	gb_graph_flip				= new_boolv(2,false)
	gb_graph_pixelmode	= new_bool(false)
	gb_graph_use_camera	= new_bool(true)
	
	gb_graph_calc_drawscale()
endfunction

