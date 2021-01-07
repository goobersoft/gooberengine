
include "net/ip.bmx"

''''''''''''
'' consts ''
''''''''''''

const gb_net_listen_port :int = 53350

''''''''''
'' type ''
''''''''''

type t_gb_net
  field socket      :tsocket
  field stream      :tstream
  field dstinfo     :taddrinfo[]
  field connected   :t_bool
endtype

function new_gb_net:t_gb_net ()
  local r:t_gb_net = new t_gb_net
  r.socket      = createudpsocket()
  r.stream      = createsocketstream(r.socket)
  r.connected   = new_bool(false)
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_net_init()
  gb.net = new_gb_net()
  bindsocket(gb.net.socket,gb_net_listen_port)
endfunction

function gb_net_start()
endfunction

function gb_net_update()
  
endfunction

function gb_net_end()
  closestream(gb.net.stream)
  closesocket(gb.net.socket)
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_net_connect:int(n:string)
  gb.net.dstinfo = addrinfo(n,string(gb_net_listen_port),af_inet_)
  local d:int = connectsocket(gb.net.socket, gb.net.dstinfo[0])
  if (d)
    bool_set(gb.net.connected, true)
  else
    bool_set(gb.net.connected, false)
  endif
  return gb.net.connected.value
endfunction

function gb_net_read:string()
  if bool_eq(gb.net.connected)
    local d:int = socketreadavail(gb.net.socket)
    local s:string = readstring(gb.net.stream, d)
    return s
  endif
  return ""
endfunction

function gb_net_write(s:string)
  if bool_eq(gb.net.connected)
    writestring(gb.net.stream, s)
  endif
endfunction