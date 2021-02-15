
const dialog_max_lines  :int = 3
const dialog_box_x      :int = 14
const dialog_box_y      :int = 12

type t_dialog

  field pos     :t_int
  field values  :t_string[]
  field boxid   :t_number
  field fontid  :t_number

endtype

function new_dialog:t_dialog()
  local r:t_dialog = new t_dialog
  r.pos       = new_int(0)
  r.values    = new_string_v(3,"")
  r.boxid     = new_number(0,0,10)
  r.fontid    = new_number(2,0,2)
  return r
endfunction

function dialog_draw(d:t_dialog, x:float=0, y:float=0)
endfunction

function dialog_draw3d(d:t_dialog, x:float=0, y:float=0, z:float=0)
endfunction