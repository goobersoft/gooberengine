
const gball_response_maybe  :int = 0
const gball_response_yes    :int = 1
const gball_response_no     :int = 2

type t_gball_response
  field value     :t_string
  field intent    :t_int
endtype

function new_gball_response:t_gball_response (v:string="", i:int=0)
  local r:t_gball_response = new t_gball_response
  r.value = new_string(v)
  r.intent = new_int(i)
  return r
endfunction

function gball_response_generate(g:t_gball_response)
  global s :string[]
  global u :int[]
  if s = null
    s = new string[20]
    u = new int[20]

    local c:t_counter = new_counter()

    s[counter_inc(c)] = "Reply hazy. Try again."
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Didn't catch that. Try again please."
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Poor signal. Once more please."
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Hey, what's the wifi password?"
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Were you saying something?"
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Zzz..."
    u[counter_get(c)] = gball_response_maybe

    s[counter_inc(c)] = "Oh my yes!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "Yep!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "Results look good!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "Definitely!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "No! ... Wait, I mean, yes!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "The sky is the limit!"
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "All is in your favor."
    u[counter_get(c)] = gball_response_yes

    s[counter_inc(c)] = "Things look grim..."
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "Nope."
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "Absolutely not."
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "Not a chance."
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "No way!"
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "Ha! You wish!"
    u[counter_get(c)] = gball_response_no

    s[counter_inc(c)] = "Sorry, but no..."
    u[counter_get(c)] = gball_response_no
  endif

  local rr:int = rand(0,19)
  g.value   = new_string( s[rr] )
  g.intent  = new_int( u[rr] )
endfunction