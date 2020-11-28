
''''''''''''
'' consts ''
''''''''''''

const palette_size :int = 256

''''''''''
'' type ''
''''''''''

type t_palette
	field data    :t_color[]
endtype

function new_palette:t_palette()
	local r:t_palette = new t_palette
	r.data = new t_color[palette_size]
	for local i:int = 0 to palette_size-1
		r.data[i] = new_color(3,3,3,3)
	next
	
  color_set(r.data[255], 0,0,0,0)
  return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function palette_set( p:t_palette, n:int, r:float, g:float, b:float, a:float )
	n = iclamp(n,0,palette_size-1)
	color_set( p.data[n], r, g, b, a )
endfunction