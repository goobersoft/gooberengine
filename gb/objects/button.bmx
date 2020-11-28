
const button_default_tile_id	:int 		= 0
const button_default_tile_x 	:float 	= 20
const button_default_tile_y		:float 	= 12

const button_state_normal			:int = 0
const button_state_hover			:int = 1
const button_state_press			:int = 2

type t_button
	field rect 					:t_rect
	field ibox					:t_ibox
	field color_normal	:t_color
	field color_hover		:t_color
	field color_press		:t_color
	field state					:t_int
	field presstime			:t_number
	field margin				:t_float
	field text					:t_string
endtype

function new_button:t_button(tx:int, ty:int)
	tx = ilow(tx,2)
	ty = ilow(ty,2)
	local r:t_button = new t_button
	r.rect = new_rect(0,0,tx,ty)
	r.ibox = new_ibox(button_default_tile_id, button_default_tile_x,
		button_default_tile_y,1,1)
	r.color_normal 	= new_color(3,3,3,3)
	r.color_hover		= new_color(0,3,0,3)
	r.color_press		= new_color(0,1,0,3)
	r.state					= new_int( button_state_normal )
	r.presstime			= new_number(0,0,0.25)
	r.margin				= new_float(0.5)
	r.text					= new_string("No Text.")
	return r
endfunction

function button_set_pos(b:t_button, x:float, y:float)
	b.rect.x = x
	b.rect.y = y
endfunction

function button_set_size(b:t_button, w:float, h:float)
	b.rect.w = low(fround(w),0)
	b.rect.h = low(fround(h),0)
endfunction

function button_set_imgpos(b:t_button, tx:float, ty:float)
	b.ibox.x = tx
	b.ibox.y = ty
endfunction

function button_set_margin(b:t_button, f:float)
	float_set( b.margin, f )
endfunction

function button_set_text(b:t_button, s:string)
	string_set(b.text, s)
endfunction

function button_pressed:int(b:t_button)
	return int_eq(b.state, button_state_press)
endfunction

''''''''''''
'' events ''
''''''''''''

function button_update(b:t_button, d:float)
	local mx:float 	= gb_mouse_virtual_tile_x()
	local my:float 	= gb_mouse_virtual_tile_y()
	local mc:int 		= gb_mouse_pressed()
	
	number_add(b.presstime, -d)
	
	if point_in_rect( mx, my, b.rect.x+b.margin.value, b.rect.y+b.margin.value,
	b.rect.w-(b.margin.value*2), b.rect.h-(b.margin.value*2) )
		if (mc)
			int_set(b.state, button_state_press)
			number_to_high(b.presstime)
		else
			int_set(b.state, button_state_hover)
		endif
	else
		int_set(b.state, button_state_normal)
	endif
endfunction

function button_draw(b:t_button, x:float=0, y:float=0)
	if int_eq(b.state, button_state_press) or number_gt(b.presstime, 0)
		gb_graph_set_color( b.color_press )
	elseif int_eq(b.state, button_state_hover)
		gb_graph_set_color( b.color_hover )
	else
		gb_graph_set_color( b.color_normal )
	endif
	gb_graph_draw_tile_window( b.rect.x+x, b.rect.y+y, b.ibox.id, b.ibox.x, b.ibox.y, int(b.rect.w), int(b.rect.h), false)
	gb_graph_set_color(3,3,3)
	gb_graph_draw_tile_text(b.rect.x+1,b.rect.y+1,1,b.text.value)
endfunction