rem
                                       
      MMMMMM            MMMMMM                              
      MMMooo            MMMooo                              
      MMM               MMM                                 
         ++++++      ++++++                                 
         ---+++oooooohhh---                                 
            +++MMMMMMMMM                                    
            ++++++MMMMMM                                    
            +++hhhoooooo---         ///---///---///         
            +++MMM      +++         yyy///yyy///yyy         
      ++++++MMM   MMMMMM   MMM   ///MMMyyyMMMyyyMMM///      
      ------ooooooMMMooo   MMM   ooommm///ooo///mmmooo---   
               MMMMMM      MMM   yyyyyy         yyyyyy///   
         MMMMMM      :::   MMM   MMM   yyyyyyMMM   MMMyyy   
         oooooo.........---MMM   mmm   mmmyyymmm   mmmooo   
               ::::::   +++MMM   yyy   MMMyyyyyy   yyy///   
      ++++++         +++MMM      ///yyyyyyyyy///   MMMyyy   
      ---+++---------hhhMMMooo   ---ooooooooo---///mmmooo   
         ++++++++++++MMMMMMMMM      /////////   yyyyyy///   
               ++++++MMMMMMMMM               yyyMMM///      
               ---+++hhhMMMMMM   ///ooo///oooyyyhhhooo      
                  ++++++MMMMMM   yyyMMMyyyMMMyyy///yyy      
                     ++++++MMMMMM   yyy///yyy///            
                     ------oooooo   ///---///---            
                                                            
DEVELOPED BY MR.GOOBER - 2020-01-31
goober was here, 2020-06-28
endrem

''''''''''''''
'' includes ''
''''''''''''''

'' objects
include "objects/types.bmx"
include "objects/point.bmx"
include "objects/point3.bmx"
include "objects/number.bmx"
include "objects/rect.bmx"
include "objects/cube.bmx"
include "objects/font.bmx"
include "objects/ibox.bmx"
include "objects/list.bmx"
include "objects/animframe.bmx"
include "objects/animation.bmx"
include "objects/stopwatch.bmx"
include "objects/color.bmx"
include "objects/palette.bmx"
include "objects/timer.bmx"
include "objects/dictval.bmx"
include "objects/dict.bmx"
include "objects/tag.bmx"
include "objects/tilemap.bmx"
include "objects/counter.bmx"
include "objects/sprite.bmx"
include "objects/button.bmx"
include "objects/bitmap.bmx"
include "objects/matrix.bmx"
include "objects/jake.bmx"
include "objects/gbscmd.bmx"
include "objects/spinicon.bmx"
include "objects/lightcube.bmx"
include "objects/image.bmx"

'' ~~unfinished modules~~
include "objects/drawcmd.bmx"
include "objects/drawstack.bmx"
include "objects/emitter.bmx" 

'' components
include "components/functions.bmx"
include "components/visual.bmx"
include "components/audio.bmx"
include "components/timing.bmx"
include "components/controller.bmx"
include "components/camera.bmx"
include "components/gbml.bmx"
include "components/scene.bmx"
include "components/pulser.bmx"
include "components/graph.bmx"
include "components/console.bmx"
include "components/achievements.bmx"
include "components/mouse.bmx"
include "components/gbs.bmx"
include "components/net.bmx"
include "components/transition.bmx"
include "components/testing.bmx"
include "components/debug.bmx"

'' editor
include "editor/editor.bmx"

''''''''''''
'' consts ''
''''''''''''

Const dir_north 		:Byte 	= 1			'' 	Y-
Const dir_east 			:Byte 	= 2			'' 	X+
Const dir_south 		:Byte 	= 3			''	Y+
Const dir_west 			:Byte 	= 4			''	X-
Const dir_up				:Byte		= 5			''	Z-
Const dir_down			:Byte		= 6			''	Z+

Const max_byte			:Byte		= %11111111
Const max_short			:Short	= %1111111111111111
Const max_int				:Int		= %01111111111111111111111111111111
const max_float			:float	= 3e37

Const undefined			:Int		= -1
Const asciistring		:String	= " !~q#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~~"

''''''''''''''''''''''''
'' GB-specific consts ''
''''''''''''''''''''''''

const gb_font_mono 					:int 			= 0
const gb_font_mini 					:int 			= 1
const gb_font_dlog					:int 			= 2

Const gb_max_images					:Int			= 100
Const gb_max_fonts					:Int			= 10
Const gb_max_sounds					:Int			= 100
const gb_max_animations			:int			= 100
const gb_max_palettes				:int			= 10

