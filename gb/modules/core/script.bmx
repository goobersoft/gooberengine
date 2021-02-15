
include "script/node.bmx"

type t_script

  '' a set of global variables
  field vars        :t_dict
  
  '' a script can have multiple nodes running at the same time.
  field nodeslist   :t_list

  '' a set of nodes which can be referenced later using goto.
  field bookmarks   :t_dict


endtype

function new_script:t_script ()
  local r:t_script = new t_script
  r.vars        = new_dict()
  r.nodeslist   = new_list()
  return r
endfunction