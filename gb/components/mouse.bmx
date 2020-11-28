
''''''''''''
'' consts ''
''''''''''''

const gb_mouse_tile_id						:int 		= 0
const gb_mouse_tile_x 						:int 		= 4
const gb_mouse_tile_y 						:int 		= 20
Const gb_mouse_clicktime_frames		:Float 	=	4

'''''''''''''
'' globals ''
'''''''''''''

global gb_mouse_click 				:t_int
global gb_mouse_clicktime 		:t_number
global gb_mouse_clickdiff			:t_point
global gb_mouse_idletime			:t_number
global gb_mouse_clickpos			:t_point
global gb_mouse_flashing			:t_bool
global gb_mouse_flashtime			:t_number

global gb_mouse_visible				:t_bool
global gb_mouse_locked				:t_bool

global gb_mouse_pos_old				:t_point
global gb_mouse_pos						:t_point
global gb_mouse_pos_diff			:t_point

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_mouse_pressed:int()
	return int_eq(gb_mouse_click,2)
endfunction

function gb_mouse_held:int()
	return int_gt(gb_mouse_click,0)
endfunction

function gb_mouse_down:int()
	return int_gt(gb_mouse_click,0)
endfunction

function gb_mouse_virtual_tile_x:float()
	Return (mousex()-gb_visual_letterbox_rect.x) / gb_tilesize / float_get(gb_visual_pixelscale)
endfunction

function gb_mouse_virtual_tile_y:float()
	Return (mousey()-gb_visual_letterbox_rect.y) / gb_tilesize / float_get(gb_visual_pixelscale)
endfunction

function gb_mouse_virtual_pos_x:float()
	Return mousex() / float_get(gb_visual_pixelscale)
endfunction

function gb_mouse_virtual_pos_y:float()
	Return mousey() / float_get(gb_visual_pixelscale)
endfunction

function gb_mouse_flash()
	bool_set(gb_mouse_flashing, true)
endfunction

function gb_mouse_move(x:float, y:float)
	point_set(gb_mouse_pos_old, gb_mouse_pos.x, gb_mouse_pos.y)
	point_set(gb_mouse_pos, x, y)
	point_set(gb_mouse_pos_diff, gb_mouse_pos.x-gb_mouse_pos_old.x, gb_mouse_pos.y-gb_mouse_pos_old.y)
	movemouse	( int(gb_mouse_pos.x * gb_tilesize * float_get(gb_visual_windowscale)),
			int(gb_mouse_pos.y * gb_tilesize * float_get(gb_visual_windowscale)) )
endfunction

function gb_mouse_center()
	gb_mouse_move(20,12)
endfunction

function gb_mouse_get_diff_x:float()
	return gb_mouse_pos_diff.x
endfunction

function gb_mouse_get_diff_y:float()
	return gb_mouse_pos_diff.y
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_mouse_init()
	gb_mouse_click 				= new_int(0)
	gb_mouse_clicktime		= new_number(gb_mouse_clicktime_frames,0,gb_mouse_clicktime_frames)
	gb_mouse_clickpos			= new_point(0,0)
	gb_mouse_clickdiff		= new_point(0,0)
	gb_mouse_idletime			= new_number(4,0,4)
	gb_mouse_flashing			= new_bool(false)
	gb_mouse_flashtime		= new_number(0,0,0.25,true)
	gb_mouse_visible			= new_bool(true)
	gb_mouse_locked				= new_bool(false)
	gb_mouse_pos					= new_point(VirtualMouseX() / gb_tilesize, VirtualMouseY() / gb_tilesize)
	gb_mouse_pos_old			= clone_point(gb_mouse_pos)
	gb_mouse_pos_diff			= new_point(0,0)
endfunction

function gb_mouse_update(d:float)
	point_set(gb_mouse_pos_old, gb_mouse_pos.x, gb_mouse_pos.y)
	point_set(gb_mouse_pos, gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y())
	point_set(gb_mouse_pos_diff, gb_mouse_pos.x - gb_mouse_pos_old.x,
		gb_mouse_pos.y - gb_mouse_pos_old.y)
	
	if mousedown(1)
		if int_eq(gb_mouse_click,0)
			int_set(gb_mouse_click,2)
			point_set(gb_mouse_clickpos, gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y())
			number_to_low(gb_mouse_clicktime)
			number_to_high(gb_mouse_idletime)
		elseif int_eq(gb_mouse_click,2)
			int_set(gb_mouse_click,1)
		endif
	else
		int_set(gb_mouse_click,0)
	endif
	number_add(gb_mouse_clicktime, d*5*gb_mouse_clicktime.high)
	number_add(gb_mouse_idletime,-d)
	
	local dd:float = virtualmousexspeed() + virtualmouseyspeed()
	if (dd <> 0)
		number_to_high(gb_mouse_idletime)
	endif
	
	if bool_eq(gb_mouse_flashing,true)
		bool_set(gb_mouse_flashing,false)
		number_add(gb_mouse_flashtime,d)
	else
		number_to_low(gb_mouse_flashtime)
	endif
	
	point_set(gb_mouse_clickdiff, gb_mouse_pos.x - gb_mouse_clickpos.x, gb_mouse_pos.y - gb_mouse_clickpos.y)
endfunction


function gb_mouse_draw(x:float=0, y:float=0)
	if bool_eq(gb_mouse_visible,true)
		gb_graph_enable_camera(false)
		gb_graph_draw_tile(gb_mouse_clickpos.x-0.5, gb_mouse_clickpos.y-0.5,
			0, gb_mouse_tile_x + 1 + ffloor(gb_mouse_clicktime.value),
			gb_mouse_tile_y)
			
		local ux:float
		local uy:float
		if bool_eq(gb_mouse_flashing,true)
			ux = 5 + ffloor(gb_mouse_flashtime.value * 20)
			uy = 19
		else
			ux = 4
			uy = 20
		endif
		
		if number_gt(gb_mouse_idletime,0.5)
			gb_graph_draw_tile(gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y(),
				0, ux, uy)
		elseif number_gt(gb_mouse_idletime,0)
			gb_graph_set_alpha(3 * gb_mouse_idletime.value*2)
			gb_graph_draw_tile(gb_mouse_virtual_tile_x(), gb_mouse_virtual_tile_y(),
				0, ux, uy) 
			gb_graph_set_alpha(3)
		endif
		gb_graph_enable_camera(true)
	endif
endfunction
