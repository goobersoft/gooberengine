
''''''''''''
'' consts ''
''''''''''''

Const gb_controller_max_keymaps 	:Int	= 12
Const gb_controller_max_buttons		:Int	= 12

Const gb_controller_button_up			:Byte	= 0
Const gb_controller_button_right	:Byte	= 1
Const gb_controller_button_down		:Byte	= 2
Const gb_controller_button_left		:Byte	= 3
Const gb_controller_button_a			:Byte	= 4
Const gb_controller_button_b			:Byte	= 5
Const gb_controller_button_c			:Byte	= 6
Const gb_controller_button_d			:Byte	= 7
Const gb_controller_button_select	:Byte	= 8
Const gb_controller_button_start	:Byte = 9
Const gb_controller_button_back		:Byte	= 10
Const gb_controller_button_shift	:Byte	= 11

'''''''''''''
'' globals ''
'''''''''''''

type t_gb_controller
  field keymaps   :t_byte[][]
  field buttons   :t_byte[]
endtype

function new_gb_controller:t_gb_controller ()
  local r:t_gb_controller = new t_gb_controller

  r.keymaps = new t_byte[gb_controller_max_keymaps][]

  for local i:int = 0 to gb_controller_max_keymaps-1
    r.keymaps[i] = [new_byte(0), new_byte(0)]
  next

  r.buttons = new t_byte[gb_controller_max_buttons]
  for local i:int = 0 to gb_controller_max_buttons-1
    r.buttons[i] = new_byte(0)
  next

  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_controller_init()
  gb.controller = new_gb_controller()
	gb_controller_set_default_keymaps()
endfunction

function gb_controller_update()
	for local i:int = 0 to gb_controller_max_buttons-1
		if keydown(gb.controller.keymaps[i][0].value) or keydown(gb.controller.keymaps[i][1].value)
      gb_controller_press_button(i)
		else
			gb_controller_release_button(i)
		endif
	next
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_controller_set_default_keymaps()
	byte_set(gb.controller.keymaps[0][0],  key_up)
	byte_set(gb.controller.keymaps[1][0],  key_right)
	byte_set(gb.controller.keymaps[2][0],  key_down)
	byte_set(gb.controller.keymaps[3][0],  key_left)
	byte_set(gb.controller.keymaps[4][0],  key_z)
	byte_set(gb.controller.keymaps[5][0],  key_x)
	byte_set(gb.controller.keymaps[6][0],  key_c)
	byte_set(gb.controller.keymaps[7][0],  key_v)
	byte_set(gb.controller.keymaps[8][0],  key_space)
	byte_set(gb.controller.keymaps[9][0],  key_return)
	byte_set(gb.controller.keymaps[10][0], key_escape)
	byte_set(gb.controller.keymaps[11][0], key_lshift)
	byte_set(gb.controller.keymaps[11][1], key_rshift)
endfunction

function gb_controller_press_button(n:int)
	n = iclamp(n,0,gb_controller_max_buttons-1)
	if (gb.controller.buttons[n].value = 0)
		gb.controller.buttons[n].value = 2
	else
		gb.controller.buttons[n].value = 1
	endif
endfunction

function gb_controller_release_button(n:int)
	n = iclamp(n,0,gb_controller_max_buttons-1)
	gb.controller.buttons[n].value = 0
endfunction

function gb_controller_tostring:string()
	local t:string = ""
	for local i:int = 0 to gb_controller_max_buttons-1
		t = t + gb.controller.buttons[i].value
	next
	return t
endfunction

function gb_controller_get_button:int(n:int)
	return gb.controller.buttons[clamp(n,0,gb_controller_max_buttons-1)].value
endfunction

function gb_controller_get_button_pressed:int(n:int)
	return (gb.controller.buttons[clamp(n,0,gb_controller_max_buttons-1)].value = 2)
endfunction

function gb_controller_get_button_held:int(n:int)
	return (gb.controller.buttons[clamp(n,0,gb_controller_max_buttons-1)].value > 0)
endfunction

