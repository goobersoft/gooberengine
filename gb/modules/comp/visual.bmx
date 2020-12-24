
''''''''''''''
'' includes ''
''''''''''''''

include "visual/letterbox.bmx"
include "visual/scanlines.bmx"

''''''''''''
'' consts ''
''''''''''''

Const gb_visual_virtual_pixels_x	:Float 	= 400
Const gb_visual_virtual_pixels_y	:Float	= 240
Const gb_visual_center_x					:Float	= 200
Const gb_visual_center_y					:Float	= 120
const gb_visual_diag_size					:float	= 466.47615158762403

''''''''''
'' type ''
''''''''''

type t_gb_visual
  field fullscreen    :t_bool
  field windowscale   :t_float
  field virtualscale  :t_float
  field viewport      :t_rect
  field pixelscale    :t_float
  field letterbox     :t_gb_visual_letterbox
  field scanlines     :t_gb_visual_scanlines
  field screenshot_id :t_int
endtype

function new_gb_visual:t_gb_visual ()
  local r:t_gb_visual = new t_gb_visual
  r.fullscreen 		      = new_bool(false)
	r.windowscale 	      = new_float(2.0)
	r.virtualscale 	      = new_float(1.0)
	r.pixelscale		      = new_float(2.0)
  r.letterbox           = new_gb_visual_letterbox()
  r.scanlines           = new_gb_visual_scanlines()
  r.screenshot_id       = new_int(0)
  return r
endfunction

