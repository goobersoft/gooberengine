
const jake_animspeed :float = 5

Type t_jake
	Field pos					:t_point3
	Field velo				:t_point
	Field smiletime		:t_number
	Field anim				:t_number
	field speechid		:t_int
	field speechamt		:t_float
	field silent			:t_bool
endtype

function new_jake:t_jake()
	local r:t_jake = new t_jake
	r.pos 			= new_point3(40,frand(-1,23),0)
	r.velo			= new_point(-1,0)
	r.smiletime	= new_number(0,0,1.0)
	r.anim			= new_number(frand(0,2),0,2,true)
	r.speechid	= new_int(0)
	r.speechamt	= new_float(0.0)
	r.silent		= new_bool(true)
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function jake_smile(j:t_jake)
	number_to_high(j.smiletime)
	if prob(1)
		int_set( j.speechid, 4 )
	else
		int_set( j.speechid, rand(0,3) )
	endif
	float_set( j.speechamt, 1.0)
	if bool_neq(j.silent)
		if (j.speechid.value = 4)
			gb_audio_play_sound( 93, rand(0,9), 0.5, clamp(0.75 - (0.05*j.pos.z),0,1))
		else
			gb_audio_play_sound( 90+j.speechid.value, rand(0,9), frand(0.8,1.2), clamp(0.75 - (0.05*j.pos.z),0,1))
		endif
	endif
endfunction

''''''''''''
'' events ''
''''''''''''

function jake_update(j:t_jake)
  local d:float = gb_delta()
	number_add( j.anim, d * jake_animspeed )
	number_add( j.smiletime, -d )
	
	local z:float = gb_graph_calc_perspec(j.pos.z)
	local dx:float = 1.0
	local dy:float = 1.0
	if (z <> 0.0)
		dx = ((40.0 / z) - 40) / 2
		dy = ((24.0 / z) - 24) / 2
	endif
	
	if number_eq(j.smiletime, 0.0)
		point3_add(j.pos, j.velo.x*d, j.velo.y*d, 0)
		if j.pos.x <= (-dx-2)
			j.pos.x = 40+dx
			j.pos.y = frand(-dy,24+dy)
		endif
	else
		float_mul( j.speechamt, 0.9 ) 
	endif
endfunction

function jake_draw(j:t_jake, x:float=0, y:float=0)
	if number_gt(j.smiletime, 0.0)
		if (j.speechid.value = 4)
			gb_graph_draw_tile_box( j.pos.x, j.pos.y, 0, 16, 19, 2, 2 )
		else
			gb_graph_draw_tile_box( j.pos.x, j.pos.y, 0, 14, 19, 2, 2 )
		endif
		gb_graph_draw_tile_box( j.pos.x, j.pos.y - 1+j.speechamt.value,
			0, 14+(j.speechid.value*2), 18, 2, 1)
	else
		gb_graph_draw_tile_box( j.pos.x, j.pos.y, 0, 10+(ffloor(j.anim.value)*2), 19, 2, 2 )
	endif
endfunction

function jake_draw3d(j:t_jake, x:float=0, y:float=0, z:float=0)
	if number_gt(j.smiletime, 0.0)
		if (j.speechid.value = 4)
			gb_graph_draw_tile_box3d( j.pos.x, j.pos.y, j.pos.z, 0, 16, 19, 2, 2 )
		else
			gb_graph_draw_tile_box3d( j.pos.x, j.pos.y, j.pos.z, 0, 14, 19, 2, 2 )
		endif
	else
		gb_graph_draw_tile_box3d( j.pos.x, j.pos.y, j.pos.z, 0, 10+(ffloor(j.anim.value)*2), 19, 2, 2 )
	endif
endfunction