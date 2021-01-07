
const gb_assets_max_images      :int = 100
const gb_assets_max_sounds      :int = 100
const gb_assets_max_animations  :int = 100
const gb_assets_max_palettes    :int = 100
const gb_assets_max_fonts       :int = 100

type t_gb_assets
  field images      :t_image      []
  field sounds      :t_sound      []
  field animations  :t_animation  []
  field palettes    :t_palette    []
  field fonts       :t_font       []
endtype

function new_gb_assets:t_gb_assets ()
  local r:t_gb_assets = new t_gb_assets
  r.images      = new t_image       [ gb_assets_max_images ]
  r.sounds      = new t_sound       [ gb_assets_max_sounds ]
  r.animations  = new t_animation   [ gb_assets_max_animations ]
  r.palettes    = new t_palette     [ gb_assets_max_animations ]
  r.fonts       = new t_font        [ gb_assets_max_fonts ]
  return r
endfunction

function gb_assets_init()
  gb.assets = new_gb_assets()
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
  if gb.assets.images[n] <> null
    return gb.assets.images[n].source
  endif
  return null
endfunction

function gb_assets_get_sound:tsound ( n:int )
  n = iclamp(n,0,gb_assets_max_sounds)
  if gb.assets.sounds[n] <> null
    return gb.assets.sounds[n].source
  endif
  return null
endfunction

function gb_assets_get_font:t_font ( n:int )
  n = iclamp(n,0,gb_assets_max_fonts)
  if gb.assets.fonts[n] <> null
    return gb.assets.fonts[n]
  endif
  return null
endfunction


function gb_assets_load_image( n:int, s:string )
  n = iwrap(n,0,gb_assets_max_images)
  local u:t_image = new_image(s)
  gb.assets.images[n] = u
endfunction

function gb_assets_load_sound( n:int, s:string )
  n = iwrap(n,0,gb_assets_max_sounds)
  local u:t_sound = new_sound(s)
  gb.assets.sounds[n] = u
endfunction

function gb_assets_load_default_images()
	gb_assets_load_image(0,"gb/media/images/gb-0.png")
  gb_assets_load_image(1,"gb/media/images/editor-bg.png")
endfunction

function gb_assets_load_default_sounds()
	gb_assets_load_sound(90,"gb/media/sounds/jake-1.ogg")
	gb_assets_load_sound(91,"gb/media/sounds/jake-2.ogg")
	gb_assets_load_sound(92,"gb/media/sounds/jake-3.ogg")
	gb_assets_load_sound(93,"gb/media/sounds/jake-4.ogg")
endfunction

function gb_assets_load_default_fonts()
  for local i:int = 0 to 2
    gb.assets.fonts[i] = load_font("gb/data/font-" + i)
  next
endfunction

function gb_assets_load_default_animations()
	local a:t_animation
	
	a = new_animation(4)
	animation_add_frame(a, 0, 0,19,22,2,2, 0.125)
	animation_add_frame(a, 1, 0,21,22,2,2, 0.125)
	animation_add_frame(a, 2, 0,23,22,2,2, 0.125)
	animation_add_frame(a, 3, 0,25,22,2,2, 0.125)
	gb.assets.animations[0] = a
	
	a = new_animation(5)
	animation_add_frame(a, 0, 0,5,20,1,1, 0.05)
	animation_add_frame(a, 1, 0,6,20,1,1, 0.05)
	animation_add_frame(a, 2, 0,7,20,1,1, 0.05)
	animation_add_frame(a, 3, 0,8,20,1,1, 0.05)
	animation_add_frame(a, 4, 0,9,20,1,1, 0.05)
	gb.assets.animations[1] = a
endfunction

function gb_assets_load_default_palettes()
	gb.assets.palettes[0] = new_palette()
	palette_set(gb.assets.palettes[0], 0,0,0,0,3)
	palette_set(gb.assets.palettes[0], 1,1,1,1,3)
	palette_set(gb.assets.palettes[0], 2,2,2,2,3)
	palette_set(gb.assets.palettes[0], 3,3,3,3,3)
	palette_set(gb.assets.palettes[0], 4,2,1,0,3)
	palette_set(gb.assets.palettes[0], 5,3,2,0,3)
	palette_set(gb.assets.palettes[0], 6,0,1,0,3)
	palette_set(gb.assets.palettes[0], 7,1,2,0,3)
	palette_set(gb.assets.palettes[0], 8,0,1,2,3)
	palette_set(gb.assets.palettes[0], 9,0,2,3,3)
	palette_set(gb.assets.palettes[0],10,2,0,1,3)
	palette_set(gb.assets.palettes[0],11,3,0,2,3)
endfunction

'''''''''''''''
'' Reloading ''
'''''''''''''''

function gb_assets_reload_images()
endfunction

function gb_assets_reload_sounds()
endfunction