
type t_sprite
	field pos 		:t_point3
	field ibox		:t_ibox
	field handle	:t_point
	field angle		:t_number
	field scale		:t_point
	field color		:t_color
	field mode		:t_byte
	field anim		:t_animation
endtype

function new_sprite:t_sprite(im:int=0, ix:float=39, iy:int=0, iw:int=1, ih:int=1)
	local r:t_sprite = new t_sprite
	r.pos 			= new_point3(0,0,0)
	r.ibox			= new_ibox(im,ix,iy,iw,ih)
	r.handle		= new_point(0,0)
	r.angle			= new_number(0,0,1000,true)
	r.scale			= new_point(1,1)
	r.color			= new_color(3,3,3,3)
	r.mode			= new_byte(gb_graph_mode_normal)
	r.anim			= null
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function sprite_move(s:t_sprite, x:float, y:float, z:float=0)
	point3_set(s.pos, x, y, z)
endfunction
global sprite_set_pos(s:t_sprite, x:float, y:float, z:float) = sprite_move

function sprite_rotate(s:t_sprite, a:float)
	number_set(s.angle, a)
endfunction

function sprite_set_color(s:t_sprite, r:float, g:float, b:float)
	color_set_rgb(s.color, r, g, b)
endfunction

function sprite_set_alpha(s:t_sprite, a:float)
	color_set_alpha(s.color, a)
endfunction

function sprite_set_mode(s:t_sprite, m:byte)
	byte_set(s.mode, m)
endfunction

function sprite_set_scale(s:t_sprite, x:float=1, y:float=1)
	point_set(s.scale, x, y)
endfunction

function sprite_set_handle(s:t_sprite, x:float=0, y:float=0)
	point_set(s.handle, x, y)
endfunction

function sprite_set_ibox(s:t_sprite, im:int, ix:float, iy:float, iw:float, ih:float)
	ibox_set(s.ibox, im,ix,iy,iw,ih)
endfunction

''''''''''''
'' events ''
''''''''''''

function sprite_update(s:t_sprite, d:float)
	if (s.anim)
		animation_update(s.anim,d)
	endif
endfunction

function sprite_draw(s:t_sprite, x:float=0, y:float=0)
	gb_graph_set_handle(s.handle.x, s.handle.y)
	gb_graph_set_scale(s.scale.x, s.scale.y)
	gb_graph_set_angle(s.angle.value)
	gb_graph_set_color(s.color.r, s.color.g, s.color.b)
	gb_graph_set_alpha(s.color.a)
	gb_graph_set_mode(s.mode.value)
	
	if (s.anim)
		local u:t_ibox = animation_get_current_frame(s.anim)
		if (u) then sprite_set_ibox(s, u.id, u.x, u.y, u.w, u.h)
	endif
	gb_graph_draw_tile_box(s.pos.x+x, s.pos.y+y, s.ibox.id, s.ibox.x,
		s.ibox.y, s.ibox.w, s.ibox.h)
	gb_graph_reset()
endfunction

function sprite_draw3d(s:t_sprite, x:float=0, y:float=0, z:float=0)
	gb_graph_set_handle(s.handle.x, s.handle.y)
	gb_graph_set_scale(s.scale.x, s.scale.y)
	gb_graph_set_angle(s.angle.value)
	gb_graph_set_color(s.color.r, s.color.g, s.color.b)
	gb_graph_set_alpha(s.color.a)
	gb_graph_set_mode(s.mode.value)
	
	if (s.anim)
		local u:t_ibox = animation_get_current_frame(s.anim)
		if (u) then sprite_set_ibox(s, u.id, u.x, u.y, u.w, u.h)
	endif
	gb_graph_draw_tile_box3d(s.pos.x+x, s.pos.y+y, s.pos.z+z, s.ibox.id,
		s.ibox.x, s.ibox.y, s.ibox.w, s.ibox.h)
	gb_graph_reset()
endfunction