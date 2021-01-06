
'''''''''''''
'' mstring ''
'''''''''''''
'' mutable string

type t_mstring
  field chars     :t_list
  field seek      :t_list_node
  field seekpos   :t_int
endtype

function new_mstring:t_mstring(s:string="")
  local r:t_mstring = new t_mstring
  r.chars = new_list()
  r.seekpos = new_int(-1)
  mstring_add(r, s)
  return r
endfunction

function mstring_add(m:t_mstring, s:string)
  if len(s) > 0
    for local i:int = 0 to len(s)-1
      list_add_last(m.chars,"byte", new_byte(s[i]) )
    next
  endif
endfunction

function mstring_type_insert(m:t_mstring, s:string)
endfunction

function mstring_type_replace(m:t_string, s:string)
endfunction

function mstring_type_delete(m:t_string, n:int)
endfunction

function mstring_type_delete(m:t_string, s:string)
  mstring_type_delete(m,len(s))
endfunction

function mstring_type_backspace(m:t_string, n:int)
endfunction

function mstring_type_seek(m:t_mstring, n:int)
  if m.chars.count.value>0

    if not m.seek then m.seek = m.chars.first

    local u1:int = m.seekpos.value
    local u2:int = iclamp(u1+n,0,m.chars.count.value-1)
    local d:int  = u2-u1
    int_set(m.seekpos, u2)

    while d<0
      m.seek = m.seek.left
      d=d+1
    wend
    while d>0
      m.seek = m.seek.right
      d=d+1
    wend
  endif
endfunction

function mstring_length:int(m:t_mstring)
  return m.chars.count.value
endfunction

function mstring_to_string:string(m:t_mstring)
  local a:byte[] = new byte[ m.chars.count.value ]
  if a.length > 0
    for local i:int = 0 to a.length-1
      a[i] = byte_get(list_iterate(m.chars))
    next
  endif
  return astr(a)
endfunction