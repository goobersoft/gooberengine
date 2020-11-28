
'''''''''''
'' nodes ''
'''''''''''

type t_drawstack_node
	field link 	:t_drawstack_node
	field data	:t_drawcmd
endtype

function new_drawstack_node:t_drawstack_node(d:t_drawcmd)
	local r:t_drawstack_node = new t_drawstack_node
	r.link = null
	r.data = d
	return r
endfunction

'''''''''''''''
'' drawstack ''
'''''''''''''''
'' count [int]: number of items
'' first [drawstack node]: first in stack.
'' perspec [bool]: use perspective scaling

type t_drawstack
	field count 				:t_int
	field first					:t_drawstack_node
	field perspec				:t_bool
endtype

function new_drawstack:t_drawstack(p:int=true)
	local r:t_drawstack = new t_drawstack
	r.count 	= new_int(0)
	r.first 	= null
	r.perspec	= new_bool(p)
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function drawstack_push(d:t_drawstack, m:t_drawcmd)
	local t	 	:t_drawstack_node = new_drawstack_node(m)
	local v	 	:t_drawstack_node
	local vl 	:t_drawstack_node
	local dn 	:byte
	
	if d.first = null
		d.first = t
		int_set(d.count,1)
	else
		v 		= d.first
		vl 		= null
		dn		= false
		repeat
			if (t.data.pos[2] < v.data.pos[2])
				if (v.link)
					vl 	= v
					v 	= v.link
				else
					v.link = t
					int_add(d.count,1)
					dn = true
				endif
			else
				if (d.first = v)
					d.first 	= t
					t.link 		= v
					int_add(d.count,1)
					dn 				= true
				else
					vl.link 	= t
					t.link		= v
					int_add(d.count,1)
					dn = true
				endif
			endif
		until dn = true
	endif
endfunction

function drawstack_pop:t_drawcmd(d:t_drawstack)
	local r :t_drawcmd
	if (d.first)
		r = d.first.data
		if (d.first.link)
			d.first = d.first.link
			int_add(d.count,-1)
		else
			d.first = null
			int_set(d.count,0)
		endif
	endif
	return r
endfunction

''''''''''''''''''''''
'' drawing commands ''
''''''''''''''''''''''

function drawstack_draw_tile:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, ix:float, iy:float )
	local r:t_drawcmd = new_drawcmd_tile(x,y,z,im,ix,iy)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tile_rect:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, ix:float, iy:float, rx:int, ry:int)
	local r:t_drawcmd = new_drawcmd_tile_rect(x,y,z,im,ix,iy,rx,ry)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tile_box:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, ix:float, iy:float, iw:float, ih:float)
	local r:t_drawcmd = new_drawcmd_tile_box(x,y,z,im,ix,iy,iw,ih)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tile_boxrect:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, ix:float, iy:float, iw:float, ih:float, rx:int, ry:int)
	local r:t_drawcmd = new_drawcmd_tile_boxrect(x,y,z,im,ix,iy,iw,ih,rx,ry)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tile_text:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, s:string)
	local r:t_drawcmd = new_drawcmd_tile_text(x,y,z,im,s)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tile_window:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
im:int, ix:float, iy:float, tw:int, th:int)
	local r:t_drawcmd = new_drawcmd_tile_window(x,y,z,im,ix,iy,tw,th)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_sprite:t_drawcmd( d:t_drawstack, x:float, y:float, z:float,
s:t_sprite)
	local r:t_drawcmd = new_drawcmd_tile_box(s.pos.x, s.pos.y, s.pos.z,
		s.ibox.id, s.ibox.x, s.ibox.y, s.ibox.w, s.ibox.h)
	r.color[0] 	= s.color.r
	r.color[1] 	= s.color.g
	r.color[2] 	= s.color.b
	r.color[3] 	= s.color.a
	r.handle[0]	= s.handle.x
	r.handle[1] = s.handle.y
	r.angle			= s.angle.value
	r.scale[0]	= s.scale.x
	r.scale[1]	= s.scale.y
	r.mode			= s.mode.value
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_tilemap:t_drawcmd(d:t_drawstack,x:float,y:float,z:float,t:t_tilemap)
endfunction

function drawstack_draw_tile_line(d:t_drawstack,x:float,y:float,z:float,
x2:float, y2:float, z2:float, im:int, ix:float, iy:float, s:int)
	Local dx:Float = (x2 - x) / (s-1)
	Local dy:Float = (y2 - y) / (s-1)
	Local dz:Float = (z2 - z) / (s-1)
	Local sx:Float = x
	Local sy:Float = y
	Local sz:Float = z
	s = low(s,2)
	for local i:int = 0 to s-1
		drawstack_draw_tile(d, sx,sy,sz, im,ix,iy)
		sx = sx + dx
		sy = sy + dy
		sz = sz + dz
	next
endfunction

function drawstack_draw_rect:t_drawcmd(d:t_drawstack, x:float, y:float, z:float,
w:float, h:float)
	local r:t_drawcmd = new_drawcmd_rect(x,y,z,w,h)
	drawstack_push(d,r)
	return r
endfunction

function drawstack_draw_oval:t_drawcmd(d:t_drawstack, x:float, y:float, z:float,
w:float, h:float)
	local r:t_drawcmd = new_drawcmd_oval(x,y,z,w,h)
	drawstack_push(d,r)
	return r
endfunction

''''''''''''
'' events ''
''''''''''''

function drawstack_update(a:t_drawstack, u:float)
endfunction

function drawstack_draw3d(d:t_drawstack, x:float=0, y:float=0, z:float=0)
	local u:t_drawcmd = drawstack_pop(d)
	while (u)
		drawcmd_draw(u, x,y,z, d.perspec.value)
		u = drawstack_pop(d)
	wend
	gb_graph_reset()
endfunction