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
                                                            
DEVELOPED BY MR.GOOBER    2020-01-31
goober was here           2020-06-28
goober was here again     2020-12-04
endrem

''''''''''''''
'' includes ''
''''''''''''''

'' core modules
include "modules/core/types.bmx"
include "modules/core/functions.bmx"
include "modules/core/point.bmx"
include "modules/core/point3.bmx"
include "modules/core/number.bmx"
include "modules/core/rect.bmx"
include "modules/core/cube.bmx"
include "modules/core/font.bmx"
include "modules/core/ibox.bmx"
' include "modules/core/list.bmx"
include "modules/core/stopwatch.bmx"
include "modules/core/color.bmx"
include "modules/core/palette.bmx"
include "modules/core/timer.bmx"
include "modules/core/dictval.bmx"
include "modules/core/dict.bmx"
include "modules/core/tag.bmx"
include "modules/core/counter.bmx"
include "modules/core/bitmap.bmx"
include "modules/core/matrix.bmx"
include "modules/core/image.bmx"
include "modules/core/sound.bmx"
include "modules/core/logchain.bmx"
include "modules/core/drawcmd.bmx"
include "modules/core/drawstack.bmx"
include "modules/core/pulser.bmx"
include "modules/core/params.bmx"
include "modules/core/transition.bmx"
include "modules/core/scene.bmx"

'' gfx modules
include "modules/gfx/animation.bmx"
include "modules/gfx/emitter.bmx" 
include "modules/gfx/lightcube.bmx"
include "modules/gfx/spinicon.bmx"
include "modules/gfx/jake.bmx"
include "modules/gfx/tilemap.bmx"
include "modules/gfx/sprite.bmx"

'' ui objects
include "modules/ui/button.bmx"


'' object components
include "modules/comp/visual.bmx"
include "modules/comp/audio.bmx"
include "modules/comp/timing.bmx"
include "modules/comp/controller.bmx"
include "modules/comp/camera.bmx"
include "modules/comp/gbml.bmx"
include "modules/comp/graph.bmx"
include "modules/comp/console.bmx"
include "modules/comp/mouse.bmx"
include "modules/comp/gbs.bmx"
include "modules/comp/net.bmx"

include "modules/comp/assets.bmx"

'' debugging
include "modules/debugging/testing.bmx"
include "modules/debugging/debug.bmx"

'' editor
include "modules/editor/editor.bmx"



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

''''''''''
'' type ''
''''''''''

type t_gb
  field running         :t_bool
  field paused          :t_bool
  field speed           :t_number
  field fonts           :t_font[]
  field bitmap          :t_bitmap
  field images          :timage[]
  field image_paths     :string[]
  field sounds          :tsound[]
  field sound_paths     :string[]
  field palettes        :t_palette[]
  field animations      :t_animation[]
  field settings        :t_dict

  ''  updatables
  field pulser          :t_pulser
  field autosave_timer  :t_timer
  field scene           :t_scene

  ''  components (these will be initialized in their
  ''  own functions not in NEW
  field assets          :t_gb_assets
  field visual          :t_gb_visual
  field graph           :t_gb_graph
  field editor          :t_gb_editor
  field mouse           :t_gb_mouse
  field console         :t_gb_console
  field debug           :t_gb_debug
  field camera          :t_gb_camera
  field timing          :t_gb_timing
  field controller      :t_gb_controller
  field audio           :t_gb_audio
  field gbs             :t_gbs

  field testing         :t_gb_testing
endtype

'''''''''''''
'' globals ''
'''''''''''''

global gb:t_gb

'''''''''
'' new ''
'''''''''

function new_gb:t_gb ()
  local r:t_gb      = new t_gb
  r.running         = new_bool(true)
  r.paused          = new_bool(false)
  r.speed           = new_number(1.0, 0.0, 10.0)
  r.fonts           = new t_font[ gb_max_fonts ]
  r.bitmap          = new_bitmap(400,240)
  r.images          = new timage[ gb_max_images ]
  r.image_paths     = new string[ gb_max_images ]
  r.sounds          = new tsound[ gb_max_sounds ]
  r.sound_paths     = new string[ gb_max_sounds ]
  r.palettes        = new t_palette[ gb_max_palettes ]
  r.animations      = new t_animation[ gb_max_animations ]
  r.settings        = gb_load_settings()

  r.autosave_timer  = new_timer(60)
  r.pulser          = new_pulser()
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_init()
  gb = new_gb()
  gb_timing_init()
  gb_assets_init()
  gb_visual_init()
  gb_graph_init()
  gb_mouse_init()
  gb_editor_init()
  gb_debug_init()
  gb_camera_init()
  gbs_init()

  gb_testing_init()
endfunction

function gb_load()
  gb_assets_load()
	gb_load_settings()
	gb_console_load()
	gb_visual_load()
	gb_assets_load()
  gb_editor_load()
	gb_testing_load()
	gb_debug_load()
endfunction

function gb_start()
	gb_testing_start()
	gb_debug_start()
endfunction

function gb_update()
	gb_timing_update	()
	gb_console_update	()
	gb_visual_update	()
	gb_mouse_update		()
	
	if bool_eq(gb.console.active,false)
		gb_controller_update()
    if gb.scene
		  scene_update(gb.scene)
    endif
  endif
	
	timer_update(gb.autosave_timer)
	if timer_finished(gb.autosave_timer)
		gb_save_settings()
		timer_reset(gb.autosave_timer)
	endif

  pulser_update(gb.pulser)
	
  gb_editor_update  ()
	gb_testing_update	()
	gb_debug_update		()
endfunction

function gb_draw()
	gb_debug_draw_bg	()
	scene_draw        (gb.scene)
  bitmap_draw       (gb.bitmap)
  gb_editor_draw    ()
	gb_testing_draw		()
	gb_visual_draw		()
	gb_debug_draw_fg	()
	gb_console_draw		()
	gb_mouse_draw			()
endfunction

function gb_end()
	gb_visual_end				()
	gb_testing_end			()
	gb_debug_end				()
	gb_save_settings		()
endfunction

'''''''''''''''
'' Functions ''
'''''''''''''''

function gb_stop()
	bool_set(gb.running, false)
endfunction

function gb_set_speed(n:float)
	number_set(gb.speed, n)
endfunction

function gb_pause(b:byte=true)
	bool_set( gb.paused, bool(b) )
endfunction

'''''''''''''''''''''''''''''''''''''''''''
'' saving and loading GBML settings file ''
'''''''''''''''''''''''''''''''''''''''''''

function gb_load_settings:t_dict ()
	local u:t_dict = gbml_load("settings")
	if (u)
    gb.settings = u
  else
    gb.settings = new_dict()
  endif
endfunction

function gb_save_settings()
	gbml_save(gb.settings,"settings")
endfunction
