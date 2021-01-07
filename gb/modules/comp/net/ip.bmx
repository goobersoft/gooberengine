
type t_gb_net_ip
  field a:byte
  field b:byte
  field c:byte
  field d:byte
  field port:short
endtype

function new_gb_net_ip:t_gb_net_ip (a:byte=127, b:byte=0, c:byte=0, d:byte=1, p:short=53350)
  local r:t_gb_net_ip = new t_gb_net_ip
  gb_net_ip_set(r, a,b,c,d,p)
  return r
endfunction

function new_gb_net_ip:t_gb_net_ip (s:string)
  local v:string[] = tokenize(s,".")
  local r:t_gb_net_ip = new t_gb_net_ip
  r.a = byte(v[0])
  r.b = byte(v[1])
  r.c = byte(v[2])
  r.d = byte(v[3])
  return r
endfunction

''''

function gb_net_ip_set(u:t_gb_net_ip, a:byte, b:byte, c:byte, d:byte, p:short=53350)
  u.a = a
  u.b = b
  u.c = c
  u.d = d
  u.port = p
endfunction

function gb_net_ip_set_ip(u:t_gb_net_ip, a:byte, b:byte, c:byte, d:byte)
  u.a = a
  u.b = b
  u.c = c
  u.d = d
endfunction

function gb_net_ip_set_port(b:t_gb_net_ip, p:short)
  b.port = p
endfunction

function gb_net_ip_to_string:string(b:t_gb_net_ip, p:int=false)
  if p
    return string(b.a) + "." + string(b.b) + "." + string(b.c) + "." + string(b.d) + "." + string(b.port)
  else
    return string(b.a) + "." + string(b.b) + "." + string(b.c) + "." + string(b.d)
  endif
endfunction

function gb_net_ip_to_int:int(b:t_gb_net_ip)
  return rgba(b.a, b.b, b.c, b.d)
endfunction