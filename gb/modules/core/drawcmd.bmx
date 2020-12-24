
Const drawcmd_id_oval						:Int	= 1
Const drawcmd_id_rect						:Int	= 2
Const drawcmd_id_tile						:Int 	= 3
Const drawcmd_id_tile_rect			:Int	= 4
Const drawcmd_id_tile_box				:Int	= 5
Const drawcmd_id_tile_boxrect		:Int	= 6
Const drawcmd_id_tile_text			:Int	= 7
Const drawcmd_id_tile_window		:Int	= 8
Const drawcmd_id_tile_line3d		:Int	= 9

type t_drawcmd
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

function new_drawcmd :t_drawcmd(id:int, p:int)
	local r:t_drawcmd = new t_drawcmd
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

function drawcmd_set_pos(d:t_drawcmd, x:float, y:float, z:float)
	d.pos[0] = x
	d.pos[1] = y
	d.pos[2] = z
endfunction

function drawcmd_set_rotation(d:t_drawcmd, r:float)
	d.angle = wrap(r,0,1000)
endfunction

function drawcmd_set_scale(d:t_drawcmd, x:float, y:float)
	d.scale[0] = x
	d.scale[1] = y
endfunction

function drawcmd_set_handle(d:t_drawcmd, x:float, y:float)
	d.handle[0] = x
	d.handle[1] = y
endfunction

function drawcmd_set_color(d:t_drawcmd, r:float, g:float, b:float)
	d.color[0] = clamp(r,0,3)
	d.color[1] = clamp(g,0,3)
	d.color[2] = clamp(b,0,3)
endfunction

function drawcmd_set_alpha(d:t_drawcmd, a:float)
	d.color[3] = clamp(a,0,3)
endfunction

function drawcmd_set_mode(d:t_drawcmd, m:int)
	d.mode = m
endfunction

function drawcmd_set_life(d:t_drawcmd, l:float)
	d.life = low(l,0)
endfunction

''''''''''''
'' events ''
''''''''''''

function drawcmd_update(e:t_drawcmd, d:float)
	e.life = low(e.life-d,0)
endfunction

function drawcmd_draw(d:t_drawcmd, x:float=0, y:float=0, z:float=0, p:int=true)
	gb_graph_set_angle		(d.angle)
	gb_graph_set_scale		(d.scale[0], d.scale[1])
	gb_graph_set_handle		(d.handle[0], d.handle[1])
	gb_graph_set_color		(d.color[0], d.color[1], d.color[2])
	gb_graph_set_alpha		(d.color[3])
	gb_graph_set_mode			(d.mode)
	
	if (p = true)
		select d.id
			case drawcmd_id_oval
				gb_graph_draw_oval3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawcmd_id_rect
				gb_graph_draw_rect3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawcmd_id_tile
				gb_graph_draw_tile3d( d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]))
			case drawcmd_id_tile_rect
				gb_graph_draw_tile_rect3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
			case drawcmd_id_tile_box
				gb_graph_draw_tile_box3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]))
			case drawcmd_id_tile_boxrect
				gb_graph_draw_tile_boxrect3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]),
					int_get(d.params[5]), int_get(d.params[6]))
			case drawcmd_id_tile_text
				gb_graph_draw_tile_text3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), string_get(d.params[1]))
			case drawcmd_id_tile_window
				gb_graph_draw_tile_window3d(d.pos[0]+x, d.pos[1]+y, d.pos[2]+z,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
		endselect
	else
		select d.id
			case drawcmd_id_oval
				gb_graph_draw_oval( d.pos[0]+x, d.pos[1]+y, 
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawcmd_id_rect
				gb_graph_draw_rect( d.pos[0]+x, d.pos[1]+y,
					float_get(d.params[0]), float_get(d.params[1]) )
			case drawcmd_id_tile
				gb_graph_draw_tile( d.pos[0]+x, d.pos[1]+y,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]))
			case drawcmd_id_tile_rect
				gb_graph_draw_tile_rect(d.pos[0]+x, d.pos[1]+y,
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
			case drawcmd_id_tile_box
				gb_graph_draw_tile_box(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]))
			case drawcmd_id_tile_boxrect
				gb_graph_draw_tile_boxrect(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					float_get(d.params[3]), float_get(d.params[4]),
					int_get(d.params[5]), int_get(d.params[6]))
			case drawcmd_id_tile_text
				gb_graph_draw_tile_text(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), string_get(d.params[1]))
			case drawcmd_id_tile_window
				gb_graph_draw_tile_window(d.pos[0]+x, d.pos[1]+y, 
					int_get(d.params[0]), float_get(d.params[1]), float_get(d.params[2]),
					int_get(d.params[3]), int_get(d.params[4]))
		endselect
	endif
endfunction

''''''''''''''
'' subtypes ''
''''''''''''''

Function new_drawcmd_oval :t_drawcmd( x:Float, y:Float, z:Float, w:Float, h:Float )
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_oval, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_float(w)
	r.params[1] = new_float(h)
	Return r
EndFunction

Function new_drawcmd_rect :t_drawcmd( x:Float, y:Float, z:Float, w:Float, h:Float )
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_rect, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_float(w)
	r.params[1] = new_float(h)
	Return r
EndFunction

Function new_drawcmd_tile :t_drawcmd( x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float )
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile, 3)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_float(ix)
	r.params[2] = new_float(iy)
	Return r
EndFunction

Function new_drawcmd_tile_rect :t_drawcmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Int, ih:Int)
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile_rect, 5)
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

Function new_drawcmd_tile_box :t_drawcmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Float, ih:Float)
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile_box, 5)
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

Function new_drawcmd_tile_boxrect :t_drawcmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, iw:Float, ih:Float, rx:Int, ry:Int)
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile_boxrect, 7)
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

Function new_drawcmd_tile_text :t_drawcmd(x:Float, y:Float, z:Float, im:Int, s:String)
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile_text, 2)
	r.pos[0]		= x
	r.pos[1]		= y
	r.pos[2]		= z
	r.params[0] = new_int(im)
	r.params[1] = new_string(s)
	Return r
EndFunction

Function new_drawcmd_tile_window :t_drawcmd(x:Float, y:Float, z:Float, im:Int, ix:Float, iy:Float, tw:Int, th:Int)
	Local r:t_drawcmd = new_drawcmd(drawcmd_id_tile_window, 5)
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