type t_matrix
	field height:int
	field width:int
	field data:float[,]
endtype

function new_matrix:t_matrix(h:int=3, w:int=3)
	h = low(h,1)
	w = low(w,1)
	
	local r:t_matrix = new t_matrix
	r.height 	= h
	r.width	  = w
	r.data 		= new float[h,w]
	
	for local i:int = 0 to min(h,w)-1
		r.data[i,i] = 1.0
	next
	return r
endfunction

function clone_matrix:t_matrix(m:t_matrix)
	local r:t_matrix = new t_matrix
	r.width = m.width
	r.height = m.height
	r.data = new float[r.height, r.width]
	for local i:int = 0 to m.width-1
		for local j:int = 0 to m.height-1
			r.data[j,i] = m.data[j,i]
		next
	next
	return r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function matrix_plot(m:t_matrix, y:int, x:int, f:float)
	y = iclamp(y-1,0,m.height-1)
	x = iclamp(x-1,0,m.width-1)
	m.data[y,x] = f
endfunction

function matrix_add:t_matrix( m1:t_matrix, m2:t_matrix )
	local r:t_matrix
	if (m1.height = m2.height) and (m1.width = m2.width) 
		r = new_matrix(m1.width, m1.height)
		for local j:int = 0 to m1.height-1
			for local i:int = 0 to m1.width-1
				r.data[j,i] = m1.data[j,i] + m2.data[j,i]
			next
		next
	endif
	return r
endfunction

function matrix_multiply:t_matrix( m1:t_matrix, m2:t_matrix )
	local r:t_matrix
	local k:int
	local t:float = 0
	if (m1.width = m2.height)
		r = new_matrix(m1.height, m2.width)
		for local j:int = 0 to r.height-1
			for local i:int = 0 to r.width-1
				t = 0
				for local k:int = 0 to m1.width-1
					t = t + (m1.data[j,k] * m2.data[k,i])
				next
				r.data[j,i] = t
			next
		next
	endif
	return r
endfunction

function matrix_tostring:string(m:t_matrix)
	local r:string = "{"
	for local j:int = 0 to m.height-1
		r = r + "{"
		for local i:int = 0 to m.width-1
			if i = m.width-1
				r = r + strdec(m.data[j,i],3) + "}"
			else
				r = r + strdec(m.data[j,i],3) + " "
			endif
		next
		if (j <> m.height-1)
			r = r + " "
		else
			r = r + "}"
		endif
	next
	return r
endfunction