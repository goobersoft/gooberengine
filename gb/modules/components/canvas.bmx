
''''''''''''
'' consts ''
''''''''''''

const gb_canvas_width     :int = 400
const gb_canvas_height    :int = 240

const gb_canvas_draw_mode_solid      :int = 0
const gb_canvas_draw_mode_alpha      :int = 1
const gb_canvas_draw_mode_add        :int = 2
const gb_canvas_draw_mode_mul        :int = 3
const gb_canvas_draw_mode_sub        :int = 4
const gb_canvas_draw_mode_div        :int = 5

'''''''''''''
'' globals ''
'''''''''''''

  '' the canvas and its associated data
global gb_canvas_data          :t_canvas

  '' blending mode.
global gb_canvas_draw_mode     :t_int

  '' this is the current origin to rotate/scale from.
global gb_canvas_draw_origin   :t_point

  '' this is the current depth when drawing.
global gb_canvas_draw_depth    :t_float

  '' used when checkering
global gb_canvas_draw_color_1  :t_color
global gb_canvas_draw_color_2  :t_color
  
  '' used for drawing tools like lines, ovals, and
  '' drawing textures scaled.
global gb_canvas_draw_clip     :t_irect

  '' these boolean flags indicate whether or not the
  '' texture drawn should be read forwards or backwards
  '' in the X and Y axis.
global gb_canvas_draw_flip_x   :t_bool
global gb_canvas_draw_flip_y   :t_bool

  '' pixels will fit to this palette when syncing.
global gb_canvas_draw_palette  :t_palette

  '' if this is true, the canvas will auto-sync every frame.
global gb_canvas_mode_sync     :t_bool

global gb_canvas_screenshot_id :t_int


''''''''''''
'' events ''
''''''''''''

function gb_canvas_init()
  gb_canvas_data            = new_canvas(400,240)
  gb_canvas_draw_mode       = new_int( gb_canvas_draw_mode_solid )
  gb_canvas_draw_origin     = new_point(0,0)
  gb_canvas_draw_depth      = new_float(0)
  gb_canvas_draw_color_1    = new_color(3,3,3,3)
  gb_canvas_draw_color_2    = new_color(0,0,0,3)
  gb_canvas_draw_clip       = new_irect(0,0,400,240)
  gb_canvas_draw_flip_x     = new_bool(false)
  gb_canvas_draw_flip_y     = new_bool(false)
  gb_canvas_mode_sync       = new_bool(false)
endfunction

function gb_canvas_update()
  if gb_canvas_mode_sync.get()
    canvas_sync(gb_canvas_data)
  endif

  if keyhit(key_f12)
    gb_canvas_screenshot()
  endif
endfunction

function gb_canvas_draw()
  canvas_draw(gb_canvas_data)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_canvas_sync()
  canvas_sync(gb_canvas_data)
endfunction

function gb_canvas_cls()
  gb_canvas_swap_color()
  gb_canvas_draw_rect(0,0,gb_canvas_width,gb_canvas_height)
  gb_canvas_swap_color()
endfunction

function gb_canvas_swap_color()
  local c:t_color = gb_canvas_draw_color_1
  gb_canvas_draw_color_1 = gb_canvas_draw_color_2
  gb_canvas_draw_color_2 = c
endfunction

function gb_canvas_set_color( c1:t_color=null, c2:t_color=null )
  if c1 <> null then color_set(gb_canvas_draw_color_1, c1)
  if c2 <> null then color_set(gb_canvas_draw_color_2, c2)
endfunction

function gb_canvas_set_draw_mode( n:int )
  int_set( gb_canvas_draw_mode, expr(in_irange(n, 0, 7),n,0) )
endfunction

''''''''''''''''''''
'' pixel blending ''
''''''''''''''''''''

function gb_canvas_blend_pixel( x:int, y:int, c:t_color )
  gb_canvas_set_color(c)
  local p:t_color = gb_canvas_data.pixels[x][y]
  select gb_canvas_draw_mode.get()
    case gb_canvas_draw_mode_solid
      color_set(p,c)
    case gb_canvas_draw_mode_alpha
      color_blend_alpha(p,c)
    case gb_canvas_draw_mode_add
      color_blend_add(p,c)
    case gb_canvas_draw_mode_mul
      color_blend_mul(p,c)
    case gb_canvas_draw_mode_sub
      color_blend_sub(p,c)
  endselect
endfunction

'''''''''''''''''''''''
'' Drawing functions ''
'''''''''''''''''''''''

function gb_canvas_plot( x:int, y:int, c:t_color=null )
  gb_canvas_set_color(c)
  if ipoint_in_irect( x, y, 0, 0, gb_canvas_width, gb_canvas_height )
	  gb_canvas_blend_pixel( x, y, c )
  endif
endfunction

function gb_canvas_draw_rect( x:int, y:int, w:int, h:int, c:t_color=null )
  gb_canvas_set_color(c)
	x = iwrap(x,0,gb_canvas_width-1)
	y = iwrap(y,0,gb_canvas_height-1)
	w = iclamp(w,0,gb_canvas_width-x)
	h = iclamp(h,0,gb_canvas_height-y)
	for local i:int = 0 to w-1
		for local j:int = 0 to h-1
		  gb_canvas_blend_pixel( x, y, c )
		next
	next
endfunction

function gb_canvas_draw_box( x:int, y:int, w:int, h:int )

endfunction

function gb_canvas_draw_tile( x:int, y:int, im:int, ix:int, iy:int)
  im = iclamp(im,0,gb_assets_max_images-1)
  ix = iclamp(ix,0,39)
  iy = iclamp(iy,0,23)
  local d:t_canvas = image_canvas(gb_assets_images[im])
  for local i:int = 0 to 9
    for local j:int = 0 to 9
      gb_canvas_plot(x+i, y+j, canvas_get_pixel(d, (ix*10) + i, (iy*10) + j))
    next
  next
endfunction

'''''''''''
'' event ''
'''''''''''

function gb_canvas_screenshot()
  canvas_sync(gb_canvas_data)
  local ss:string   = "./screenshots/" + currentdate("%Y-%m-%d-%H:%M:%S-") + string(rand(100,999)) + ".png"
  local x:tpixmap   = lockimage(gb_canvas_data.image)
  local m:int       = savepixmappng(x,ss,0)
  unlockimage       (gb_canvas_data.image)
endfunction