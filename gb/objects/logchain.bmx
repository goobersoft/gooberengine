

'''''''''''''''''''
'' logchain node ''
'''''''''''''''''''

type t_logchain_node
  field data        :string
  field chain_prev  :t_logchain_node
  field chain_next  :t_logchain_node
endtype

function new_logchain_node:t_logchain_node (s:string="")
  local r:t_logchain_node = new t_logchain_node
  r.data = s
  return r
endfunction

function logchain_node_set( l:t_logchain_node, s:string="" )
  l.data = s
endfunction

'''''''''''''''''''
'' logchain type ''
'''''''''''''''''''

type t_logchain
  field length      :t_int
  field chain_curr  :t_logchain_node
  field chain_seek  :t_logchain_node
endtype

'''''''''
'' new ''
'''''''''

function new_logchain:t_logchain ()
  local r:t_logchain = new t_logchain
  r.length = new_int(0)
  return r
endfunction

function new_logchain_ex:t_logchain (n:int)
  n = ilow(n,1)
  local r:t_logchain = new_logchain()
  for local i:int = 1 to n
    logchain_add(r,"")
  next 
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function logchain_add(l:t_logchain, s:string)
  local c:t_logchain_node = new_logchain_node(s)
  if int_eq(l.length,0)
    l.chain_curr = c
    l.chain_curr.chain_next = c
    l.chain_curr.chain_prev = c
    l.chain_seek = c
  elseif int_gte(l.length,1)
    local d1:t_logchain_node = l.chain_curr
    local d2:t_logchain_node = l.chain_curr.chain_next
    l.chain_curr = c
    l.chain_curr.chain_prev = d1
    l.chain_curr.chain_next = d2
    d1.chain_next = l.chain_curr
    d2.chain_prev = l.chain_curr
    if l.chain_seek = d1
      l.chain_seek = l.chain_curr
    endif
  endif
  int_add(l.length,1)
endfunction

function logchain_remove(l:t_logchain)
  if int_eq(l.length,1)
    int_set(l.length,0)
    l.chain_curr = null
    l.chain_seek = null
  elseif int_gte(l.length,2)
    local c:t_logchain_node = l.chain_curr
    c.chain_prev.chain_next = c.chain_next
    c.chain_next.chain_prev = c.chain_prev
    if l.chain_seek = c
      l.chain_seek = l.chain_curr
    endif
    int_add(l.length,-1)
  endif
endfunction

function logchain_get:string (l:t_logchain)
  if l.chain_curr
    return l.chain_curr.data
  endif
  return ""
endfunction

''  this will set the current line imn the logchain.
''  if you want to add new unique lines, use logchain_add.
''  This does nothing if there are no links in the chain.
function logchain_set (l:t_logchain, s:string )
  if l.length.value > 0
    logchain_node_set(l.chain_curr, s)
  endif
endfunction

function logchain_next:string(l:t_logchain)
  if l.length.value > 0
    l.chain_curr = l.chain_curr.chain_next
    return l.chain_curr.data
  endif
  return ""
endfunction

function logchain_prev:string(l:t_logchain)
  if l.length.value > 0
    l.chain_curr = l.chain_curr.chain_prev
    return l.chain_curr.data
  endif
  return ""
endfunction

function logchain_to_array:string[] ( l:t_logchain )
  
  if l.length.value > 0
    local s:string[] = new string[l.length.value]
    for local i:int = 0 to l.length.value-1
      logchain_next(l)
      s[i] = l.chain_curr.data
    next
    return s
  endif
  return new string[0]
endfunction

function logchain_seek_back(l:t_logchain, n:int=1)
  n = ilow(n,1)
  for local i:int = 1 to n
    l.chain_seek = l.chain_seek.chain_prev
  next
endfunction

function logchain_seek_forward(l:t_logchain, n:int=1)
  n = ilow(n,1)
  for local i:int = 1 to n
    l.chain_seek = l.chain_seek.chain_next
  next
endfunction

function logchain_reset_seek(l:t_logchain)
  l.chain_seek = l.chain_curr
endfunction