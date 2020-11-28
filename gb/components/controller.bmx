
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

global gb_controller_keymaps	:byte[,]
global gb_controller_buttons	:byte[]

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_controller_set_default_keymaps()
	gb_controller_keymaps[0,0]  = key_up
	gb_controller_keymaps[1,0]  = key_right
	gb_controller_keymaps[2,0]  = key_down
	gb_controller_keymaps[3,0]  = key_left
	gb_controller_keymaps[4,0]  = key_z
	gb_controller_keymaps[5,0]  = key_x
	gb_controller_keymaps[6,0]  = key_c
	gb_controller_keymaps[7,0]  = key_v
	gb_controller_keymaps[8,0]  = key_space
	gb_controller_keymaps[9,0]  = key_return
	gb_controller_keymaps[10,0] = key_escape
	gb_controller_keymaps[11,0] = key_lshift
	gb_controller_keymaps[11,1] = key_rshift
endfunction

function gb_controller_press_button(n:int)
	n = iclamp(n,0,gb_controller_max_buttons-1)
	if (gb_controller_buttons[n] = 0)
		gb_controller_buttons[n] = 2
	else
		gb_controller_buttons[n] = 1
	endif
endfunction

function gb_controller_release_button(n:int)
	n = iclamp(n,0,gb_controller_max_buttons-1)
	gb_controller_buttons[n] = 0
endfunction

function gb_controller_tostring:string()
	local t:string = ""
	for local i:int = 0 to gb_controller_max_buttons-1
		t = t + gb_controller_buttons[i]
	next
	return t
endfunction

function gb_controller_get_button:int(n:int)
	return gb_controller_buttons[clamp(n,0,gb_controller_max_buttons-1)]
endfunction

function gb_controller_get_button_pressed:int(n:int)
	return (gb_controller_buttons[clamp(n,0,gb_controller_max_buttons-1)] = 2)
endfunction

function gb_controller_get_button_held:int(n:int)
	return (gb_controller_buttons[clamp(n,0,gb_controller_max_buttons-1)] > 0)
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_controller_init()
	gb_controller_keymaps = new byte[gb_controller_max_keymaps,2]
	gb_controller_buttons = new byte[gb_controller_max_buttons]
	gb_controller_set_default_keymaps()
endfunction

function gb_controller_update(d:float)
	for local i:int = 0 to gb_controller_max_buttons-1
		if keydown(gb_controller_keymaps[i,0]) or keydown(gb_controller_keymaps[i,1])
			gb_controller_press_button(i)
		else
			gb_controller_release_button(i)
		endif
	next
endfunction