
type t_image
	field source 	  :timage
	field path 		  :string
endtype

function new_image:t_image ( p:string )
  local r:t_image = new t_image
  r.source = loadimage(p,DYNAMICIMAGE)
  r.path  = p
  return  r
endfunction

function image_source:timage( u:t_image )
  return u.source
endfunction

function image_width:int ( u:t_image )
  return imagewidth(u.source)
endfunction

function image_height:int ( u:t_image )
  return imageheight(u.source)
endfunction

function image_path:string ( u:t_image )
  return u.path
endfunction

function image_draw( u:t_image, x:float, y:float )
  gb_graph_draw_image( x, y, u.source )
endfunction