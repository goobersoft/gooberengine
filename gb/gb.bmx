Rem
                                       
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
                                                            
DEVELOPED BY MR.GOOBER    2020-01-31
goober was here           2020-06-28
goober was here again     2020-12-04
endrem

''''''''''''''
'' includes ''
''''''''''''''

'' core modules
Include "modules/core/types.bmx"
Include "modules/core/functions.bmx"
Include "modules/core/point.bmx"
Include "modules/core/point3.bmx"
Include "modules/core/number.bmx"
Include "modules/core/rect.bmx"
Include "modules/core/cube.bmx"
Include "modules/core/font.bmx"
Include "modules/core/ibox.bmx"
include "modules/core/list_iterator.bmx"
include "modules/core/list_node.bmx"
include "modules/core/list.bmx"
Include "modules/core/stopwatch.bmx"
Include "modules/core/color.bmx"
Include "modules/core/palette.bmx"
Include "modules/core/timer.bmx"
Include "modules/core/dict_val.bmx"
Include "modules/core/dict.bmx"
Include "modules/core/counter.bmx"
Include "modules/core/canvas.bmx"
Include "modules/core/image.bmx"
Include "modules/core/sound.bmx"
Include "modules/core/logchain.bmx"
include "modules/core/drawstack_cmd.bmx"
Include "modules/core/drawstack.bmx"
Include "modules/core/pulser.bmx"
Include "modules/core/array.bmx"
Include "modules/core/scene.bmx"
include "modules/core/mstring.bmx"

'' gfx modules
Include "modules/gfx/animation.bmx"
Include "modules/gfx/lightcube.bmx"
Include "modules/gfx/spinicon.bmx"
Include "modules/gfx/jake.bmx"
Include "modules/gfx/tilemap.bmx"
Include "modules/gfx/sprite.bmx"
include "modules/gfx/model.bmx"
include "modules/gfx/tri.bmx"
Include "modules/gfx/transition.bmx"

'' ui objects
Include "modules/ui/button.bmx"


'' object components
Include "modules/components/visual.bmx"
Include "modules/components/audio.bmx"
Include "modules/components/timing.bmx"
Include "modules/components/controller.bmx"
Include "modules/components/camera.bmx"
Include "modules/components/gbml.bmx"
Include "modules/components/graph.bmx"
include "modules/components/canvas.bmx"
Include "modules/components/console.bmx"
Include "modules/components/mouse.bmx"
Include "modules/components/gbs.bmx"
Include "modules/components/assets.bmx"
Include "modules/components/debug.bmx"
include "modules/components/world_entity.bmx"
include "modules/components/world.bmx"

'' debugging
Include "modules/debugging/testing.bmx"

'' editor
Include "modules/editor/editor.bmx"



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
Const max_float			:Float	= 3e37

Const undefined			:Int		= -1
Const asciistring		:String	= " !~q#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~~"

''''''''''''''''''''''''
'' GB-specific consts ''
''''''''''''''''''''''''

Const gb_font_mono 					:Int 			= 0
Const gb_font_mini 					:Int 			= 1
Const gb_font_dlog					:Int 			= 2

Const gb_max_images					:Int			= 100
Const gb_max_fonts					:Int			= 10
Const gb_max_sounds					:Int			= 100
Const gb_max_animations			:Int			= 100
Const gb_max_palettes				:Int			= 10

Const	gb_tilesize						:Float		= 10.0
Const gb_version						:String		= "0.2.00"

Const gb_autosave_time_max	:Float 		= 30.0
Const gb_gravity						:Float		= 0.25

const gb_bool   :string = "bool"
const gb_byte   :string = "byte"
const gb_short  :string = "short"
const gb_int    :string = "int"
const gb_long   :string = "long"
const gb_float  :string = "float"
const gb_double :string = "double"
const gb_string :string = "string"

'''''''''''''
'' globals ''
'''''''''''''

global gb_running         :t_bool
global gb_paused          :t_bool
global gb_settings        :t_dict
global gb_pulser          :t_pulser
global gb_autosave_timer  :t_timer
global gb_scene           :t_scene

''''''''''''
'' events ''
''''''''''''

Function gb_init()
  gb_running        = new_bool(true)
  gb_paused         = new_bool(false)
  gb_settings       = null
  gb_autosave_timer = new_timer(60)
  gb_pulser         = new_pulser()

  gb_timing_init()
  gb_assets_init()
  gb_audio_init()
  gb_visual_init()
  gb_graph_init()
  gb_canvas_init()
  gb_mouse_init()
  gb_debug_init()
  gb_camera_init()
  gb_console_init()
  gb_controller_init()
  gbs_init()

  gb_editor_init()
  gb_testing_init()
EndFunction

Function gb_load()
  gb_load_settings()
  
  gb_assets_load()
	gb_visual_load()
	gb_assets_load()
	gb_debug_load()

  gb_editor_load()
	gb_testing_load()
EndFunction

Function gb_start()
	gb_testing_start()
	gb_debug_start()
EndFunction

Function gb_update()
	gb_timing_update	()
  gb_console_update	()
  gb_visual_update	()
  gb_canvas_update  ()
  gb_mouse_update		()
  gb_controller_update()
  If gb_scene Then scene_update(gb_scene)
	
	timer_update(gb_autosave_timer)
	If timer_finished(gb_autosave_timer)
		gb_save_settings()
		timer_reset(gb_autosave_timer)
	EndIf

  pulser_update(gb_pulser)
  gb_editor_update  ()
	gb_testing_update	()
	gb_debug_update		()
EndFunction

Function gb_draw_1()
  gb_graph_draw     ()
	gb_debug_draw_bg	()
	If gb_scene Then scene_draw(gb_scene)
  gb_canvas_draw    ()
  gb_editor_draw    ()
	gb_testing_draw		()
	gb_visual_draw		()
endfunction

function gb_draw_2()
	gb_debug_draw_fg	()
	gb_mouse_draw			()
EndFunction

Function gb_end()
	gb_visual_end				()
	gb_testing_end			()
	gb_debug_end				()
	gb_save_settings		()
EndFunction

'''''''''''''''
'' Functions ''
'''''''''''''''

Function gb_stop()
	bool_set(gb_running, False)
EndFunction

Function gb_pause(b:Byte=True)
	bool_set( gb_paused, bool(b) )
EndFunction

function gb_switch_scene(s:t_scene)
  scene_end(gb_scene)
  gb_scene = s
  scene_init(gb_scene)
endfunction

'''''''''''''''''''''''''''''''''''''''''''
'' saving and loading GBML settings file ''
'''''''''''''''''''''''''''''''''''''''''''

Function gb_load_settings:t_dict ()
	Local u:t_dict = gbml_load("settings")
	If (u)
    gb_settings = u
  Else
    gb_settings = new_dict()
  EndIf
EndFunction

Function gb_save_settings()
	gbml_save(gb_settings,"settings")
EndFunction
