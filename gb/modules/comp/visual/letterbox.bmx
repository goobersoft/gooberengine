
type t_gb_visual_letterbox
  field aspect		:t_float
  field diagonal	:t_float
  field rect			:t_rect
  field irect		  :t_irect
  field scales		:t_point
endtype

function new_gb_visual_letterbox:t_gb_visual_letterbox ()
  local r:t_gb_visual_letterbox = new t_gb_visual_letterbox
  r.aspect    = new_float(5.0/3.0)
  r.diagonal  = new_float(dist2d(0,0,800,480))
  r.rect      = new_rect(0,0,800,480)
  r.irect     = new_irect(0,0,800,480)
  r.scales    = new_point(1.0,1.0)
  return r
endfunction