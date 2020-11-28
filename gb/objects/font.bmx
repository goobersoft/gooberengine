
Type t_font
	''	Global image ID
	Field image_id		:Int
	''	number of tiles X and Y
	Field tiles				:t_ipoint
	''	tile spacing
	Field tilesize		:t_point
	''	the tile location on the tileset
	Field origin			:t_point
	''	The ASCII offset (by default, 32)
	Field offset			:Int
	''	Individual spacing for each character.
	''	By default, this is 1 (one tile). Can be a decimal.
	Field spacing			:Float[]
endtype

function new_font:t_font(im:int, ox:float=0.0, oy:float=0.0, tx:int=10, ty:int=10, sx:float=1.0, sy:float=1.0, off:int=32)
	local r:t_font = new t_font
	r.image_id 	= im
	r.origin 		= new_point(ox,oy)
	r.tiles 		= new_ipoint(tx,ty)
	r.tilesize	= new_point(sx,sy)
	r.offset		= off
	r.spacing		= farray(tx*ty,sx)
	return r
endfunction

function font_string_width:float(f:t_font, t:string)
	local l:int = len(t)
	local r:float = 0.0
	if l > 0
		for local i:int = 0 to l-1
			r = r + f.spacing[ t[i]-32 ]
		next
	endif
	return r
endfunction

function font_set_spacing (f:t_font, n:int, a:float)
	n = iclamp(n,0,(f.tiles.x*f.tiles.y)-1)
	f.spacing[n] = a
endfunction