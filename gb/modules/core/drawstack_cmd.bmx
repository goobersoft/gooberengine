
Const drawstack_cmd_id_oval						:Int	= 1
Const drawstack_cmd_id_rect						:Int	= 2
const drawstack_cmd_id_tri            :int  = 3
Const drawstack_cmd_id_tile						:Int 	= 4
Const drawstack_cmd_id_tile_rect			:Int	= 5
Const drawstack_cmd_id_tile_box				:Int	= 6
Const drawstack_cmd_id_tile_boxrect		:Int	= 7
Const drawstack_cmd_id_tile_text			:Int	= 8
Const drawstack_cmd_id_tile_window		:Int	= 9
Const drawstack_cmd_id_tile_line3d		:Int	= 10

type t_drawstack_cmd
	Field pos			:Float[]
	Field id			:Int
	Field params	:Object[]
	Field angle		:Float
	Field scale		:Float[]
	Field handle	:Float[]
	Field color		:Float[]
	Field mode		:Byte
	
	''	This field is used for command drawing persistence
	''	Setting this less than 0 will allow it to persist forever.
	Field life		:Float
endtype

function new_drawstack_cmd :t_drawstack_cmd(id:int, p:int)
	local r:t_drawstack_cmd = new t_drawstack_cmd
	r.id 				= id
	r.pos 			= [0#, 0#, 0#]
	r.scale 		= [1#, 1#]
	r.handle 		= [0#, 0#]
	r.color			= [3#, 3#, 3#, 3#]
	r.angle			= 0.0
	r.mode			= alphablend
	r.params		= new object[p]
	r.life			= 0.0
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function drawstack_cmd_set_pos(d:t_drawstack_cmd, x:float, y:float, z:float)
	d.pos[0] = x
	d.pos[1] = y
	d.pos[2] = z
endfunction

function drawstack_cmd_set_rotation(d:t_drawstack_cmd, r:float)
	d.angle = wrap(r,0,1000)
endfunction

function drawstack_cmd_set_scale(d:t_drawstack_cmd, x:float, y:float)
	d.scale[0] = x
	d.scale[1] = y
endfunction

function drawstack_cmd_set_handle(d:t_drawstack_cmd, x:float, y:float)
	d.handle[0] = x
	d.handle[1] = y
endfunction

function drawstack_cmd_set_color(d:t_drawstack_cmd, r:float, g:float, b:float)
	d.color[0] = clamp(r,0,3)
	d.color[1] = clamp(g,0,3)
	d.color[2] = clamp(b,0,3)
endfunction

function drawstack_cmd_set_alpha(d:t_drawstack_cmd, a:float)
	d.color[3] = clamp(a,0,3)
endfunction

function drawstack_cmd_set_mode(d:t_drawstack_cmd, m:int)
	d.mode = m
endfunction

function drawstack_cmd_set_life(d:t_drawstack_cmd, l:float)
	d.life = low(l,0)
endfunction

''''''''''''
'' events ''
''''''''''''

function drawstack_cmd_update(e:t_drawstack_cmd, d:float)
	e.life = low(e.life-d,0)
endfunction

function drawstack_cmd_draw(d:t_drawstack_cmd, x:float=0, y:float=0, z:float=0, p:int=true)
	gb_graph_set_angle		(d.angle)
	gb_graph_set_scale		(d.scale[0], d.scale[1])
	gb_graph_set_handle		(d.handle[0], d.handle[1])
	gb_graph_set_color		(d.color[0], d.color[1], d.color[2])
	gb_graph_set_alpha		(d.color[3])
	gb_graph_set_mode			(d.mode)
	
	if (p = true)
		select d.id
			case drawstack_cmd_id_oval
				gb_graph_draw_oval3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawstack_cmd_id_rect
				gb_graph_draw_rect3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					float_get(d.params[0]), float_get(d.params[1]) )
      case drawstack_cmd_id_tri
        gb_graph_draw_tri3d( t_point3(d.params[0]), t_point3(d.params[1]), t_point3(d.params[2]) )
			case drawstack_cmd_id_tile
				gb_graph_draw_tile3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]))
			case drawstack_cmd_id_tile_rect
				gb_graph_draw_tile_rect3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
			case drawstack_cmd_id_tile_box
				gb_graph_draw_tile_box3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]))
			case drawstack_cmd_id_tile_boxrect
				gb_graph_draw_tile_boxrect3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]),
					int_get(d.params[5]), int_get(d.params[6]))
			case drawstack_cmd_id_tile_text
				gb_graph_draw_tile_text3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), string_get(d.params[1]))
			case drawstack_cmd_id_tile_window
				gb_graph_draw_tile_window3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
		endselect
	else
		select d.id
			case drawstack_cmd_id_oval
				gb_graph_draw_oval( d.pos[0]+x, d.pos[1]+y, 
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawstack_cmd_id_rect
				gb_graph_draw_rect( d.pos[0]+x, d.pos[1]+y,
					float_get(d.params[0]), float_get(d.params[1]) )
      case drawstack_cmd_id_tri
        gb_graph_draw_tri( t_point3(d.params[0]), t_point3(d.params[1]), t_point3(d.params[2]) )
			case drawstack_cmd_id_tile
				gb_graph_draw_tile( d.pos[0]+x, d.pos[1]+y,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]))
			case drawstack_cmd_id_tile_rect
				gb_graph_draw_tile_rect(d.pos[0]+x, d.pos[1]+y,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
			case drawstack_cmd_id_tile_box
				gb_graph_draw_tile_box(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]))
			case drawstack_cmd_id_tile_boxrect
				gb_graph_draw_tile_boxrect(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]),
					int_get(d.params[5]), int_get(d.params[6]))
			case drawstack_cmd_id_tile_text
				gb_graph_draw_tile_text(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), string_get(d.params[1]))
			case drawstack_cmd_id_tile_window
				gb_graph_draw_tile_window(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
		endselect
	endif
endfunction

''''''''''''''
'' subtypes ''
''''''''''''''

Function new_drawstack_cmd_oval :t_drawstack_cmd( x:Float, y:Float, z:Float, w:Float, h:Float )
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_oval, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_float(w)
	r.params[1] = new_float(h)
	Return r
EndFunction

Function new_drawstack_cmd_rect :t_drawstack_cmd( x:Float, y:Float, z:Float, w:Float, h:Float )
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_rect, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_float(w)
	r.params[1] = new_float(h)
	Return r
EndFunction

Function new_drawstack_cmd_tri:t_drawstack_cmd( p1:t_point3, p2:t_point3, p3:t_point3 )
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tri, 3)
	r.pos[0]		= (p1.x + p2.x + p3.x) / 3
	r.pos[1]		= (p1.y + p2.y + p3.y) / 3
	r.pos[2]		= (p1.z + p2.z + p3.z) / 3
	r.params[0] = p1
	r.params[1] = p2
  r.params[2] = p3
	Return r
