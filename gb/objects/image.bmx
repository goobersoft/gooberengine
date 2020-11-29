
type t_image
	field image 	  :timage
	field path 		  :string
endtype

function new_image:t_image ( p:string )
  local r:t_image = new t_image
  r.image = loadimage(p)
  r.path  = p
  return  r
endfunction

function image_width:int ( u:t_image )
  return imagewidth(u.image)
endfunction

function image_height:int ( u:t_image )
  return imageheight(u.image)
endfunction

function image_path:string ( u:t_image )
  return u.path
endfunction