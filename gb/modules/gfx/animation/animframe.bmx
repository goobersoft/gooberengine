

'''''''''''''''
'' animframe ''
'''''''''''''''

Type t_animframe
	Field ibox					:t_ibox
	Field framedelay		:t_float
endtype

function new_animframe:t_animframe(id:Int=0, ix:Float=0, iy:Float=0, iw:Float=1, ih:Float=1, d:Float=0.1)
	local r:t_animframe = new t_animframe
	r.ibox 				= new_ibox(id,ix,iy,iw,ih)
	r.framedelay 	= new_float(d)
	return r
endfunction