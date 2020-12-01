
''''''''''''
'' consts ''
''''''''''''

Const gb_visual_virtual_pixels_x	:Float 	= 400
Const gb_visual_virtual_pixels_y	:Float	= 240
Const gb_visual_center_x					:Float	= 200
Const gb_visual_center_y					:Float	= 120
const gb_visual_diag_size					:float	= 466.47615158762403

'''''''''''''
'' globals ''
'''''''''''''

global gb_visual_fullscreen					:t_bool
global gb_visual_windowscale				:t_float
global gb_visual_virtualscale				:t_float
global gb_visual_viewport						:t_rect 
global gb_visual_pixelscale					:t_float

global gb_visual_letterbox_aspect		:t_float
global gb_visual_letterbox_diagonal	:t_float
global gb_visual_letterbox_rect			:t_rect
global gb_visual_letterbox_irect		:t_irect
global gb_visual_letterbox_scales		:t_point

global gb_visual_use_scanlines			:t_bool
global gb_visual_scanlines_style		:t_number
global gb_visual_scanlines_thick		:t_number

global gb_visual_screenshot_id      :t_int

'''''''''''''''
'' Functions ''
'''''''''''''''

function gb_visual_reopen_graphics()
	gb_visual_set_fullscreen(gb_visual_fullscreen.value)
endfunction

function gb_visual_refresh()
	local gx:float 		= graphicswidth()
	local gy:float 		= graphicsheight()
	local a:float 		= gx / gy
	local la:float 		= a  / float_get(gb_visual_letterbox_aspect)
	
	if (la >= 1.0)
		
		local d:float = gx - (gx/la) 
	
		gb_visual_letterbox_rect.x = d/2
		gb_visual_letterbox_rect.y = 0
		gb_visual_letterbox_rect.w = gx/la
		gb_visual_letterbox_rect.h = gy
		
		gb_visual_letterbox_irect.x = d/2
		gb_visual_letterbox_irect.y = 0
		gb_visual_letterbox_irect.w = gx/la
		gb_visual_letterbox_irect.h = gy
		
		point_set( gb_visual_letterbox_scales, la, 1.0)

	elseif (la < 1.0)
		
		local d:float = gy - (gy*la) 
	
		gb_visual_letterbox_rect.x = 0
		gb_visual_letterbox_rect.y = d/2
		gb_visual_letterbox_rect.w = gx
		gb_visual_letterbox_rect.h = gy*la
		
		gb_visual_letterbox_irect.x = 0
		gb_visual_letterbox_irect.y = d/2
		gb_visual_letterbox_irect.w = gx
		gb_visual_letterbox_irect.h = gy*la
		
		point_set( gb_visual_letterbox_scales, 1.0, 1/la)
	endif
	
	float_set( gb_visual_letterbox_diagonal,
		dist2d(gb_visual_letterbox_rect.w,gb_visual_letterbox_rect.h) )	
	
	''	divide the diagonal by the 1.0 scale diagonal size to get pixel scaling.
	float_set(gb_visual_pixelscale,gb_visual_letterbox_diagonal.value / gb_visual_diag_size )	
	gb_graph_calc_drawscale()
endfunction

function gb_visual_set_virtual_scale(f:float)
	float_set(gb_visual_virtualscale, f)
	gb_graph_calc_drawscale()
endfunction

function gb_visual_set_fullscreen(b:int)
	bool_set(gb_visual_fullscreen, b)
	endgraphics()
	
	if bool_eq(gb_visual_fullscreen, true)
		Graphics(DesktopWidth(), DesktopHeight(), 32, 0, GRAPHICS_BACKBUFFER)
	else
		graphics(400*int(float_get(gb_visual_windowscale)),
			240*int(float_get(gb_visual_windowscale)), 0, 0, GRAPHICS_BACKBUFFER)
	endif
	hidemouse()
	gb_visual_refresh()
endfunction

function gb_visual_set_window_scale(n:float)
	float_set(gb_visual_windowscale, clamp(n,1,4))
	endgraphics()
	graphics(400*int(float_get(gb_visual_windowscale)),
		240*int(float_get(gb_visual_windowscale)), 0, 0, GRAPHICS_BACKBUFFER)
	hidemouse()
	bool_set(gb_visual_fullscreen, false)
	gb_visual_refresh()
endfunction

