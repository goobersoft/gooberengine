
const lightcube_max_points :int = 1000

type t_lightcube
	field drawstack 	:t_drawstack
	field pos					:t_point3
  field newpos      :t_point3
  field posfric     :t_float
	field points			:t_point3[]
	field points_new	:t_point3[]
	field angle				:t_point3
	field anglespeed	:t_point3
	
	field ibox				:t_ibox
endtype

function new_lightcube:t_lightcube(f:byte=false)
	local r:t_lightcube = new t_lightcube
	r.drawstack 	= new_drawstack()
	r.pos					= new_point3(0,0,1000)
  r.newpos      = new_point3(0,0,-24)
  r.posfric     = new_float(0.05)
	r.points			= new t_point3[ lightcube_max_points ]
	r.points_new	= new t_point3[ lightcube_max_points ]
	r.angle				= new_point3(frand(0,1000),frand(0,1000),frand(0,1000))
	r.anglespeed	= new_point3(frand(-10,10),frand(-10,10),frand(-10,10))
	r.ibox				= new_ibox(0,0,0,0,0)
	
  ibox_set(r.ibox, 0,5+ffloor(frand(0,5)),9+ffloor(frand(0,5)),1,1)
	
	local px:int = 0
	local py:int = 0
	local pz:int = 0
	local d:int  = 0
	
	while d < lightcube_max_points
		r.points[d] 		= new_point3( px-4.5, py-4.5, pz-4.5 )
		r.points_new[d]	= new_point3(0,0,0)
		
		px = px + 1
		if px >= 10
			px = 0
			py = py + 1
			if py >= 10
				py = 0
				pz = pz + 1
			endif
		endif
		d = d + 1
	wend
	
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function lightcube_set_icon(a:t_lightcube, x:float=0, y:float=0)
	x = ffloor(clamp(x,0,3))
	y = ffloor(clamp(y,0,3))
	ibox_set(a.ibox, 1, 17+x, 15+y, 1, 1)
endfunction

function lightcube_set_pos( a:t_lightcube, x:float, y:float, z:float )
	point3_set(a.newpos, x, y, z)
endfunction

function lightcube_set_anglespeed( a:t_lightcube, x:float, y:float, z:float )
	point3_set( a.anglespeed, x, y, z )
endfunction

function lightcube_set_angle( a:t_lightcube, x:float, y:float, z:float )
	point3_set( a.angle, x, y, z )
endfunction

function lightcube_reset( l:t_lightcube )
  point3_set( l.pos, 0,0, 1000 )
  point3_set( l.newpos, 0, 0, -24 )
  point3_set( l.angle, frand(0,1000), frand(0,1000), frand(0,1000) )
  ibox_set(l.ibox, 0,5 + ffloor(frand(0,5)), 9+ffloor(frand(0,5)), 1,1)
endfunction

''''''''''''
'' events ''
''''''''''''

function lightcube_update(a:t_lightcube)
  local d:float = gb_delta()


  a.pos.x = a.pos.x + ((a.newpos.x - a.pos.x) * a.posfric.value)
  a.pos.y = a.pos.y + ((a.newpos.y - a.pos.y) * a.posfric.value)
  a.pos.z = a.pos.z + ((a.newpos.z - a.pos.z) * a.posfric.value)

	a.angle.x = wrap(a.angle.x + (a.anglespeed.x*d), 0, 1000)
	a.angle.y = wrap(a.angle.y + (a.anglespeed.y*d), 0, 1000)
	a.angle.z = wrap(a.angle.z + (a.anglespeed.z*d), 0, 1000)
	
	local u:float[]
	local r:float
	for local i:int = 0 to lightcube_max_points-1
		
		u = unit		( a.points[i].y, a.points[i].z )
		r = angle_x	( u[0], u[1] )
		r = wrap		( r + a.angle.x, 0, 1000 )
		
		a.points_new[i].x = a.points[i].x
		a.points_new[i].y = rot_xy( r, u[2] )
		a.points_new[i].z = rot_xz( r, u[2] )
		
		u = unit		( a.points_new[i].x, a.points_new[i].z )
		r = angle_y	( u[0], u[1] )
		r = wrap		( r + a.angle.y, 0, 1000 )
		
		a.points_new[i].x = rot_yx( r, u[2] )
		a.points_new[i].z = rot_yz( r, u[2] )
		
		u = unit		( a.points_new[i].x, a.points_new[i].y )
		r = angle_z	( u[0], u[1] )
		r = wrap		( r + a.angle.z, 0, 1000 )
		
		a.points_new[i].x = rot_zx( r, u[2] )
		a.points_new[i].y = rot_zy( r, u[2] )
		
	next
endfunction

function lightcube_draw( a:t_lightcube, x:float=0, y:float=0 )
	lightcube_draw3d( a, x, y, 0 )
endfunction

function lightcube_draw3d(a:t_lightcube, x:float=0, y:float=0, z:float=0)
	local u:t_drawcmd
	local xx:int = 0
	local yy:int = 0
	local zz:int = 0
	for local i:int = 0 to lightcube_max_points-1
		u = drawstack_draw_tile(a.drawstack,
			a.pos.x + a.points_new[i].x,
			a.pos.y + a.points_new[i].y,
			a.pos.z + a.points_new[i].z,
			a.ibox.id, a.ibox.x, a.ibox.y )
		xx = i / 100
		yy = (i / 10) mod 10
		zz = i mod 10
		drawcmd_set_color( u, (float(xx)/15), (float(yy)/15), (float(zz)/15))
		drawcmd_set_mode( u, gb_graph_mode_add )
	next
	drawstack_draw3d(a.drawstack, x,y,z)
endfunction