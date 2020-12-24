
const gbs_node_id_assign :int = 1
const gbs_node_id_equal  :int = 2

'''''''''''''''
'' base node ''
'''''''''''''''

type t_gbs_node
  field id        :int
  field keyword   :string
  field aliases   :string[]
  field params    :object[]
  field on_true   :t_gbs_node
  field on_false  :t_gbs_node
  field data      :object
endtype

function new_gbs_node:t_gbs_node(id:int=0)
  local r:t_gbs_node = new t_gbs_node
  r.id        = id
  r.keyword   = ""
  r.aliases   = []
  r.params    = []
  r.on_true   = null
  r.on_false  = null
  r.data      = null
  return      r
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gbs_node_set_keyword:string (n:t_gbs_node, s:string)
  n.keyword = s
endfunction

function gbs_node_set_aliases:string (n:t_gbs_node, s:string[])
  n.aliases = s
endfunction

function gbs_node_set_true_node (n:t_gbs_node, m:t_gbs_node)
  n.on_true = m
endfunction

function gbs_node_set_false_node (n:t_gbs_node, m:t_gbs_node)
  n.on_false = m
endfunction

function gbs_node_set_data (n:t_gbs_node, d:object)
  n.data = d
endfunction

function gbs_node_set_param (n:t_gbs_node, p:int, o:object)
endfunction

'''''''''
'' set ''
'''''''''

function new_gbs_node_assign:t_gbs_node (n:string="", v:string="")
  local r:t_gbs_node = new_gbs_node(gbs_node_id_assign)
  r.keyword = "assign"
  r.aliases = ["set","let"]
  r.params  = [Object(new_string("")), object(new_dictval())]
  return r
endfunction

