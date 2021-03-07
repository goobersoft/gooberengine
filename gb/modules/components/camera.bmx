
''''''''''''
'' consts ''
''''''''''''

const gb_camera_center_x :float = 20
const gb_camera_center_y :float = 12

'''''''''''''
'' globals ''
'''''''''''''

global gb_camera_pos    :t_point3
global gb_camera_angle  :t_point3

''''''''''''
'' events ''
''''''''''''

function gb_camera_init()
  gb_camera_pos = new_point3(0,0,0)
  gb_camera_angle = new_point3(0,0,0)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_camera_set_pos(x:float, y:float)
	gb_camera_pos.x = x
	gb_camera_pos.y = y
endfunction

function gb_camera_add_pos(x:float, y:float)
	gb_camera_pos.x = gb_camera_pos.x + x
	gb_camera_pos.y = gb_camera_pos.y + y
endfunction

function gb_camera_set_pos3d(x:float, y:float, z:float)
	gb_camera_pos.x = x
	gb_camera_pos.y = y
	gb_camera_pos.z = z
endfunction

function gb_camera_add_pos3d(x:float, y:float, z:float)
	gb_camera_pos.x = gb_camera_pos.x + x
	gb_camera_pos.y = gb_camera_pos.y + y
	gb_camera_pos.z = gb_camera_pos.z + z
endfunction

function gb_camera_set_angle(z:float)
	gb_camera_angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle(z:float)
	gb_camera_angle.z = wrap(gb_camera_angle.z+z,0,1000)
endfunction

function gb_camera_set_angle3d(x:float, y:float, z:float)
	gb_camera_angle.x = wrap(x,0,1000)
	gb_camera_angle.y = wrap(y,0,1000)
	gb_camera_angle.z = wrap(z,0,1000)
endfunction

function gb_camera_add_angle3d(x:float, y:float, z:float)
	gb_camera_angle.x = wrap(gb_camera_angle.x+x,0,1000)
	gb_camera_angle.y = wrap(gb_camera_angle.y+y,0,1000)
	gb_camera_angle.z = wrap(gb_camera_angle.z+z,0,1000)
endfunction

function gb_camera_reset()
	point3_set(gb_camera_pos,0,0,0)
	point3_set(gb_camera_angle,0,0,0)
endfunction

function gb_camera_reset_pos()
	point3_set(gb_camera_pos,0,0,0)
endfunction

function gb_camera_reset_angle()
	point3_set(gb_camera_angle,0,0,0)
endfunction