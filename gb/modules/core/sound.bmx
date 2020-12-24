
type t_sound
	field source 	  :tsound
	field path 		  :string
endtype

function new_sound:t_sound ( p:string )
  local r:t_sound = new t_sound
  r.source = loadsound(p,0)
  r.path  = p
  return  r
endfunction

function sound_source:tsound( s:t_sound )
  return s.source
endfunction

function sound_path:string ( u:t_sound )
  return u.path
endfunction