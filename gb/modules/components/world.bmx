

''''''''''''''''
'' world type ''
''''''''''''''''
''  this type holds entities which are all part of a 3D environment made of voxels.
''  The world will queue all of the commands for the entities in a drawstack and
''  then draw everything at once.

type t_world
  field drawstack   :t_drawstack
  field entities    :t_list
endtype

function new_world:t_world ()
  local r:t_world = new t_world
  r.drawstack   = new_drawstack()
  r.entities    = new_list()
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function world_update(w:t_world)
  
endfunction