function gb_visual_take_screenshot()
  local u:timage = createimage(int(gb_visual_letterbox_rect.w),
    int(gb_visual_letterbox_rect.h),DYNAMICIMAGE)
  grabimage         (u,int(gb_visual_letterbox_rect.x), int(gb_visual_letterbox_rect.y))
  local ss:string   = "./screenshots/" + rpad(string(gb_visual_screenshot_id.value),3,"0") + ".png"
  int_add           (gb_visual_screenshot_id,1)
  local x:tpixmap   = lockimage(u)
  local m:int       = savepixmappng(x,ss,0)
  unlockimage       (u)
endfunction

''''''''''''
'' Events ''
''''''''''''

function gb_visual_init()
	gb_visual_fullscreen 		      = new_bool(false)
	gb_visual_windowscale 	      = new_float(2.0)
	gb_visual_virtualscale 	      = new_float(1.0)
	gb_visual_pixelscale		      = new_float(2.0)
	
	gb_visual_letterbox_aspect 		= new_float(5.0/3.0)
	gb_visual_letterbox_diagonal 	= new_float(dist2d(0,0,800,480))
	gb_visual_letterbox_rect			= new_rect(0,0,800,480)
	gb_visual_letterbox_irect			= new_irect(0,0,800,480)
	gb_visual_letterbox_scales		= new_point(1.0,1.0)
	
	gb_visual_use_scanlines				= new_bool(false)
	gb_visual_scanlines_style			= new_number(0,0,4,true)
	gb_visual_scanlines_thick			= new_number(0,0,4,true)
	
	AppTitle 								=	"GooberEngine " + gb_version
	setgraphicsdriver				(glmax2ddriver())
	Graphics 								(800,480,0,0,graphics_backbuffer)
	SetBlend								(alphablend)
	setmaskcolor						(150,0,250)
	HideMouse								()
	SeedRnd									(millisecs())
endfunction

function gb_visual_load()
	local u:t_dict = gb_settings
	gb_visual_set_fullscreen  ( dict_read_byte(u,"visual/fullscreen") )
	if (not gb_visual_fullscreen.value)
		gb_visual_set_window_scale( dict_read_float(u,"visual/windowscale") )
	endif
	
	bool_set    (gb_visual_use_scanlines,   dict_read_byte(u,"visual/use-scanlines"))
	number_set  (gb_visual_scanlines_style, dict_read_byte(u,"visual/scanlines-style"))
	number_set  (gb_visual_scanlines_thick, dict_read_byte(u,"visual/scanlines-thick"))
	int_set     (gb_visual_screenshot_id,   dict_read_int(u,"visual/screenshot-id"))
endfunction

function gb_visual_update(d:float)
	if keyhit(key_f2)
		if float_gt(gb_visual_windowscale, 1.0)
			gb_visual_set_window_scale( float_get(gb_visual_windowscale)-1.0)
		endif
	endif
	if keyhit(key_f3)
		if float_lt(gb_visual_windowscale, 4.0)
			gb_visual_set_window_scale( float_get(gb_visual_windowscale)+1.0)
		endif
	endif
	if keyhit(key_f4)
		gb_visual_set_fullscreen( bool_nt(gb_visual_fullscreen) )
	endif
  if keyhit(key_f12)
    gb_visual_take_screenshot()
  endif
endfunction

function gb_visual_draw(x:float=0, y:float=0)
	if bool_eq(gb_visual_use_scanlines)
		gb_visual_set_virtual_scale(0.5)
		gb_graph_set_mode(gb_graph_mode_multiply)
		gb_graph_draw_tile_boxrect( 0,0, 0,
			22 + (gb_visual_scanlines_thick.value*2),
			4 + (gb_visual_scanlines_style.value*2),
			2,2,40,24)
		gb_graph_set_mode(gb_graph_mode_normal)
		gb_visual_set_virtual_scale(1)
	endif
endfunction

function gb_visual_end()
	local d:t_dict = new_dict()
	dict_set_float	(d,"windowscale",gb_visual_windowscale.value)
	dict_set_byte		(d,"fullscreen",gb_visual_fullscreen.value)
	dict_set_byte		(d,"use-scanlines",gb_visual_use_scanlines.value)
	dict_set_byte		(d,"scanlines-thick",byte(gb_visual_scanlines_thick.value))
	dict_set_byte		(d,"scanlines-style",byte(gb_visual_scanlines_style.value))
  dict_set_int    (d,"screenshot-id",int(gb_visual_screenshot_id.value))
	dict_set_dir		(gb_settings,"visual",d)
endfunction

