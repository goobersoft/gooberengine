
''''''''''''
'' consts ''
''''''''''''

const gb_mouse_tile_id						:int 		= 0
const gb_mouse_tile_x 						:int 		= 4
const gb_mouse_tile_y 						:int 		= 20
Const gb_mouse_clicktime_frames		:Float 	=	4

''''''''''
'' type ''
''''''''''

type t_gb_mouse
  field click       :t_int
  field clicktime   :t_number
  field clickdiff   :t_point
  field idletime    :t_number
  field clickpos    :t_point
  field flashing    :t_bool
  field flashtime   :t_number
  
  field visible     :t_bool
  field locked      :t_bool

  field pos         :t_point
  field pos_old     :t_point
  field pos_diff    :t_point
endtype

function new_gb_mouse:t_gb_mouse ()
  local r:t_gb_mouse = new t_gb_mouse
  
  r.click       = new_int(0)
  r.clicktime   = new_number(gb_mouse_clicktime_frames,0,gb_mouse_clicktime_frames)
  r.clickpos    = new_point(0,0)
  r.clickdiff   = new_point(0,0)
  r.idletime    = new_number(4,0,4)
  r.flashing    = new_bool(false)
  r.flashtime   = new_number(0,0,0.25,true)
  r.visible     = new_bool(true)
  r.locked      = new_bool(false)
  r.pos         = new_point(VirtualMouseX() / gb_tilesize, VirtualMouseY() / gb_tilesize)
  r.pos_old     = clone_point(r.pos)
  r.pos_diff    = new_point(0,0)

  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_mouse_init()
  gb.mouse = new_gb_mouse()
endfunction

function gb_mouse_update()
  local d:float   = gb_delta()
	point_set(gb.mouse.pos_old, gb.mouse.pos.x, gb.mouse.pos.y)
	point_set(gb.mouse.pos, gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y())
	point_set(gb.mouse.pos_diff, gb.mouse.pos.x - gb.mouse.pos_old.x,
		gb.mouse.pos.y - gb.mouse.pos_old.y)
	
	if mousedown(1)
		if int_eq(gb.mouse.click,0)
			int_set(gb.mouse.click,2)
			point_set(gb.mouse.clickpos, gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y())
			number_to_low(gb.mouse.clicktime)
			number_to_high(gb.mouse.idletime)
		elseif int_eq(gb.mouse.click,2)
			int_set(gb.mouse.click,1)
		endif
	else
		int_set(gb.mouse.click,0)
	endif
	number_add(gb.mouse.clicktime, d*5*gb.mouse.clicktime.high)
	number_add(gb.mouse.idletime,-d)
	
	local dd:float = virtualmousexspeed() + virtualmouseyspeed()
	if (dd <> 0)
		number_to_high(gb.mouse.idletime)
	endif
	
	if bool_eq(gb.mouse.flashing,true)
		bool_set(gb.mouse.flashing,false)
		number_add(gb.mouse.flashtime,d)
	else
		number_to_low(gb.mouse.flashtime)
	endif
	
	point_set(gb.mouse.clickdiff, gb.mouse.pos.x - gb.mouse.clickpos.x, gb.mouse.pos.y - gb.mouse.clickpos.y)
endfunction


function gb_mouse_draw(x:float=0, y:float=0)
	if bool_eq(gb.mouse.visible,true)
		gb_graph_enable_camera(false)
		gb_graph_draw_tile(gb.mouse.clickpos.x-0.5, gb.mouse.clickpos.y-0.5,
			0, gb_mouse_tile_x + 1 + ffloor(gb.mouse.clicktime.value),
			gb_mouse_tile_y)
			
		local ux:float
		local uy:float
		if bool_eq(gb.mouse.flashing,true)
			ux = 5 + ffloor(gb.mouse.flashtime.value * 20)
			uy = 19
		else
			ux = 4
			uy = 20
		endif
		
		if number_gt(gb.mouse.idletime,0.5)
			gb_graph_draw_tile(gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y(),
				0, ux, uy)
		elseif number_gt(gb.mouse.idletime,0)
			gb_graph_set_alpha(3 * gb.mouse.idletime.value*2)
			gb_graph_draw_tile(gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y(),
				0, ux, uy) 
			gb_graph_set_alpha(3)
		endif
		gb_graph_enable_camera(true)
	endif
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_mouse_pressed:int()
	return int_eq(gb.mouse.click,2)
endfunction

function gb_mouse_held:int()
	return int_gt(gb.mouse.click,0)
endfunction

function gb_mouse_down:int()
	return int_gt(gb.mouse.click,0)
endfunction

function gb_mouse_virtual_tile_x:float()
	Return (mousex()-gb.visual.letterbox.rect.x) / gb_tilesize / float_get(gb.visual.pixelscale)
endfunction

function gb_mouse_virtual_tile_y:float()
	Return (mousey()-gb.visual.letterbox.rect.y) / gb_tilesize / float_get(gb.visual.pixelscale)
endfunction

function gb_mouse_virtual_pos_x:float()
	Return mousex() / float_get(gb.visual.pixelscale)
endfunction

function gb_mouse_virtual_pos_y:float()
	Return mousey() / float_get(gb.visual.pixelscale)
endfunction

function gb_mouse_flash()
	bool_set(gb.mouse.flashing, true)
endfunction

function gb_mouse_move(x:float, y:float)
	point_set(gb.mouse.pos_old, gb.mouse.pos.x, gb.mouse.pos.y)
	point_set(gb.mouse.pos, x, y)
	point_set(gb.mouse.pos_diff, gb.mouse.pos.x-gb.mouse.pos_old.x, gb.mouse.pos.y-gb.mouse.pos_old.y)
	movemouse	( int(gb.mouse.pos.x * gb_tilesize * float_get(gb.visual.windowscale)),
			int(gb.mouse.pos.y * gb_tilesize * float_get(gb.visual.windowscale)) )
endfunction

function gb_mouse_center()
	gb_mouse_move(20,12)
endfunction

function gb_mouse_get_diff_x:float()
	return gb.mouse.pos_diff.x
endfunction

function gb_mouse_get_diff_y:float()
	return gb.mouse.pos_diff.y
endfunction