EndFunction

Function new_drawstack_cmd_tile :t_drawstack_cmd( x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float )
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile, 3)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	Return r
EndFunction

Function new_drawstack_cmd_tile_rect :t_drawstack_cmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Int, ih:Int)
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile_rect, 5)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	r.params[3] = new_int(iw)
	r.params[4] = new_int(ih)
	Return r
EndFunction

Function new_drawstack_cmd_tile_box :t_drawstack_cmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Float, ih:Float)
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile_box, 5)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	r.params[3] = new_float(iw)
	r.params[4] = new_float(ih)
	Return r
EndFunction

Function new_drawstack_cmd_tile_boxrect :t_drawstack_cmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Float, ih:Float, rx:Int, ry:Int)
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile_boxrect, 7)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	r.params[3] = new_float(iw)
	r.params[4] = new_float(ih)
	r.params[5] = new_int(rx)
	r.params[6] = new_int(ry)
	Return r
EndFunction

Function new_drawstack_cmd_tile_text :t_drawstack_cmd(x:Float, y:Float, z:Float, im:Int, s:String)
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile_text, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_string(s)
	Return r
EndFunction

Function new_drawstack_cmd_tile_window :t_drawstack_cmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, tw:Int, th:Int)
	Local r:t_drawstack_cmd = new_drawstack_cmd(drawstack_cmd_id_tile_window, 5)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	r.params[3] = new_int(tw)
	r.params[4] = new_int(th)
	Return r
EndFunction