Const	gb_tilesize						:Float		= 10.0
Const gb_version						:String		= "0.2.00"

const gb_autosave_time_max	:float 		= 30.0
const gb_gravity						:float		= 0.25

'''''''''''''
'' Globals ''
'''''''''''''

global gb_running 		:t_bool
global gb_paused 			:t_bool
global gb_speed 			:t_number

global gb_fonts				:t_font[]

global gb_images				:timage[]
global gb_image_paths		:string[]

global gb_sounds				:tsound[]
global gb_sound_paths		:string[]

global gb_palettes		:t_palette[]
global gb_animations	:t_animation[]


global gb_settings					:t_dict
global gb_autosave_timer		:t_timer

'''''''''''''''
'' Functions ''
'''''''''''''''

function gb_stop()
	bool_set(gb_running, false)
endfunction

function gb_add_font(n:int, f:t_font)
	n = iwrap(n,0,gb_max_fonts-1)
	gb_fonts[n] = f
endfunction

function gb_add_image(n:int, f:string)
	n = iwrap(n,0,gb_max_images-1)
	gb_images[n] = loadimage(f,dynamicimage)
	if gb_images[n] then gb_image_paths[n] = f
endfunction

function gb_add_sound(n:int, f:string)
	n = iwrap(n,0,gb_max_sounds-1)
	gb_sounds[n] = loadsound(f,0)
	if gb_sounds[n] then gb_sound_paths[n] = f
endfunction

function gb_set_speed(n:float)
	number_set(gb_speed, n)
endfunction

function gb_pause(b:byte=true)
	bool_set( gb_paused, bool(b) )
endfunction

function gb_reload_images()
	for local i:int = 0 to gb_max_images-1
		if gb_image_paths[i]
			gb_add_image(i,gb_image_paths[i])
			gb_console_out("Reloaded [" + i + "]:" + gb_image_paths[i])
		endif
	next
endfunction

function gb_reload_sounds()
	for local i:int = 0 to gb_max_sounds-1
		if gb_sound_paths[i]
			gb_add_sound(i,gb_sound_paths[i])
			gb_console_out("Reloaded [" + i + "]:" + gb_sound_paths[i])
		endif
	next
endfunction

'''''''''''''''''''''''''''''''''''''''''''
'' saving and loading GBML settings file ''
'''''''''''''''''''''''''''''''''''''''''''

function gb_load_settings()
	local u:t_dict = gbml_load("settings")
	if (u)
		gb_settings = u
	else
		gb_settings = new_dict()
	endif
endfunction

function gb_save_settings()
	gbml_save(gb_settings,"settings")
endfunction

''''''''''''''''''''''''''''
'' loading default assets ''
''''''''''''''''''''''''''''

function gb_load_default_images()
	gb_add_image(0,"gb/media/images/gb-0.png")
endfunction

function gb_load_default_sounds()
	gb_add_sound(90,"gb/media/sounds/jake-1.ogg")
	gb_add_sound(91,"gb/media/sounds/jake-2.ogg")
	gb_add_sound(92,"gb/media/sounds/jake-3.ogg")
	gb_add_sound(93,"gb/media/sounds/jake-4.ogg")
endfunction

function gb_load_default_fonts()
	gb_fonts[0] = new_font(0,0,0,10,10,1,1)
	gb_fonts[1] = new_font(0,0,10,10,10,0.5,1)
	gb_fonts[2] = new_font(0,30,2,10,10,1,2)
	
	gb_fonts[2].spacing = ..
	[ .6, .3, .4, .7, .6, .7, .8, .2, .5, .5,
		.6, .6, .3, .6, .3, .7, .6, .6, .6, .6,
		.6, .6, .6, .6, .6, .6, .3, .3, .8, .6,
		.8, .6, .8, .6, .6, .6, .6, .6, .6, .6,
		.6, .5, .6, .6, .5, .7, .7, .6, .6, .7,
		.6, .6, .7, .6, .7, .7, .7, .7, .6, .5,
		.7, .5, .7, .6, .5, .7, .6, .6, .6, .6,
		.7, .6, .6, .3, .4, .6, .3, .9, .6, .6,
		.6, .6, .6, .6, .7, .6, .7, .9, .7, .6,
		.7, .5, .3, .5, .6, .1, .1, .1, .1, .1 ]
		
endfunction

