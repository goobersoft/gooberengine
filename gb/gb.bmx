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
include "modules/core/list.bmx"
Include "modules/core/stopwatch.bmx"
Include "modules/core/color.bmx"
Include "modules/core/palette.bmx"
Include "modules/core/timer.bmx"
Include "modules/core/dict.bmx"
Include "modules/core/tag.bmx"
Include "modules/core/counter.bmx"
Include "modules/core/canvas.bmx"
Include "modules/core/image.bmx"
Include "modules/core/sound.bmx"
Include "modules/core/logchain.bmx"
Include "modules/core/drawstack.bmx"
Include "modules/core/pulser.bmx"
Include "modules/core/array.bmx"
Include "modules/core/transition.bmx"
Include "modules/core/scene.bmx"
include "modules/core/world.bmx"
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

'' ui objects
Include "modules/ui/button.bmx"


'' object components
Include "modules/comp/visual.bmx"
Include "modules/comp/audio.bmx"
Include "modules/comp/timing.bmx"
Include "modules/comp/controller.bmx"
Include "modules/comp/camera.bmx"
Include "modules/comp/gbml.bmx"
Include "modules/comp/graph.bmx"
Include "modules/comp/console.bmx"
Include "modules/comp/mouse.bmx"
Include "modules/comp/gbs.bmx"
Include "modules/comp/net.bmx"

Include "modules/comp/assets.bmx"

'' debugging
Include "modules/debugging/testing.bmx"
Include "modules/debugging/debug.bmx"

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

''''''''''
'' type ''
''''''''''

Type t_gb
  Field running         :t_bool
  Field paused          :t_bool
  Field speed           :t_number
  Field canvas          :t_canvas
  Field settings        :t_dict
  

  ''  updatables
  Field pulser          :t_pulser
  Field autosave_timer  :t_timer
  Field scene           :t_scene

  ''  components (these will be initialized in their
  ''  own functions not in NEW
  Field assets          :t_gb_assets
  Field visual          :t_gb_visual
  Field graph           :t_gb_graph
  Field mouse           :t_gb_mouse
  Field console         :t_gb_console
  Field debug           :t_gb_debug
  Field camera          :t_gb_camera
  Field timing          :t_gb_timing
  Field controller      :t_gb_controller
  Field audio           :t_gb_audio
  Field gbs             :t_gbs
  field net             :t_gb_net

  Field editor          :t_gb_editor
  Field testing         :t_gb_testing
EndType

'''''''''''''
'' globals ''
'''''''''''''

Global gb:t_gb

'''''''''
'' new ''
'''''''''

Function new_gb:t_gb ()
  Local r:t_gb      = New t_gb
  r.running         = new_bool(True)
  r.paused          = new_bool(False)
  r.speed           = new_number(1.0, 0.0, 10.0)
  r.canvas          = new_canvas(400,240)
  r.settings        = null

  r.autosave_timer  = new_timer(60)
  r.pulser          = new_pulser()
  Return r
EndFunction

''''''''''''
'' events ''
''''''''''''

Function gb_init()
  gb = new_gb()
  gb_timing_init()
  gb_assets_init()
  gb_visual_init()
  gb_graph_init()
  gb_mouse_init()
  gb_debug_init()
  gb_camera_init()
  gb_console_init()
  gb_controller_init()
  gb_net_init()
  gbs_init()

  gb_editor_init()
  gb_testing_init()
EndFunction

Function gb_load()
  gb_assets_load()
	gb_load_settings()
	gb_console_load()
	gb_visual_load()
	gb_assets_load()
	gb_debug_load()

  gb_editor_load()
	gb_testing_load()
EndFunction

Function gb_start()
  gb_net_start()
	gb_testing_start()
	gb_debug_start()
EndFunction

Function gb_update()
	gb_timing_update	()
  gb_console_update	()
  gb_visual_update	()
  gb_mouse_update		()
  gb_net_update     ()
	If bool_neq(gb.console.active)
    gb_controller_update()
    If gb.scene Then scene_update(gb.scene)
  EndIf
	
	timer_update(gb.autosave_timer)
	If timer_finished(gb.autosave_timer)
		gb_save_settings()
		timer_reset(gb.autosave_timer)
	EndIf

  pulser_update(gb.pulser)
  gb_editor_update  ()
	gb_testing_update	()
	gb_debug_update		()
EndFunction

Function gb_draw()
  gb_graph_draw     ()
	gb_debug_draw_bg	()
	If gb.scene Then scene_draw(gb.scene)
  canvas_draw       (gb.canvas)
  gb_editor_draw    ()
	gb_testing_draw		()
	gb_visual_draw		()
	gb_debug_draw_fg	()
	gb_console_draw		()
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
	bool_set(gb.running, False)
EndFunction

Function gb_set_speed(n:Float)
	number_set(gb.speed, n)
EndFunction

Function gb_pause(b:Byte=True)
	bool_set( gb.paused, bool(b) )
EndFunction

'''''''''''''''''''''''''''''''''''''''''''
'' saving and loading GBML settings file ''
'''''''''''''''''''''''''''''''''''''''''''

Function gb_load_settings:t_dict ()
	Local u:t_dict = gbml_load("settings")
	If (u)
    gb.settings = u
  Else
    gb.settings = new_dict()
  EndIf
EndFunction

Function gb_save_settings()
	gbml_save(gb.settings,"settings")
EndFunction
