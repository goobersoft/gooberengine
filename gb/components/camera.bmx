
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

global gb_camera :t_gb_camera

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_camera_set_pos(x:float, y:float)
	gb_camera.pos.x = x
	gb_camera.pos.y = y
endfunction

function gb_camera_add_pos(x:float, y:float)
	gb_camera.pos.x = gb_camera.pos.x + x
	gb_camera.pos.y = gb_camera.pos.y + y
endfunction

function gb_camera_set_pos3d(x:float, y:float, z:float)
	gb_camera.pos.x = x
	gb_camera.pos.y = y
	gb_camera.pos.z = z
endfunction

function gb_camera_add_pos3d(x:float, y:float, z:float)
	gb_camera.pos.x = gb_camera.pos.x + x
	gb_camera.pos.y = gb_camera.pos.y + y
	gb_camera.pos.z = gb_camera.pos.z + z
endfunction

function gb_camera_set_angle(z:float)
	gb_camera.angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle(z:float)
	gb_camera.angle.z = wrap(gb_camera.angle.z+z,0,1000)
endfunction

function gb_camera_set_angle3d(x:float, y:float, z:float)
	gb_camera.angle.x = wrap(x,0,1000)
	gb_camera.angle.y = wrap(y,0,1000)
	gb_camera.angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle3d(x:float, y:float, z:float)
	gb_camera.angle.x = wrap(gb_camera.angle.x+x,0,1000)
	gb_camera.angle.y = wrap(gb_camera.angle.y+y,0,1000)
	gb_camera.angle.z = wrap(gb_camera.angle.z+z,0,1000)
endfunction



function gb_camera_reset()
	point3_set(gb_camera.pos,0,0,0)
	point3_set(gb_camera.angle,0,0,0)
endfunction

function gb_camera_reset_pos()
	point3_set(gb_camera.pos,0,0,0)
endfunction

function gb_camera_reset_angle()
	point3_set(gb_camera.angle,0,0,0)
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_camera_init()
	gb_camera 			= new t_gb_camera()
	gb_camera.pos   = new_point3(0,0,0)
	gb_camera.angle = new_point3(0,0,0)
endfunction