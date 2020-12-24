
''''''''''''
'' consts ''
''''''''''''

const gb_camera_center_x :float = 20
const gb_camera_center_y :float = 12

'''''''''''''
'' globals ''
'''''''''''''

type t_gb_camera
	field pos 		:t_point3
	field angle 	:t_point3
endtype

function new_gb_camera:t_gb_camera ()
  local r:t_gb_camera = new t_gb_camera
  r.pos     = new_point3(0,0,0)
  r.angle   = new_point3(0,0,0)
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_camera_init()
	gb.camera       = new_gb_camera()
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_camera_set_pos(x:float, y:float)
	gb.camera.pos.x = x
	gb.camera.pos.y = y
endfunction

function gb_camera_add_pos(x:float, y:float)
	gb.camera.pos.x = gb.camera.pos.x + x
	gb.camera.pos.y = gb.camera.pos.y + y
endfunction

function gb_camera_set_pos3d(x:float, y:float, z:float)
	gb.camera.pos.x = x
	gb.camera.pos.y = y
	gb.camera.pos.z = z
endfunction

function gb_camera_add_pos3d(x:float, y:float, z:float)
	gb.camera.pos.x = gb.camera.pos.x + x
	gb.camera.pos.y = gb.camera.pos.y + y
	gb.camera.pos.z = gb.camera.pos.z + z
endfunction

function gb_camera_set_angle(z:float)
	gb.camera.angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle(z:float)
	gb.camera.angle.z = wrap(gb.camera.angle.z+z,0,1000)
endfunction

function gb_camera_set_angle3d(x:float, y:float, z:float)
	gb.camera.angle.x = wrap(x,0,1000)
	gb.camera.angle.y = wrap(y,0,1000)
	gb.camera.angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle3d(x:float, y:float, z:float)
	gb.camera.angle.x = wrap(gb.camera.angle.x+x,0,1000)
	gb.camera.angle.y = wrap(gb.camera.angle.y+y,0,1000)
	gb.camera.angle.z = wrap(gb.camera.angle.z+z,0,1000)
endfunction

function gb_camera_reset()
	point3_set(gb.camera.pos,0,0,0)
	point3_set(gb.camera.angle,0,0,0)
endfunction

function gb_camera_reset_pos()
	point3_set(gb.camera.pos,0,0,0)
endfunction

function gb_camera_reset_angle()
	point3_set(gb.camera.angle,0,0,0)
endfunction