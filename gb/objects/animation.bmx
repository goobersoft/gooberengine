
''''''''''''
'' consts ''
''''''''''''

Const animation_status_stopped	:Int		= 0
Const animation_status_playing	:Int		= 1
Const animation_status_paused		:Int		= 2
Const animation_status_looping	:Int		= 3

''''''''''
'' Type ''
''''''''''

Type t_animation
	Field status				:t_int
	Field pos						:t_int
	Field length				:t_int
	Field framedelay		:t_float
	Field frames				:t_animframe[]
	field stoptofirst		:t_bool
endtype

'' create an animation with blank frames
function new_animation:t_animation(l:int=1, s:int=false)
	local r:t_animation = new t_animation
	r.status 			= new_int( animation_status_stopped )
	r.pos 				= new_int( undefined )
	r.length			= new_int( max(1,l) )
	r.framedelay	= new_float(0)
	r.frames			= new t_animframe[int_get(r.length)]
	r.stoptofirst = new_bool(s)
	return r
endfunction

'' create an animation from predefined frames
function new_animation:t_animation(a:t_animframe[], s:int=false)
	local r:t_animation = new t_animation
	r.status 			= new_int( animation_status_stopped )
	r.pos 				= new_int( undefined )
	r.framedelay	= new_float(0)
	r.frames			= a
	r.length			= new_int( a.length )
	r.stoptofirst = new_bool(s)
	return r
endfunction

function animation_get_current_frame:t_ibox(a:t_animation)
	if int_neq(a.pos, undefined)
		if (a.frames[ int_get(a.pos) ])
			return a.frames[ int_get(a.pos) ].ibox
		endif
	endif
endfunction

function animation_add_frame(a:t_animation, n:int, f:t_animframe)
	n = clamp(n,0,int_get(a.length))
	a.frames[n] = f
endfunction

function animation_add_frame(a:t_animation, n:int, id:int, fx:float, fy:float, fw:float, fh:float, fd:float)
	animation_add_frame(a, n, new_animframe( id,fx,fy,fw,fh,fd )	)
endfunction

function animation_find_empty_frame:int(a:t_animation)
	for local i:int = 0 to int_get(a.length)-1
		if (a.frames[i] = null)
			return i
		endif
	next
	return undefined
endfunction
	
function animation_play:int(a:t_animation, s:int=false)
	if int_gt(a.length, 0)
		int_set( a.status, animation_status_playing )
		bool_set( a.stoptofirst, s )
		if int_eq(a.pos, undefined)
			int_set(a.pos, 0)
			float_set(a.framedelay, float_get(a.frames[ int_get(a.pos)].framedelay))
		endif
		return true
	endif
	return false
endfunction

function animation_loop:int(a:t_animation, s:int=false)
	if int_gt(a.length, 0)
		int_set( a.status, animation_status_looping )
		bool_set( a.stoptofirst, s )
		if int_eq(a.pos, undefined)
			int_set(a.pos, 0)
			float_set(a.framedelay, float_get(a.frames[ int_get(a.pos)].framedelay))
		endif
		return true
	endif
	return false
endfunction

function animation_pause(a:t_animation)
	int_set(a.status, animation_status_paused)
endfunction
	
function animation_stop(a:t_animation)
	int_set(a.status, animation_status_stopped)
	int_set(a.pos, undefined)
	float_set(a.framedelay,0)
endfunction
		
''''''''''''
'' events ''
''''''''''''

function animation_update(a:t_animation, d:Float)
	Local q:Int	= int_get(a.length)
	Local m:Int = False
	
	While (m = False) And (q > 0)
		
		if int_eq(a.status, animation_status_stopped) or int_eq(a.status, animation_status_stopped)
			m = true
		Else
			if float_lte(a.framedelay, 0)
				if int_eq(a.pos, int_get(a.length)-1)
					if int_eq(a.status, animation_status_playing)
						int_set(a.status, animation_status_stopped)
						if bool_eq(a.stoptofirst,true)
							int_set(a.pos, 0)
						endif
						m = true
					elseif int_eq(a.status, animation_status_looping)
						int_set(a.pos, 0)
						if (a.frames[int_get(a.pos)] <> null)
							float_add(a.framedelay, float_get(a.frames[int_get(a.pos)].framedelay))
							m = true
						endif
					endif
				elseif int_lt(a.pos, int_get(a.length)-1)
					int_add(a.pos, 1)
					if (a.frames[int_get(a.pos)] <> null)
						float_add(a.framedelay, float_get(a.frames[int_get(a.pos)].framedelay))
						m = true
					else
						float_set(a.framedelay,0)
					endif
				endif
			
			elseif float_gt(a.framedelay, 0)
				float_add(a.framedelay, -d)
				m = true
			endif
		endif
		q = q - 1
	Wend
	If (q = 0)
		int_set		(a.status, animation_status_stopped)
		float_set	(a.framedelay, 0)
		int_set		(a.pos, undefined)
	EndIf
endfunction