
Function empty()
EndFunction

'''''''''''''''''''''''''''''''''''''''''''''''
'' Ensuring default values in case of "null" ''
'''''''''''''''''''''''''''''''''''''''''''''''
''	These functions will

function def:byte(i:byte=0, o:byte=0, c:byte=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:short(i:short=0, o:short=0, c:short=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:int(i:int=0, o:int=0, c:int=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:long(i:long=0, o:long=0, c:long=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:float(i:float=0, o:float=0, c:float=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:double(i:double=0, o:double=0, c:double=-1)
	if (i = c)
		return o
	endif
	return i
endfunction

function def:string(i:string="", o:string="", c:string=null)
	if (i = c)
		return o
	endif
	return i
endfunction

'''''''''''''''''''''''''''''
'' finding stuff in arrays ''
'''''''''''''''''''''''''''''

Function find:Int ( a:Byte[], d:Byte=0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:Short[], d:Short=0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:Int[], d:Int=0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:Long[], d:Long=0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:Float[], d:Float=0.0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:Double[], d:Double=0.0 )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

Function find:Int ( a:String[], d:String="" )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

'' generic object finding function to check memory addresses
Function find:Int ( a:Object[], d:Object=Null )
	If (a)
		If (a.length > 0)
			For Local i:Int = 0 To a.length
				If (a[i] = d)
					Return i
				EndIf
			Next
		EndIf
	EndIf
	Return -1
EndFunction

'''''''''''''''''''''''''''''''''''''''''

function car:string (s:string[])
	if len(s) > 0
		return s[0]
	endif
	return null
endfunction

function cdr:string[] (s:string[])
	if len(s) > 1
		return s[1..]
	endif
	return []
endfunction

function sind:string (s:string[], n:int)
	if len(s) > 0
		if in_irange(n,0,len(s))
			return s[n]
		endif
	endif
	return null
endfunction

function inc:int(n:int ptr)
	n = n + 1
endfunction

function dec:int(n:int ptr)
	n = n - 1
endfunction

function addr:float(a:float, m:float)
	if (a > m) then return m-a
	return 0.0
endfunction

function iaddr:float(a:int, m:int)
	if (a > m) then return m-a
	return 0
endfunction

function low:float( a:float, b:float )
	if (a < b) then return b
	return a
endfunction

function ilow:int( a:int, b:int )
	if (a < b) then return b
	return a
endfunction

function high:float( a:float, b:float )
	if (a > b) then return b
	return a
endfunction

function ihigh:int( a:int, b:int )
	if (a > b) then return b
	return a
endfunction

Function tokenize:String[] ( s:String="", d:String=" ", qt:Byte=True )
	Local r:String[] = New String[0]
	
	Local c:String	= ""
	Local w:String	= ""
	Local l:Int = Len(s)
	Local p:Int = 0
	Local q:Byte = False
	
	If (l > 0)
		p = 0
		While p < l
			
			c = Chr(s[p])
			If (c = d) And (Not q)
				If (Len(w) > 0)
					r = r[..r.length+1]
					r[r.length-1] = w
					w = ""
				EndIf
			ElseIf (qt) And (c = "~q")
				q = Not q
			Else
				w = w + c
			EndIf
		
			p = p + 1
		Wend
		
		If (Len(w) > 0)
			r = r[..r.length+1]
			r[r.length-1] = w
		EndIf

	EndIf
	Return r
EndFunction

Function prob:Int (r:Float)
	r = clamp(r,0,100)
	If ((RndFloat()*100) <= r)
		Return True
	EndIf
	Return False
EndFunction

Function icmp:Int (a:Int, b:Int)
	If (a = b)
		Return True
	EndIf
	Return False
EndFunction

Function ffloor:Float(n:Float)
	Return Float(Floor(n))
EndFunction

Function fceil:Float(n:Float)
	Return Float(Ceil(n))
EndFunction

Function argb:Int (a:Byte, r:Byte, g:Byte, b:Byte)
	Local m:Int = 0
	m = m | (Int(a) Shl 24)
	m = m | (Int(r) Shl 16)
	m = m | (Int(g) Shl 8)
	m = m | b
	Return m
EndFunction
Global rgba:Int(r:Byte,g:Byte,b:Byte,a:Byte) = argb

function argb_a:byte (a:int)
  return ($ff000000 & a) shr 24
endfunction 

function argb_r:byte (a:int)
  return ($00ff0000 & a) shr 16
endfunction 

function argb_g:byte (a:int)
  return ($0000ff00 & a) shr 8
endfunction 

function argb_b:byte (a:int)
  return ($000000ff & a)
endfunction 

global rgba_r:byte(a:int) = argb_a
global rgba_g:byte(a:int) = argb_r
global rgba_b:byte(a:int) = argb_g
global rgba_a:byte(a:int) = argb_b

Function bool :Int (n:Int)
	Return Not Not n
EndFunction

Function rep:String(t:String, n:Int)
	Local r:String = ""
	For Local i:Int = 0 To n-1
		r = r + t
	Next
	Return r
EndFunction

Function lpad:String(t:String, n:Int, p:String=" ")
	Local l:Int = Len(t)
	If (l < n)
		Return t + rep(p,n-l)
	ElseIf (l = n)
		Return t
	ElseIf (l > n)
		Return Left(t,n)
	EndIf
EndFunction

Function rpad:String(t:String, n:Int, p:String=" ")
	Local l:Int = Len(t)
	If (l < n)
		Return rep(p,n-l) + t
	ElseIf (l = n)
		Return t
	ElseIf (l > n)
		Return Right(t,n)
	EndIf
EndFunction

Function iswap( a:Int Var, b:Int Var )
	Local u:Int = a
	a = b
	b = u
EndFunction

Function fswap( a:Float Var, b:Float Var )
	Local u:Float = a
	a = b
	b = u
EndFunction
Global swap (l:Float Var, h:Float Var) = fswap

Function frand :Float( l:Float, h:Float )
	If (l>h) Then fswap(l,h)
	Return l + ((h-l)*RndFloat())
EndFunction

Function fclamp:Float (n:Float, l:Float, h:Float)
	Return Max(Min(n,h),l)
EndFunction
Global clamp :Float (n:Float, l:Float, h:Float) = fclamp

Function iclamp:Int (n:Int, l:Int, h:Int)
	Return Max(Min(n,h),l)
EndFunction

Function fwrap:Float (n:Float, l:Float, h:Float)
	If (l > h)
		fswap(l,h)
	ElseIf ((h-l) = 0)
		Return l
	EndIf
	
	While (n < l)
		n = n + (h-l)
	EndWhile
	While (n >= h)
		n = n - (h-l)
	EndWhile
	Return n
EndFunction
Global wrap:Float (n:Float, l:Float, h:Float) = fwrap

Function iwrap:Int(n:Int, l:Int, h:Int)
	If (l > h)
		iswap(l,h)
	ElseIf ((h-l) = 0)
		Return l
	EndIf

	While (n < l)
		n = n + (h-l)
	EndWhile
	While (n >= h)
		n = n - (h-l)
	EndWhile
	Return n
EndFunction

Function in_range:int ( n:Float, l:Float, h:Float )
	If (n >= l) And (n < h)
		Return True
	EndIf
	Return False
EndFunction

Function in_range_x:int ( n:Float, l:Float, h:Float )
	If (n > l) And (n < h)
		Return True
	EndIf
	Return False
EndFunction

Function in_irange:int ( n:Int, l:Int, h:Int )
	If (n >= l) And (n < h)
		Return True
	EndIf
	Return False
EndFunction

Function in_irange_x:int ( n:Int, l:Int, h:Int )
	If (n > l) And (n < h)
		Return True
	EndIf
	Return False
EndFunction

Function line_in_range:int (a1:Float, a2:Float, b1:Float, b2:Float)
	Local u:Int = 0
	
	u = in_range(a1, b1, b2)
	u = u | in_range_x(a2, b1, b2)
	u = u | in_range(b1, a1, a2)
	u = u | in_range_x(b2, a1, a2)
	
	Return u
EndFunction

Function iline_in_irange:int (a1:Int, a2:Int, b1:Int, b2:Int)
	Local u:Int = 0
	
	u = in_irange(a1, b1, b2)
	u = u | in_irange_x(a2, b1, b2)
	u = u | in_irange(b1, a1, a2)
	u = u | in_irange_x(b2, a1, a2)
	
	Return u
EndFunction

Function point_in_rect:int( x:Float, y:Float, rx:Float, ry:Float, rw:Float, rh:Float)
	Return in_range(x,rx,rx+rw) And in_range(y,ry,ry+rh)
EndFunction

Function ipoint_in_irect:int( x:Int, y:Int, rx:Int, ry:Int, rw:Int, rh:Int)
	Return in_irange(x,rx,rx+rw) And in_irange(y,ry,ry+rh)
EndFunction

Function rect_in_rect:int( x1:Float, y1:Float, w1:Float, h1:Float, x2:Float, y2:Float, w2:Float, h2:Float)
	Return line_in_range( x1, x1+w1, x2, x2+w2 ) And line_in_range( y1, y1+h1, y2, y2+h2 )
EndFunction

Function irect_in_irect:int( x1:Int, y1:Int, w1:Int, h1:Int, x2:Int, y2:Int, w2:Int, h2:Int)
	Return iline_in_irange( x1, x1+w1, x2, x2+w2 ) And iline_in_irange( y1, y1+h1, y2, y2+h2 )
EndFunction

Function point_in_cube:int( x:Float, y:Float, z:Float, cx:Float, cy:Float, cz:Float, sx:Float, sy:Float, sz:Float )
	If (x >= cx) And (y >= cy) And (z >= cz) And (x < cx+sx) And (y < cy+sy) And (z < cz+sz)
		Return True
	EndIf
	Return False
EndFunction

Function ipoint_in_icube:int( x:Int, y:Int, z:Int, cx:Int, cy:Int, cz:Int, sx:Int, sy:Int, sz:Int )
	If (x >= cx) And (y >= cy) And (z >= cz) And (x < cx+sx) And (y < cy+sy) And (z < cz+sz)
		Return True
	EndIf
	Return False
EndFunction

Function cube_in_cube:int( x1:Float, y1:Float, z1:Float, sx1:Float, sy1:Float, sz1:Float, ..
	x2:Float, y2:Float, z2:Float, sx2:Float, sy2:Float, sz2:Float )
	
	Local u:Int = 0
	
	u = line_in_range( x1, x1+sx1, x2, x2+sx2 )
	u = u & line_in_range( y1, y1+sy1, y2, y2+sy2 )
	u = u & line_in_range( z1, z1+sz1, z2, z2+sz2 )
	
	Return u
EndFunction


Function icube_in_icube:int( x1:Int, y1:Int, z1:Int, sx1:Int, sy1:Int, sz1:Int, ..
	x2:Int, y2:Int, z2:Int, sx2:Int, sy2:Int, sz2:Int )
	
	Local u:Int = 0
	
	u = line_in_range( x1, x1+sx1, x2, x2+sx2 )
	u = u & line_in_range( y1, y1+sy1, y2, y2+sy2 )
	u = u & line_in_range( z1, z1+sz1, z2, z2+sz2 )
	
	Return u
EndFunction

Function dist2d:Float( x1:Float, y1:Float, x2:Float=0, y2:Float=0 )
	Return Sqr( pow( x1+x2, 2 ) + pow( y1+y2, 2 ) )
EndFunction
Global dist:Float( x1:Float, y1:Float, x2:Float=0, y2:Float=0 ) = dist2d

Function dist3d:Float( x1:Float, y1:Float, z1:Float, x2:Float=0, y2:Float=0, z2:Float=0)
	Return Sqr( pow(x1+x2,2) + pow(y1+y2,2) + pow(z1+z2,2) )
EndFunction

function idist2d:int( x1:int, y1:int, x2:int, y2:int )
	return abs(x2-x1) + abs(y2-y1)
endfunction

function idist3d :int( x1:int, y1:int, z1:int, x2:int, y2:int, z2:int )
	return abs(x2-x1) + abs(y2-y1) + abs(z2-z1)
endfunction

Function pow:Float(x:Float, n:Float)
	Return x ^ n
EndFunction
Global fpow:Float(x:Float,y:Float) = pow

Function ipow:Int(x:Int, n:Int)
	Return x ^ n
EndFunction

'' 3D functions for perspective space

Function sinf:Float(f:Float)
	Return Float(Sin(f/1000*360))
EndFunction

Function cosf:Float(f:Float)
	Return Float(Cos(f/1000*360))
EndFunction

function asinf:float( f:float )
	return float(asin(f))/180*500
endfunction

function acosf:float( f:float )
	return float(acos(f))/180*500
endfunction

'' ---------------------

Function rot_x:Float(f:Float)
	Return sinf(f)
EndFunction

Function rot_y:Float(f:Float)
	Return -cosf(f)
EndFunction

'' -------------------------------------------------------------------------------

function rot_xx:float( r:float, m:float=1.0 )
	return 1.0 * m
endfunction

function rot_xy:float( r:float, m:float=1.0 )
	return -cosf( r ) * m
endfunction

function rot_xz:float( r:float, m:float=1.0 )
	return -sinf( r ) * m
endfunction

function rot_yx:float( r:float, m:float=1.0 )
	return cosf(r) * m
endfunction

function rot_yy:float( r:float, m:float=1.0 )
	return 1.0 * m
endfunction

function rot_yz:float( r:float, m:float=1.0 )
	return sinf(r) * m
endfunction

function rot_zx:float( r:float, m:float=1.0 )
	return sinf(r) * m
endfunction

function rot_zy:float( r:float, m:float=1.0 )
	return -cosf(r) * m
endfunction

function rot_zz:float( r:float, m:float=1.0 )
	return 1.0 * m
endfunction

'' -------------------------------------------------------------------------------

global angle_x_d:float[] = new float[2]
function angle_x:float( y:float, z:float )
	local ay:float = asinf(y)
	local az:float = asinf(z)
	local r:float = 0
	
	if (y < 0) and (z < 0)
		r = -az
	elseif (y >= 0) and (z < 0)
		r = 250 + ay
	elseif (y >= 0) and (z >= 0)
		r = 500 + az
	elseif (y < 0) and (z >= 0)
		r = 750 - ay
	endif
	
	angle_x_d[0] = ay
	angle_x_d[1] = az
	return r
endfunction

global angle_y_d:float[] = new float[2]
function angle_y:float( x:float, z:float )
	local ax:float = asinf(x)
	local az:float = asinf(z)
	local r:float = 0
	
	if (x >= 0) and (z < 0)
		r = -az
	elseif (x < 0) and (z < 0)
		r = 250 - ax
	elseif (x < 0) and (z >= 0)
		r = 500 + az
	elseif (x >= 0) and (z >= 0)
		r = 750 +  ax
	endif
	
	angle_y_d[0] = ax
	angle_y_d[1] = az
	return r
endfunction

global angle_z_d:float[] = new float[2]
function angle_z:float( x:float, y:float )
	local ax:float = asinf(x)
	local ay:float = asinf(y)
	local r:float = 0
	
	if (x >= 0) and (y < 0)
		r = ax
	elseif (x >= 0) and (y >= 0)
		r = 250 + ay
	elseif (x < 0) and (y >= 0)
		r = 500 - ax
	elseif (x < 0) and (y < 0)
		r = 750 - ay
	endif
	
	angle_z_d[0] = ax
	angle_z_d[1] = ay
	return r
endfunction
global angle:float(x:float,y:float) = angle_z

'' -------------------------------------------------------------------------------



''	returns 3-cell array (unit vector x,y and magnitude)
Function unit2:Float[](x:Float, y:Float)
	Local m:Float = dist(x,y)
	Local r:Float[] = New Float[3]
	If (m = 0)
		r[0] = 0
		r[1] = 0
		r[2] = 0
	Else
		r[0] = x/m
		r[1] = y/m
		r[2] = m
	EndIf
	Return r
EndFunction

function unit:float[]( x:float, y:float )
	return unit2(x,y)
endfunction

function unit2:float[]( f:float )
	local r :float[] = new float[3]
	r[0] = sinf( f )
	r[1] = -cosf( f )
	r[2] = 1.0
	return r
endfunction

function unit:float[]( f:float )
	return unit2( f )
endfunction

''	returns 4-cell array (unit vector x,y,z and magnitude)
global unit3_d:float[] = new float[4]
Function unit3:Float[](x:Float, y:Float, z:Float)
	Local m:Float = dist3d(x,y,z)
	Local r:Float[] = New Float[4]
	If (m = 0)
		r[0] = 0
		r[1] = 0
		r[2] = 0
		r[3] = 0
	Else
		r[0] = x/m
		r[1] = y/m
		r[2] = z/m
		r[3] = m
	EndIf
	
	unit3_d[0] = r[0]
	unit3_d[1] = r[1]
	unit3_d[2] = r[2]
	unit3_d[3] = r[3]
	
	Return r
EndFunction

''''''''''''''''''''
'' Array Creation ''
''''''''''''''''''''

function barray:byte[] (n:int, v:byte)
	local r:byte[] = new byte[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function barray2:byte[,] (x:int, y:int, v:byte)
	local r:byte[,] = new byte[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function barray3:byte[,,] (x:int, y:int, z:int, v:byte)
	local r:byte[,,] = new byte[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

'''''''''''''''''

function warray:short[] (n:int, v:short)
	local r:short[] = new short[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function warray2:short[,] (x:int, y:int, v:short)
	local r:short[,] = new short[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function warray3:short[,,] (x:int, y:int, z:int, v:short)
	local r:short[,,] = new short[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

''''''''''''

function iarray:int[] (n:int, v:int)
	local r:int[] = new int[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function iarray2:int[,] (x:int, y:int, v:int)
	local r:int[,] = new int[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function iarray3:int[,,] (x:int, y:int, z:int, v:int)
	local r:int[,,] = new int[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

'''''''''''''''''''''''

function larray:long[] (n:int, v:long)
	local r:long[] = new long[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function larray2:long[,] (x:int, y:int, v:long)
	local r:long[,] = new long[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function larray3:long[,,] (x:int, y:int, z:int, v:long)
	local r:long[,,] = new long[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

'''''''''''''''''''''''''''''''''''

function farray:float[] (n:int, v:float)
	local r:float[] = new float[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function farray2:float[,] (x:int, y:int, v:float)
	local r:float[,] = new float[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function farray3:float[,,] (x:int, y:int, z:int, v:float)
	local r:float[,,] = new float[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

'''''''''''''''''''''''''''''

function darray:double[] (n:int, v:double)
	local r:double[] = new double[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function darray2:double[,] (x:int, y:int, v:double)
	local r:double[,] = new double[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function darray3:double[,,] (x:int, y:int, z:int, v:double)
	local r:double[,,] = new double[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

'''''''''''''''''''''''''

function sarray:string[] (n:int, v:string)
	local r:string[] = new string[n]
	for local i:int = 0 to n-1
		r[i] = v
	next
	return r
endfunction

function sarray2:string[,] (x:int, y:int, v:string)
	local r:string[,] = new string[x,y]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			r[i,j] = v
		next
	next
	return r
endfunction

function sarray3:string[,,] (x:int, y:int, z:int, v:string)
	local r:string[,,] = new string[x,y,z]
	for local i:int = 0 to x-1
		for local j:int = 0 to y-1
			for local k:int = 0 to z-1
				r[i,j,k] = v
			next
		next
	next
	return r
endfunction

''''''''''''''''''''''''''''''''''''''''''''

function astr:string(b:byte[])
  return string.fromcstring(b)
endfunction


function strdec:string( f:float=0.0, d:int=2 )
	d = iclamp(d,0,10)
	if abs(f) < 1.0 * (10.0 ^ -d) then f = 0.0
	local r:string
	local w:string[] = tokenize(string(f),".")
	if (d > 0)
		r = w[0] + "." + left(w[1],d)
	else
		r = w[0]
	endif
	return r
endfunction

function strdbl:string( f:double=0.0, d:int=2 )
	d = iclamp(d,0,10)
	if abs(f) < 1.0 * (10.0 ^ -d) then f = 0.0
	local r:string
	local w:string[] = tokenize(string(f),".")
	if (d > 0)
		r = w[0] + "." + left(w[1],d)
	else
		r = w[0]
	endif
	return r
endfunction

function roundint:int( f:float )
	local n:float
	if (f < 0) then n = -1.0 else n = 1.0 
	local u:float = abs(f)
	local v:float = u - floor(u)
	if (u >= 0.5)
		return n * ceil(u)
	else
		return n * floor(u)
	endif
endfunction

function froundint:float( f:float )
  return float(roundint(f))
endfunction

function fmod:float( v:float, m:float=1.0 )
  m = low(m,0)
  if m = 0 then return v
  return wrap(v,0,m)
endfunction

function imod:int( v:int, m:int=1 )
  m = ilow(m,0)
  if m = 0 then return v
  if m = 1 then return 0
  return iwrap(v,0,m)
endfunction

function iround:int( n:int, d:int=10 )
  local v:int = imod(n,d)
  local w:int = d-v
  if (v >= w) then return n+w
  return n-v 
endfunction

function fround:float( n:float, d:float=1.0 )
  if abs(n) < (d/2) then return 0.0
  local v:float = fmod(n,d)
  local w:float = d-v
  if (v >= w) then return n+w
  return n-v 
endfunction

function char:string(s:string, n:int)
	return chr( s[n] )
endfunction

'' locks a position to increments of 0.1 (screen pixels)
function pixel:float(f:float)
	return ffloor(f*10) / 10
endfunction

function cut:string[](s:string, n:int=80)
	local l:int = len(s)
	local r:string[] = new string[(l/n)+1]
	if (l <= n)
		r[0] = s
	else
		local ll:int = len(r)-1
		for local i:int = 0 to ll
			if (i = ll)
				r[i] = s[i*n..l]
			else
				r[i] = s[i*n .. (i*n)+n]
			endif
		next
	endif
	return r
endfunction

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

