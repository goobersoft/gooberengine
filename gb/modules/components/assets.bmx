
''''''''''''
'' consts ''
''''''''''''

const gb_assets_max_images      :int = 100
const gb_assets_max_sounds      :int = 100
const gb_assets_max_animations  :int = 100
const gb_assets_max_palettes    :int = 100
const gb_assets_max_fonts       :int = 100

'''''''''''''
'' globals ''
'''''''''''''

global gb_assets_images      :t_image      []
global gb_assets_sounds      :t_sound      []
global gb_assets_animations  :t_animation  []
global gb_assets_palettes    :t_palette    []
global gb_assets_fonts       :t_font       []

''''''''''
'' init ''
''''''''''

function gb_assets_init()
  gb_assets_images      = new t_image       [ gb_assets_max_images ]
  gb_assets_sounds      = new t_sound       [ gb_assets_max_sounds ]
  gb_assets_animations  = new t_animation   [ gb_assets_max_animations ]
  gb_assets_palettes    = new t_palette     [ gb_assets_max_animations ]
  gb_assets_fonts       = new t_font        [ gb_assets_max_fonts ]
  autoimageflags(0)
endfunction

function gb_assets_load()
  gb_assets_load_default_images()
  gb_assets_load_default_sounds()
  gb_assets_load_default_fonts()
  gb_assets_load_default_animations()
  gb_assets_load_default_palettes()
  gb_assets_load_default_fonts()
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_assets_get_image:timage ( n:int )
  n = iclamp(n,0,gb_assets_max_images)
  if gb_assets_images[n] <> null
    return gb_assets_images[n].source
  endif
  return null
endfunction

function gb_assets_get_sound:tsound ( n:int )
  n = iclamp(n,0,gb_assets_max_sounds)
  if gb_assets_sounds[n] <> null
    return gb_assets_sounds[n].source
  endif
  return null
endfunction

function gb_assets_get_font:t_font ( n:int )
  n = iclamp(n,0,gb_assets_max_fonts)
  if gb_assets_fonts[n] <> null
    return gb_assets_fonts[n]
  endif
  return null
endfunction


function gb_assets_load_image( n:int, s:string )
  n = iwrap(n,0,gb_assets_max_images)
  local u:t_image = new_image(s)
  gb_assets_images[n] = u
endfunction

function gb_assets_load_sound( n:int, s:string )
  n = iwrap(n,0,gb_assets_max_sounds)
  local u:t_sound = new_sound(s)
  gb_assets_sounds[n] = u
endfunction

function gb_assets_load_default_images()
	gb_assets_load_image(0,"gb/media/images/gb-0.png")
  gb_assets_load_image(1,"gb/media/images/gb-1.png")
  gb_assets_load_image(2,"gb/media/images/editor-bg.png")
endfunction

function gb_assets_load_default_sounds()
	gb_assets_load_sound(90,"gb/media/sounds/jake-1.ogg")
	gb_assets_load_sound(91,"gb/media/sounds/jake-2.ogg")
	gb_assets_load_sound(92,"gb/media/sounds/jake-3.ogg")
	gb_assets_load_sound(93,"gb/media/sounds/jake-4.ogg")
  gb_assets_load_sound(80,"gb/media/instruments/silence.ogg")
  gb_assets_load_sound(81,"gb/media/instruments/sawtooth.ogg")
endfunction

function gb_assets_load_default_fonts()
  for local i:int = 0 to 2
    gb_assets_fonts[i] = load_font("gb/data/font-" + i)
  next
endfunction

function gb_assets_load_default_animations()
	local a:t_animation
	
	a = new_animation(4)
	animation_add_frame(a, 0, 0,19,22,2,2, 0.125)
	animation_add_frame(a, 1, 0,21,22,2,2, 0.125)
	animation_add_frame(a, 2, 0,23,22,2,2, 0.125)
	animation_add_frame(a, 3, 0,25,22,2,2, 0.125)
	gb_assets_animations[0] = a
	
	a = new_animation(5)
	animation_add_frame(a, 0, 0,5,20,1,1, 0.05)
	animation_add_frame(a, 1, 0,6,20,1,1, 0.05)
	animation_add_frame(a, 2, 0,7,20,1,1, 0.05)
	animation_add_frame(a, 3, 0,8,20,1,1, 0.05)
	animation_add_frame(a, 4, 0,9,20,1,1, 0.05)
	gb_assets_animations[1] = a
endfunction

function gb_assets_load_default_palettes()
	gb_assets_palettes[0] = new_palette()
	palette_set(gb_assets_palettes[0], 0,0,0,0,3)
	palette_set(gb_assets_palettes[0], 1,1,1,1,3)
	palette_set(gb_assets_palettes[0], 2,2,2,2,3)
	palette_set(gb_assets_palettes[0], 3,3,3,3,3)
	palette_set(gb_assets_palettes[0], 4,2,1,0,3)
	palette_set(gb_assets_palettes[0], 5,3,2,0,3)
	palette_set(gb_assets_palettes[0], 6,0,1,0,3)
	palette_set(gb_assets_palettes[0], 7,1,2,0,3)
	palette_set(gb_assets_palettes[0], 8,0,1,2,3)
	palette_set(gb_assets_palettes[0], 9,0,2,3,3)
	palette_set(gb_assets_palettes[0],10,2,0,1,3)
	palette_set(gb_assets_palettes[0],11,3,0,2,3)
endfunction

'''''''''''''''
'' Reloading ''
'''''''''''''''

function gb_assets_reload_images()
endfunction

function gb_assets_reload_sounds()
endfunction