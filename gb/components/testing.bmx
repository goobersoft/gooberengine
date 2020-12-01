
''''''''''
'' type ''
''''''''''

type t_gb_testing
  field logchain :t_logchain
endtype

''''''''''''
'' global ''
''''''''''''

global gb_testing :t_gb_testing

''''''''''''
'' Events ''
''''''''''''

function gb_testing_init()
	gb_testing              = new t_gb_testing
  gb_testing.logchain     = new_logchain()
endfunction

function gb_testing_load()
endfunction

function gb_testing_start()
  for local i:int = 1 to 10
    logchain_add(gb_testing.logchain, rpad(string(i),2,"0"))
  next
  for local i:int = 1 to 14
    logchain_next(gb_testing.logchain)
    print(logchain_get(gb_testing.logchain))
  next
endfunction

function gb_testing_update(d:float)
endfunction

function gb_testing_draw(x:float=0, y:float=0)
endfunction

function gb_testing_end()
endfunction