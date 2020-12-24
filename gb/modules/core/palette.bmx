
''''''''''''
'' consts ''
''''''''''''

const palette_size :int = 256

''''''''''
'' type ''
''''''''''

''  a depth of 0 indicates that the palette is using a pre-defined array of colors.
type t_palette
	field data      :t_color[]
  field depth     :float
endtype

function new_palette:t_palette()
	local r:t_palette = new t_palette
	r.data = new t_color[palette_size]
	for local i:int = 0 to palette_size-1
		r.data[i] = new_color(3,3,3,3)
	next
  color_set(r.data[255], 0,0,0,0)
  r.depth = 0
  return r
endfunction

function palette_fit_to_depth ( p:t_palette, d:float=4.0 )

endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function palette_set( p:t_palette, n:int, r:float, g:float, b:float, a:float )
	n = iclamp(n,0,palette_size-1)
	color_set( p.data[n], r, g, b, a )
endfunction