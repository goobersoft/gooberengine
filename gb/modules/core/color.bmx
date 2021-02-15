
''''''''''''''''
'' Color Type ''
''''''''''''''''
''  class:    primitive
''  mutable:  yes

''''''''''
'' type ''
''''''''''

type t_color
	field r:float
	field g:float
	field b:float
	field a:float
  field depth:float
  method get:int (); return color_to_argb(self); endmethod
endtype

'' Creates a new color.
'' If no params are given, creates an opaque white color.
function new_color:t_color(r:float=3, g:float=3, b:float=3, a:float=3, d:float=3.0)
	local u:t_color = new t_color
	u.r     = r
	u.g     = g
	u.b     = b
	u.a     = a
  u.depth = d
	return u
endfunction

function new_color_random:t_color(d:float=3)
  return new_color( frand(0,d), frand(0,d), frand(0,d), d, d )
endfunction

'' Creates a color based on a 32-bit ARGB value.
'' Because RGBA 
function new_color_from_argb:t_color (ii:int)
  local r:t_color = new t_color
  r.a     = argb_a(ii)
  r.r     = argb_r(ii)
  r.g     = argb_g(ii)
  r.b     = argb_b(ii)
  r.depth = 0
  return r
endfunction

function new_color_from_rgba:t_color (ii:int)
  local r:t_color = new t_color
  r.r       = rgba_r(ii)
  r.g       = rgba_g(ii)
  r.b       = rgba_b(ii)
  r.a       = rgba_a(ii)
  r.depth   = 0
  return r
endfunction

function clone_color:t_color(c:t_color)
	return new_color(c.r, c.g, c.b, c.a, c.depth)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function color_randomize( c:t_color, r:float=3, g:float=3, b:float=3, a:float=0 )
  c.r = frand(0,r)
  c.g = frand(0,g)
  c.b = frand(0,b)
  if a > 0
    c.a = frand(0,a)
  endif
endfunction

function color_set(c:t_color, r:float, g:float, b:float, a:float=3)
	c.r = clamp(r,0,c.depth)
	c.g = clamp(g,0,c.depth)
	c.b = clamp(b,0,c.depth)
	c.a = clamp(a,0,c.depth)
endfunction

function color_set(c:t_color, d:t_color)
	c.r = d.r
	c.g = d.g
	c.b = d.b
	c.a = d.a
endfunction

'' use this if you only want to set RGB values and not alpha.
function color_set_rgb(c:t_color, r:float, g:float, b:float)
	c.r = clamp(r,0,c.depth)
	c.g = clamp(g,0,c.depth)
	c.b = clamp(b,0,c.depth)
endfunction

'' use this if you only want to set the alpha value of the color.
function color_set_alpha(c:t_color, a:float)
	c.a = clamp(a,0,c.depth)
endfunction

''

function color_add(c:t_color, r:float, g:float, b:float, a:float=3)
	c.r = clamp(c.r + r, 0, c.depth)
	c.g = clamp(c.g + g, 0, c.depth)
	c.b = clamp(c.b + b, 0, c.depth)
	c.a = clamp(c.a + a, 0, c.depth)
endfunction

function color_add_rgb(c:t_color, r:float, g:float, b:float)
	c.r = clamp(c.r+r,0,c.depth)
	c.g = clamp(c.g+g,0,c.depth)
	c.b = clamp(c.b+b,0,c.depth)
endfunction

function color_add_alpha(c:t_color, a:float)
	c.a = clamp(c.a+a,0,c.depth)
endfunction

''

function color_mul(c:t_color, r:float, g:float, b:float, a:float=3)
	c.r = clamp(c.r * r, 0, c.depth)
	c.g = clamp(c.g * g, 0, c.depth)
	c.b = clamp(c.b * b, 0, c.depth)
	c.a = clamp(c.a * a, 0, c.depth)
endfunction

function color_mul_rgb(c:t_color, r:float, g:float, b:float)
	c.r = clamp(c.r*r,0,c.depth)
	c.g = clamp(c.g*g,0,c.depth)
	c.b = clamp(c.b*b,0,c.depth)
endfunction

function color_mul_alpha(c:t_color, a:float)
	c.a = clamp(c.a*a,0,c.depth)
endfunction

''

function color_div(c:t_color, r:float, g:float, b:float, a:float=3)
	if (r<>0) and (g<>0) and (b<>0) and (a<>0)
		c.r = clamp(c.r / r, 0, c.depth)
		c.g = clamp(c.g / g, 0, c.depth)
		c.b = clamp(c.b / b, 0, c.depth)
		c.a = clamp(c.a / a, 0, c.depth)
	endif
endfunction

function color_div_rgb(c:t_color, r:float, g:float, b:float)
	if (r<>0) and (g<>0) and (b<>0)
		c.r = clamp(c.r / r, 0, c.depth)
		c.g = clamp(c.g / g, 0, c.depth)
		c.b = clamp(c.b / b, 0, c.depth)
	endif
endfunction

function color_div_alpha(c:t_color, a:float=3)
	if (a<>0)
		c.a = clamp(c.a / a, 0, c.depth)
	endif
endfunction

''

function color_to_argb:int(c:t_color)
	return argb( ..
		byte(c.a*255/c.depth),
		byte(c.r*255/c.depth),
		byte(c.g*255/c.depth),
		byte(c.b*255/c.depth))
endfunction



''''''''''''''''''''''''''''
'' special blending modes ''
''''''''''''''''''''''''''''

function color_blend_alpha( c:t_color, d:t_color)
	local u:float = clamp(c.a+d.a, 0, c.depth)
	local v:float = d.a/3
	c.r = c.r + ((d.r - c.r) * v)
	c.g = c.g + ((d.g - c.g) * v)
	c.b = c.b + ((d.b - c.b) * v)
	c.a = u
endfunction

function color_blend_add( c:t_color, d:t_color )
	c.r = clamp(c.r + d.r, 0, c.depth)
	c.g = clamp(c.g + d.g, 0, c.depth)
	c.b = clamp(c.b + d.b, 0, c.depth)
	c.a = clamp(c.a + d.a, 0, c.depth)
endfunction

function color_blend_mul( c:t_color, d:t_color )
	c.r = clamp(c.r * (d.r/c.depth),0,c.depth)
	c.g = clamp(c.g * (d.g/c.depth),0,c.depth)
	c.b = clamp(c.b * (d.b/c.depth),0,c.depth)
	c.a = clamp(c.a * (d.a/c.depth),0,c.depth)
endfunction

function color_blend_sub( c:t_color, d:t_color )
	c.r = clamp(c.r-d.r,0,c.depth)
	c.g = clamp(c.g-d.g,0,c.depth)
	c.b = clamp(c.b-d.b,0,c.depth)
	c.a = clamp(c.a-d.a,0,c.depth)
endfunction

function color_blend_div( c:t_color, d:t_color )
	c.r = clamp(c.r/low(d.r,0.0001),0,c.depth)
	c.g = clamp(c.g/low(d.r,0.0001),0,c.depth)
	c.b = clamp(c.b/low(d.r,0.0001),0,c.depth)
	c.a = clamp(c.a/low(d.r,0.0001),0,c.depth)
endfunction

function color_blend_custom( c:t_color, d:t_color, f(c:t_color, d:t_color) )
	f(c,d)
endfunction