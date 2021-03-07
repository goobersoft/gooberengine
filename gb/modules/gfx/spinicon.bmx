type t_spinicon
	field pos 				:t_point
	field angle 			:t_number
	field speed				:t_number
	field iconpos			:t_point3
	field iconlength	:t_int
  field fade        :t_number
  field fadespeed   :t_number
  field particles   :t_list
endtype

function new_spinicon:t_spinicon(im:int=0, tx:float=30, ty:float=23, l:int=10)
	local r:t_spinicon = new t_spinicon
	r.pos 				= new_point(0,0)
	r.angle 			= new_number_angle(0)
	r.speed 			= new_number(3000,250,3000)
	r.iconpos 		= new_point3(im,tx,ty)
	r.iconlength 	= new_int(l)
  r.fade        = new_number(1000,0,1000)
  r.fadespeed   = new_number(0,-1000,1000)
  r.particles   = new_list()
	return r
endfunction

function spinicon_update(s:t_spinicon)
	number_add(s.angle, s.speed.value * gb_delta())
  number_add(s.fade, gb_delta()*number_get(s.fadespeed))
  number_add(s.speed, -1000*gb_delta())
  local v:float = (s.speed.value / 3000) * 50
  if prob(v)
    local q:t_sprite = new_sprite()
    sprite_set_pos(q, s.pos.x, s.pos.y-0.5, 0)
    sprite_set_handle(q, 0.5,0.5)
    sprite_write_data(q,"angle",new_float(frand(0,1000)))
    sprite_write_data(q,"life",new_float(1))
    sprite_set_ibox(q,0,29,21+rand(0,2),1,1)
    list_add_last(s.particles,"sprite",q)
  endif

  local u:t_sprite
  local f:t_float
  repeat
    u = t_sprite(list_iterate(s.particles))
    if (u)
      f = t_float(sprite_read_data(u,"angle"))
      sprite_move_pos(u,rot_x(float_get(f))*gb_delta(2), rot_y(float_get(f))*gb_delta(2),0)
      f = t_float(sprite_read_data(u,"life"))
      float_add(f,-gb_delta())
      if float_lte(f,0)
        list_remove_first(s.particles)
      else
        sprite_set_alpha(u,float_get(f)*1.5)
      endif
    endif
  until u = null

endfunction

function spinicon_draw(s:t_spinicon, x:float=0, y:float=0)
	gb_graph_set_handle(gb_graph_scale.x/2, gb_graph_scale.y/2)
	gb_graph_set_angle( s.angle.value )
  gb_graph_set_alpha( s.fade.value/1000 * 3 )
	for local i:int = 0 to s.iconlength.value-1
		gb_graph_draw_tile(s.pos.x+x, s.pos.y+y-(0.1*float(i)),
			int(s.iconpos.x), s.iconpos.y+float(i), s.iconpos.z)
	next
	gb_graph_reset()
  
  local u:t_sprite
  local f:float
  repeat
    u = t_sprite(list_iterate(s.particles))
    if (u)
      sprite_draw(u)
    endif
  until u = null
endfunction

function spinicon_fade_out(s:t_spinicon, f:byte=false)
  number_set(s.fadespeed, -1000)
  if f=true then number_set(s.fade,1000)
endfunction

function spinicon_fade_in(s:t_spinicon, f:byte=false)
  number_set(s.fadespeed, 1000)
  if f=true then number_set(s.fade,0)
endfunction

function spinicon_spin(s:t_spinicon, f:float=3000.00)
  number_set(s.speed, f)
endfunction

function spinicon_set_pos(s:t_spinicon, x:float, y:float)
  point_set(s.pos, x, y)
endfunction