'''''''''''
'' Events ''
''''''''''''

function gb_visual_init()
	gb.visual = new_gb_visual()
	
	AppTitle 								=	"GooberEngine " + gb_version
	setgraphicsdriver				(glmax2ddriver())
	Graphics 								(800,480,0,0,graphics_backbuffer)
	SetBlend								(alphablend)
	setmaskcolor						(150,0,250)
	HideMouse								()
	SeedRnd									(millisecs())
endfunction

function gb_visual_load()
	local u:t_dict = gb.settings
	gb_visual_set_fullscreen  ( dict_read_byte(u,"visual/fullscreen") )
	if (not gb.visual.fullscreen.value)
		gb_visual_set_window_scale( dict_read_float(u,"visual/windowscale") )
	endif
	
	bool_set    (gb.visual.scanlines.active,    dict_read_byte(u,"visual/scanlines/active"))
	number_set  (gb.visual.scanlines.style,     dict_read_byte(u,"visual/scanlines/style"))
	number_set  (gb.visual.scanlines.thick,     dict_read_byte(u,"visual/scanlines/thick"))
	int_set     (gb.visual.screenshot_id,       dict_read_int(u,"visual/screenshot-id"))
endfunction

function gb_visual_update()
  local d:float   = gb_delta()
	if keyhit(key_f2)
		if float_gt(gb.visual.windowscale, 1.0)
			gb_visual_set_window_scale( float_get(gb.visual.windowscale)-1.0)
		endif
	endif
	if keyhit(key_f3)
		if float_lt(gb.visual.windowscale, 4.0)
			gb_visual_set_window_scale( float_get(gb.visual.windowscale)+1.0)
		endif
	endif
	if keyhit(key_f4)
		gb_visual_set_fullscreen( bool_nt(gb.visual.fullscreen) )
	endif
  if keyhit(key_f12)
    gb_visual_take_screenshot()
  endif
endfunction

function gb_visual_draw(x:float=0, y:float=0)
	if bool_eq(gb.visual.scanlines.active)
		gb_visual_set_virtual_scale(0.5)
		gb_graph_set_mode(gb_graph_mode_multiply)
		gb_graph_draw_tile_boxrect( 0,0, 0,
			22 + (gb.visual.scanlines.thick.value*2),
			4 + (gb.visual.scanlines.style.value*2),
			2,2,40,24)
		gb_graph_set_mode(gb_graph_mode_normal)
		gb_visual_set_virtual_scale(1)
	endif
endfunction

function gb_visual_end()
	local d:t_dict = new_dict()
	dict_set_float	  (d,"windowscale",gb.visual.windowscale.value)
	dict_set_byte		  (d,"fullscreen",gb.visual.fullscreen.value)
	dict_write_byte		(d,"scanlines/active",gb.visual.scanlines.active.value)
	dict_write_byte		(d,"scanlines/thick",byte(gb.visual.scanlines.thick.value))
	dict_write_byte		(d,"scanlines/style",byte(gb.visual.scanlines.style.value))
  dict_set_int      (d,"screenshot-id",int(gb.visual.screenshot_id.value))
	dict_set_dir		  (gb.settings,"visual",d)
endfunction


'''''''''''''''
'' Functions ''
'''''''''''''''

function gb_visual_reopen_graphics()
	gb_visual_set_fullscreen(gb.visual.fullscreen.value)
endfunction

function gb_visual_refresh()
	local gx:float 		= graphicswidth()
	local gy:float 		= graphicsheight()
	local a:float 		= gx / gy
	local la:float 		= a  / float_get(gb.visual.letterbox.aspect)
	
	if (la >= 1.0)
		
		local d:float = gx - (gx/la) 
	
		gb.visual.letterbox.rect.x = d/2
		gb.visual.letterbox.rect.y = 0
		gb.visual.letterbox.rect.w = gx/la
		gb.visual.letterbox.rect.h = gy
		
		gb.visual.letterbox.irect.x = d/2
		gb.visual.letterbox.irect.y = 0
		gb.visual.letterbox.irect.w = gx/la
		gb.visual.letterbox.irect.h = gy
		
		point_set( gb.visual.letterbox.scales, la, 1.0)

	elseif (la < 1.0)
		
		local d:float = gy - (gy*la) 
	
		gb.visual.letterbox.rect.x = 0
		gb.visual.letterbox.rect.y = d/2
		gb.visual.letterbox.rect.w = gx
		gb.visual.letterbox.rect.h = gy*la
		
		gb.visual.letterbox.irect.x = 0
		gb.visual.letterbox.irect.y = d/2
		gb.visual.letterbox.irect.w = gx
		gb.visual.letterbox.irect.h = gy*la
		
		point_set( gb.visual.letterbox.scales, 1.0, 1/la)
	endif
	
	float_set( gb.visual.letterbox.diagonal,
		dist2d(gb.visual.letterbox.rect.w,gb.visual.letterbox.rect.h) )	
	
	''	divide the diagonal by the 1.0 scale diagonal size to get pixel scaling.
	float_set(gb.visual.pixelscale, gb.visual.letterbox.diagonal.value / gb_visual_diag_size )	
	gb_graph_calc_drawscale()
endfunction

function gb_visual_set_virtual_scale(f:float)
	float_set(gb.visual.virtualscale, f)
	gb_graph_calc_drawscale()
endfunction

function gb_visual_set_fullscreen(b:int)
	bool_set(gb.visual.fullscreen, b)
	endgraphics()
	
	if bool_eq(gb.visual.fullscreen, true)
		Graphics(DesktopWidth(), DesktopHeight(), 32, 0, GRAPHICS_BACKBUFFER)
	else
		graphics(400*int(float_get(gb.visual.windowscale)),
			240*int(float_get(gb.visual.windowscale)), 0, 0, GRAPHICS_BACKBUFFER)
	endif
	hidemouse()
	gb_visual_refresh()
endfunction

function gb_visual_set_window_scale(n:float)
	float_set(gb.visual.windowscale, clamp(n,1,4))
	endgraphics()
	graphics(400*int(float_get(gb.visual.windowscale)),
		240*int(float_get(gb.visual.windowscale)), 0, 0, GRAPHICS_BACKBUFFER)
	hidemouse()
	bool_set(gb.visual.fullscreen, false)
	gb_visual_refresh()
endfunction

function gb_visual_take_screenshot()
  local u:timage = createimage(int(gb.visual.letterbox.rect.w),
    int(gb.visual.letterbox.rect.h),DYNAMICIMAGE)
  grabimage         (u,int(gb.visual.letterbox.rect.x), int(gb.visual.letterbox.rect.y))
  local ss:string   = "./screenshots/" + rpad(string(gb.visual.screenshot_id.value),3,"0") + ".png"
  int_add           (gb.visual.screenshot_id,1)
  local x:tpixmap   = lockimage(u)
  local m:int       = savepixmappng(x,ss,0)
  unlockimage       (u)
endfunction


