
'''''''''''
'' const ''
'''''''''''

const gb_net_socket_port 			:short 	= 53350
const gb_net_master_server		:string = "mr-goober.com"

''''''''''''
'' global ''
''''''''''''


global gb_net_socket :tsocket
global gb_net_stream :tstream

''''''''''''
'' events ''
''''''''''''

function gb_net_init()
	gb_net_socket = createudpsocket()
	bindsocket( gb_net_socket, gb_net_socket_port )
	gb_net_stream = createsocketstream( gb_net_socket, false )
endfunction