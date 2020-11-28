
''''''''''''
'' consts ''
''''''''''''

const gb_audio_max_channels :int = 10
const gb_audio_max_sounds 	:int = 100

'''''''''''''
'' Globals ''
'''''''''''''

global gb_audio_channels 	:tchannel[]

'''''''''''''''
'' functions ''
'''''''''''''''

function gb_audio_play_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	n = iclamp(n,0,gb_audio_max_sounds-1)
	c = iclamp(c,0,gb_audio_max_channels-1)
	if (gb_sounds[n])
		setchannelrate( gb_audio_channels[c], r )
		setchannelvolume( gb_audio_channels[c], v )
		playsound( gb_sounds[n], gb_audio_channels[c] )
	endif
endfunction

function gb_audio_cue_sound(n:int, c:int=0, r:float=1.0, v:float=1.0)
	n = iclamp(n,0,gb_audio_max_sounds-1)
	c = iclamp(c,0,gb_audio_max_channels-1)
	if gb_sounds[n]
		if channelplaying( gb_audio_channels[c] ) = false
			setchannelrate( gb_audio_channels[c], r )
			setchannelvolume( gb_audio_channels[c], v )
			playsound( gb_sounds[n], gb_audio_channels[c] )
		endif
	endif
endfunction

''''''''''''
'' events ''
''''''''''''

function gb_audio_init()
	
	EnableOpenALAudio()
	setaudiodriver("OpenAL")
	
	gb_audio_channels		= new tchannel[ gb_audio_max_channels ]
	for local i:int = 0 to gb_audio_max_channels-1
		gb_audio_channels[i] = allocchannel()
	next
endfunction
