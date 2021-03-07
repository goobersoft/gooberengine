
''''''''''''''
'' includes ''
''''''''''''''

'' random things
include "modules/misc/gball.bmx"
include "modules/misc/redskull.bmx"

'' testing module.
include "modules/testing/testing.bmx"



''''''''''''
'' events ''
''''''''''''

function gf_init()
  gf_testing_init()
endfunction

function gf_load()
  gf_testing_load()
endfunction

function gf_start()
  gf_testing_start()
endfunction

function gf_update()
  gf_testing_update()
endfunction

function gf_draw()
  gf_testing_draw()
endfunction

function gf_end()
  gf_testing_end()
endfunction