function gb_load_default_animations()
	local a:t_animation
	
	a = new_animation(4)
	animation_add_new_frame(a, 0, 0,19,22,2,2, 0.125)
	animation_add_new_frame(a, 1, 0,21,22,2,2, 0.125)
	animation_add_new_frame(a, 2, 0,23,22,2,2, 0.125)
	animation_add_new_frame(a, 3, 0,25,22,2,2, 0.125)
	gb_animations[0] = a
	
	a = new_animation(5)
	animation_add_new_frame(a, 0, 0,5,20,1,1, 0.05)
	animation_add_new_frame(a, 1, 0,6,20,1,1, 0.05)
	animation_add_new_frame(a, 2, 0,7,20,1,1, 0.05)
	animation_add_new_frame(a, 3, 0,8,20,1,1, 0.05)
	animation_add_new_frame(a, 4, 0,9,20,1,1, 0.05)
	gb_animations[1] = a
endfunction

function gb_load_default_palettes()
	gb_palettes[0] = new_palette()
	palette_set(gb_palettes[0], 0,0,0,0,3)
	palette_set(gb_palettes[0], 1,1,1,1,3)
	palette_set(gb_palettes[0], 2,2,2,2,3)
	palette_set(gb_palettes[0], 3,3,3,3,3)
	palette_set(gb_palettes[0], 4,2,1,0,3)
	palette_set(gb_palettes[0], 5,3,2,0,3)
	palette_set(gb_palettes[0], 6,0,1,0,3)
	palette_set(gb_palettes[0], 7,1,2,0,3)
	palette_set(gb_palettes[0], 8,0,1,2,3)
	palette_set(gb_palettes[0], 9,0,2,3,3)
	palette_set(gb_palettes[0],10,2,0,1,3)
	palette_set(gb_palettes[0],11,3,0,2,3)
endfunction



''''''''''''
'' Events ''
''''''''''''

function gb_init()
	gb_running 		= new_bool(true)
	gb_paused 		= new_bool(false)
	gb_speed 			= new_number(1.0,0.0,10.0)
	
	gb_fonts					= new t_font				[gb_max_fonts]
	gb_images					= new timage				[gb_max_images]
	gb_sounds					= new tsound				[gb_max_sounds]
	gb_animations			= new t_animation		[gb_max_animations]
	gb_palettes				= new t_palette			[gb_max_palettes]
	gb_autosave_timer	= new_timer(gb_autosave_time_max)
	
	gb_image_paths		= new string[gb_max_images]
	gb_sound_paths		= new string[gb_max_sounds]
	
	gb_console_init()
	gb_timing_init()
	gb_audio_init()
	gb_controller_init()
	gb_mouse_init()
	gb_camera_init()
	gb_pulser_init()
	gb_visual_init()
	gb_graph_init()
	gb_camera_init()
	gb_scene_init()
	gb_net_init()
	gbs_init()
	
	gb_testing_init()
	gb_debug_init()
endfunction

function gb_load()
	gb_load_settings()
	gb_achievements_load()
	
	gb_console_load()
	gb_visual_load()
	
	gb_load_default_palettes()
	gb_load_default_fonts()
	gb_load_default_animations()
	gb_load_default_sounds()
	gb_load_default_images()
	
	gb_testing_load()
	gb_debug_load()
endfunction

function gb_start()
	gb_testing_start()
	gb_debug_start()
endfunction

function gb_update(d:float=0.0)
	gb_timing_update	()
	
	local e:float = gb_get_deltatime()
	gb_console_update	(e)
	gb_visual_update	(e)
	
	gb_mouse_update		(e)
	gb_pulser_update	(e)
	
	if bool_eq(gb_console_enabled,false)
		gb_controller_update(e)
		gb_scene_update(e * gb_speed.value)
	endif
	
	timer_update(gb_autosave_timer,e)
	if timer_finished(gb_autosave_timer)
		gb_save_settings()
		timer_reset(gb_autosave_timer)
	endif
	
	gb_testing_update	(e)
	gb_debug_update		(e)
endfunction

function gb_draw(x:float=0, y:float=0)
	gb_debug_draw_bg	(x,y)
	gb_scene_draw			(x,y)
	gb_testing_draw		(x,y)
	gb_visual_draw		(x,y)
	gb_debug_draw_fg	(x,y)
	gb_console_draw		(x,y)
	gb_mouse_draw			(x,y)
endfunction

function gb_end()
	gb_visual_end				()
	gb_testing_end			()
	gb_debug_end				()
	gb_save_settings		()
	gb_achievements_end	()
endfunction