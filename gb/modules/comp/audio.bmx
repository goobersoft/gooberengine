
''''''''''''
'' consts ''
''''''''''''

const gb_audio_max_channels :int = 10

''''''''''
'' type ''
''''''''''

type t_gb_audio
  field active    :t_bool
  field channels  :tchannel[]
endtype

function new_gb_audio:t_gb_audio()
  local r:t_gb_audio = new t_gb_audio
  r.active      = new_bool(true)
  r.channels    = new tchannel[ gb_audio_max_channels ]
  for local i:int = 0 to gb_audio_max_channels-1
    r.channels[i] = allocchannel()
  next
  return r
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_audio_init()
  EnableOpenALAudio()
	setaudiodriver("OpenAL")

  gb.audio = new_gb_audio()
endfunction

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_audio_play_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	c = iclamp(c,0,gb_audio_max_channels-1)
  n = iwrap( n,0,gb_assets_max_sounds )
	if (gb.assets.sounds[n])
		setchannelrate( gb.audio.channels[c], r )
		setchannelvolume( gb.audio.channels[c], v )
		playsound( gb.assets.sounds[n].source, gb.audio.channels[c] )
	endif
endfunction

function gb_audio_cue_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	c = iclamp(c,0,gb_audio_max_channels-1)
  n = iwrap( n,0,gb_assets_max_sounds )
	if (gb.assets.sounds[n])
		if channelplaying( gb.audio.channels[c] ) = false
			setchannelrate( gb.audio.channels[c], r )
			setchannelvolume( gb.audio.channels[c], v )
			playsound( gb.assets.sounds[n].source, gb.audio.channels[c] )
		endif
	endif
endfunction
