
''''''''''''
'' consts ''
''''''''''''

const gb_audio_max_channels :int = 10

'''''''''''''
'' globals ''
'''''''''''''

global gb_audio_active    :t_bool
global gb_audio_channels  :tchannel[]

''''''''''''
'' events ''
''''''''''''

function gb_audio_init()
  EnableOpenALAudio()
	setaudiodriver("OpenAL")
  gb_audio_active      = new_bool(true)
  gb_audio_channels    = new tchannel[ gb_audio_max_channels ]
  for local i:int = 0 to gb_audio_max_channels-1
    gb_audio_channels[i] = allocchannel()
  next
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_audio_stop_all()
  for local i:int = 0 to gb_audio_max_channels-1
    gb_audio_stop(i)
  next
endfunction

function gb_audio_stop(c:int=0)
  c = iclamp(c,0,gb_audio_max_channels-1)
  playsound( gb_assets_sounds[80].source, gb_audio_channels[c] )
endfunction

function gb_audio_set_volume(c:int=0, v:float=1.0)
  c = iclamp(c,0,gb_audio_max_channels-1)
  v = clamp(v,0,1.0)
  setchannelvolume( gb_audio_channels[c], v )
endfunction

function gb_audio_set_volume_all(v:float=1.0)
  for local i:int = 0 to gb_audio_max_channels-1
    gb_audio_set_volume(i,v)
  next
endfunction

function gb_audio_set_rate(c:int=0, v:float=1.0)
  c = iclamp(c,0,gb_audio_max_channels-1)
  v = clamp(v,0,10.0)
  setchannelrate( gb_audio_channels[c], v )
endfunction

function gb_audio_set_rate_all(v:float=1.0)
  for local i:int = 0 to gb_audio_max_channels-1
    gb_audio_set_rate(i,v)
  next
endfunction

function gb_audio_play_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	c = iclamp(c,0,gb_audio_max_channels-1)
  n = iwrap( n,0,gb_assets_max_sounds )
	if (gb_assets_sounds[n])
		setchannelrate( gb_audio_channels[c], r )
		setchannelvolume( gb_audio_channels[c], v )
		playsound( gb_assets_sounds[n].source, gb_audio_channels[c] )
	endif
endfunction

function gb_audio_cue_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	c = iclamp(c,0,gb_audio_max_channels-1)
  n = iwrap( n,0,gb_assets_max_sounds )
	if (gb_assets_sounds[n])
		if channelplaying( gb_audio_channels[c] ) = false
			setchannelrate( gb_audio_channels[c], r )
			setchannelvolume( gb_audio_channels[c], v )
			playsound( gb_assets_sounds[n].source, gb_audio_channels[c] )
		endif
	endif
endfunction
