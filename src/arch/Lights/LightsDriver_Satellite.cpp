/*
The satellites are crazy. They have an MCU in them so a 1 bit change can totally change everything. I haven't mapped every possibility or dumped the MCU to find out how it ticks.
So what can we do with them? Well I CAN do some animations but I didn't include that code here. Feel free to read the research here:
https://github.com/Nadeflore/ACreal_IO/issues/2

*/

#include "global.h"
#include "LightsDriver_Satellite.h"
#include "RageLog.h"
#include "GameState.h"
#include "RageThreads.h"
#include "PrefsManager.h"

#ifdef PRODUCT_ID_BARE
REGISTER_LIGHTS_DRIVER_CLASS(Satellite);
#else
REGISTER_LIGHTS_DRIVER(Satellite);
#endif

static Preference<PSTRING> m_SATELLITE_COMPORT("SatelliteComPort", ""); //COM2 or ttys1

//stepmania constructs
LightsState LightsDriver_Satellite::m_State;
RageMutex LightsDriver_Satellite::m_Lock( "LightsDriver_Satellite");
bool LightsDriver_Satellite::s_bInitialized = false;
bool LightsDriver_Satellite::lightsUpdated = true;
bool LightsDriver_Satellite::m_bShutdown = false;
bool pNeon=false;
int neon_switch_count=0;
int randBase=0;

//acio constructs
serial::Serial LightsDriver_Satellite::satellites;
//static serial::Timeout serial_timeout = serial::Timeout::simpleTimeout(1000);
static serial::Timeout serial_timeout_baud(4294967295,0,10,0,100);
uint8_t LightsDriver_Satellite::acio_request[256];
uint8_t LightsDriver_Satellite::acio_response[256];



//magic numbers for satellite init -- this controls satellites and monitor underlightings not buttons or "neons" it appears.
//"Neons" do turn off for an all off though...
#define NUM_SATELLITE_INIT_PACKETS 24
static uint8_t satellite_init[NUM_SATELLITE_INIT_PACKETS][7]={
//Enumeration reply
// 00    00    01    00    PP    07    CC -- 01 is the command for enumeration, PP is 01 (payload length), 07 is the payload with a reply of 7 in it, CC is checksum
{0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02}, //count nodes / enumerate nodes

//Satellite version string is:
//82 00 02 00 2C 05 01 00 00 00 01 00 02 44 44 52 53 41 75 67 20 32 35 20 32 30 30 38 00 00 00 00 00 31 34 3A 32 38 3A 34 35 00 00 00 00 00 00 00 00 20 
//In ascii that is:
//‚...,........DDRSAug 25 2008.....14:28:45........ 
{0x01, 0x00, 0x02, 0x00, 0x00, 0x03}, //get version of device 1
{0x02, 0x00, 0x02, 0x00, 0x00, 0x04}, //get version of device 2
{0x03, 0x00, 0x02, 0x00, 0x00, 0x05}, //get version of device 3
{0x04, 0x00, 0x02, 0x00, 0x00, 0x06}, //get version of device 4
{0x05, 0x00, 0x02, 0x00, 0x00, 0x07}, //get version of device 5
{0x06, 0x00, 0x02, 0x00, 0x00, 0x08}, //get version of device 6
{0x07, 0x00, 0x02, 0x00, 0x00, 0x09}, //get version of device 7
//Command 3 response:
// XX    00    03    00    00    CC -- XX is device number, CC is checksum
{0x01, 0x00, 0x03, 0x00, 0x00, 0x04}, //send command 3 to device 1
{0x02, 0x00, 0x03, 0x00, 0x00, 0x05}, //send command 3 to device 2
{0x03, 0x00, 0x03, 0x00, 0x00, 0x06}, //send command 3 to device 3
{0x04, 0x00, 0x03, 0x00, 0x00, 0x07}, //send command 3 to device 4
{0x05, 0x00, 0x03, 0x00, 0x00, 0x08}, //send command 3 to device 5
{0x06, 0x00, 0x03, 0x00, 0x00, 0x09}, //send command 3 to device 6
{0x07, 0x00, 0x03, 0x00, 0x00, 0x0A}, //send command 3 to device 7
//Command 0 response:
// XX    01    00    00    PP    00    CC -- XX is the replying device ID (81), if >=0x80 this is a REPLY, subtract 0x80 for the device. 01 is the type?, 00 is the command, the next 00 is a packet id?, PP is payload size of 01. 00 is the payload. CC is the checksum
{0x01, 0x01, 0x00, 0x00, 0x00, 0x02}, //send command 0 with type? 1 to device 1
{0x02, 0x01, 0x00, 0x00, 0x00, 0x03}, //send command 0 with type? 1 to device 2
{0x03, 0x01, 0x00, 0x00, 0x00, 0x04}, //send command 0 with type? 1 to device 3
{0x04, 0x01, 0x00, 0x00, 0x00, 0x05}, //send command 0 with type? 1 to device 4
{0x05, 0x01, 0x00, 0x00, 0x00, 0x06}, //send command 0 with type? 1 to device 5
{0x06, 0x01, 0x00, 0x00, 0x00, 0x07}, //send command 0 with type? 1 to device 6
{0x07, 0x01, 0x00, 0x00, 0x00, 0x08}, //send command 0 with type? 1 to device 7
};

//init color array for COM2 Lights
static uint8_t satellite_colors[5][4][10]=
{ 
	//black
	{ 
		//under
		{
			0x22, 0x56, 0x79, 0xA9, 0xFD, 0xA3, 0x93, 0xAD, 0x84, 0x80
		},
		//inner
		{
			0x22, 0x56, 0x79, 0xA9, 0xFD, 0xA3, 0x93, 0xAD, 0x84, 0x80
		},
		//middle
		{
			0x22, 0x56, 0x79, 0xA9, 0xFD, 0xA3, 0x93, 0xAD, 0x84, 0x80
		},
		//outer
		{
			0x22, 0x56, 0x79, 0xA9, 0xFD, 0xA3, 0x93, 0xAD, 0x84, 0x80
		},
	},
	
	//red
	{ //under
		{
			0x44, 0x68, 0xD0, 0xB9, 0x52, 0x68, 0x11, 0x76, 0x88, 0x00
		},
		//inner
		{
			0x44, 0x68, 0xD0, 0xB9, 0x52, 0x68, 0x10, 0x70, 0x7C, 0xF9
		},
		//middle
		{
			0x44, 0x68, 0xD0, 0xB9, 0x52, 0x68, 0x11, 0x75, 0x84, 0xFD
		},
		//outer
		{
			0x44, 0x68, 0xD0, 0xB9, 0x52, 0x68, 0x11, 0x76, 0x88, 0x00
		},
	},
	
	//green
	{ //under
		{
			0x22, 0x9A, 0x18, 0x2B, 0x6F, 0x85, 0x8F, 0x56, 0x88, 0x00
		},
		//inner
		{
			0x22, 0x9A, 0x18, 0x2B, 0x6F, 0x85, 0x8E, 0x50, 0x7C, 0xF9
		},
		//middle
		{
			0x22, 0x9A, 0x18, 0x2B, 0x6F, 0x85, 0x8F, 0x55, 0x84, 0xFD
		},
		//outer
		{
			0x22, 0x9A, 0x18, 0x2B, 0x6F, 0x85, 0x8F, 0x56, 0x88, 0x00
		},
	},

	//blue
	{ //under
		{
			0x22, 0x57, 0x02, 0xE3, 0xE1, 0xDC, 0x1F, 0x36, 0x88, 0x00
		},
		//inner
		{
			 0x22, 0x57, 0x02, 0xE3, 0xE1, 0xDC, 0x1E, 0x30, 0x7C, 0xF9
		},
		//middle
		{
			0x22, 0x57, 0x02, 0xE3, 0xE1, 0xDC, 0x1F, 0x35, 0x84, 0xFD
		},
		//outer
		{
			0x22, 0x57, 0x02, 0xE3, 0xE1, 0xDC, 0x1F, 0x36, 0x88, 0x00
		},
	},

	//white
	{ //under
		{
			0x44, 0xAD, 0xF4, 0x55, 0xFC, 0x49, 0x29, 0x5B, 0x0A, 0x00
		},
		//inner
		{
			0x44, 0xAD, 0xF4, 0x55, 0xFC, 0x49, 0x28, 0x54, 0xFD, 0xF9
		},
		//middle
		{
			0x44, 0xAD, 0xF4, 0x55, 0xFC, 0x49, 0x29, 0x5A, 0x06, 0xFD
		},
		//outer
		{
			0x44, 0xAD, 0xF4, 0x55, 0xFC, 0x49, 0x29, 0x5B, 0x0A, 0x00
		},
	},
};


LightsDriver_Satellite::LightsDriver_Satellite()
{

	serial::Timeout serial_timeout(10,10,10,10,10);
	if (s_bInitialized) return;
	s_bInitialized = true; //only one instance should do this
	m_bShutdown = false;


	LOG->Info("Satellite: Initializing Satellite Light drivers...");
	LOG->Info("Satellite: Configured Satellite serial port: %s", m_SATELLITE_COMPORT.Get().c_str());


	//check if we have a valid com port name for satellites
	//-- by default disable these, people probably dont have them unless they are *special*
	if (m_SATELLITE_COMPORT.Get().length()>1)
	{
		//this is nice because satellite code will now only run after this SINGLE check
		SatelliteThread.SetName("Satellite thread");
		SatelliteThread.Create(SatelliteThread_Start, this);
	}

}




LightsDriver_Satellite::~LightsDriver_Satellite()
{
	m_bShutdown = true;
	if (SatelliteThread.IsCreated())
	{
		LOG->Info("Satellite: Shutting down Satellite serial thread...");
		SatelliteThread.Wait();
		LOG->Info("Satellite: Satellite serial thread shut down.");
	}
}



void LightsDriver_Satellite::SatelliteThreadMain()
{

	/*
	HEY! Important information here! I REed and looked at com port state after DDR loaded and found out it left the flow control vars in the following states:
	
	dcbSerialParams.fOutxCtsFlow = false;
    dcbSerialParams.fRtsControl = 0x01;
    dcbSerialParams.fOutX = false;
    dcbSerialParams.fInX = false;


	...yes RTS is set, but CTS is not... bizarre but whatever.
	For some reason, unlike DDR, I cannot get the satellites to open the first time I try to. I must forciby terminate either stepmania or
	ddr (which works openning the first time btw...) and THEN try again and it works. I disassembled the code for the com port and I wrote a few
	additions to the library. Theres no exact equivilent to setmask on *nix, so no idea if this even works. The purge function I tried to make work
	like WINE does it. And I added a flowcontrol_ddr setup for the com port setup in windows. In *nux... it looks like you must set CTS as well as RTS
	so who knows if this will work there. Any com port gurus around? What am I doing stupid / wrong here?

	Windows uses createeventw like a semaphore of sorts I think, so I don't need to mimic that here. This thread is the only thread accessing the port.
	*/

	//setup satellites if we should
	LOG->Info("Satellite: SET COM PORT...");
	satellites.setPort(m_SATELLITE_COMPORT.Get().c_str()); //on a ddr pcb, satellites must reside on COM2
	LOG->Info("Satellite: Prepping to open...");
	bool r =true;
	r=satellites.ACIOopen();
	//satellites.open();
	if (r)
	{
		LOG->Info("Satellite: Opened Satellites ok");
	}
	else
	{
		LOG->Info("Satellite: error Openning Satellites");
	}

	/*
	LOG->Info("Satellite: SET MASK...");
	satellites.setMask(1u); //confirmed from output -- windows only :(

	LOG->Info("Satellite: Purge Comms...");
	satellites.purgeComm();

	LOG->Info("Satellite: SET TO...");
	satellites.setTimeout(serial_timeout_baud);

	LOG->Info("Satellite: SET BAUD...");
	satellites.setBaudrate(57600);
	
	//dont need to set this?
	//LOG->Info("Satellite: SET BREAK...");
	//satellites.setBreak(); //bool

	LOG->Info("Satellite: SET BYTE SIZE...");
	satellites.setBytesize(serial::eightbits);  //5,6,7,8
	
	LOG->Info("Satellite: SET FLOW CONTROL...");
	satellites.setFlowcontrol(serial::flowcontrol_ddr);
	LOG->Info("Satellite: SET PARITY...");
	satellites.setParity(serial::parity_none); // typedef enum {
							  //parity_none = 0,
							  //parity_odd = 1,
							  //parity_even = 2,
							  //parity_mark = 3,
							  //parity_space = 4
	
	LOG->Info("Satellite: SET Stop bits...");
	satellites.setStopbits(serial::stopbits_one); //   stopbits_one = 1,
							  // stopbits_two = 2,
							  // stopbits_one_point_five = 3

	LOG->Info("Satellite: SET DTR...");
	satellites.setDTR(true); //bool

	*/
	/*
	usleep(1000000);
	//init satellites
	*/
	//first confirm they are connected by spamming 0xAA for 151 times. This is normally used for baud rate detection

	bool baudConfirmed=ACIO::baudCheckWrapper(satellites);
	if (baudConfirmed==true)
	{
		LOG->Info("Satellite: Baud rate confirmed!");
	}
	else
	{
		LOG->Info("Satellite: Failed to get baud confirmation!");
		//sendSatelliteTest(SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X);
		return;
	}


	//satellites.setTimeout(serial_timeout);

	//We have the proper baud rate, now start sending init packets
	for (int i=0;i<NUM_SATELLITE_INIT_PACKETS;i++)
	{
		
		//int len=assemble_init_packet_to_write(i,true);
		int len = ACIO::assemble_init_packet_to_write(acio_request,satellite_init[i]);
		ACIO::write_acio_packet(satellites,acio_request,len);
		
		//int r = read_acio_packet();
		//read_acio_packet(); //suppress warning
		ACIO::read_acio_packet(satellites,acio_response);

		/* why was I doing this? Was I drunk?
		int response_length = ACIO::read_acio_packet(satellites,acio_response);
		int match=1;

		
		//for (int j=0;j<get_expected_reply_bytes_from_request()-1;j++)
		for (int j=0;j<response_length;j++)
		{
			if (acio_response[j]!=acio_request[j]) match=0;
		}
		if (match==1)
		{
			LOG->Info("Satellite: Init %d MATCH!",i);
		}
		else
		{
			LOG->Info("Satellite: Init %d NO match!",i);
		}
		*/
		usleep(1000);
	}
	


	while (!m_bShutdown)
	{
		UpdateLightsSatellite();
		usleep(8333); //120 times per sec
	}


	//code to turn off satellites
	sendSatelliteSolid(SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X, SATELLITE_X);

}

void LightsDriver_Satellite::UpdateLightsSatellite()
{
	if (!AreLightsUpdated()) return;
	LightsState ls = GetState();

//try to generate SOME sort of pattern for these RGB lights....
	int satellite_left_color[3]={SATELLITE_X,SATELLITE_X,SATELLITE_X};
	int satellite_right_color[3]={SATELLITE_X,SATELLITE_X,SATELLITE_X};
	int monitor_color=SATELLITE_X;
	bool p1_start=false;
	bool p2_start=false;
	bool ul=false;
	bool ur= false;
	bool ll=false;
	bool lr=false;
	bool neon=false;
	bool neon_switch=false;

	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) ul=true;
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) ur=true;
	if (ls.m_bCabinetLights[LIGHT_BASS_LEFT]) neon=true;
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) lr=true;
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) ll=true;
	if (ls.m_bCabinetLights[LIGHT_BASS_RIGHT]) neon=true;

	if (neon!=pNeon)
	{
		neon_switch=true;
		neon_switch_count%=3;
		pNeon=neon;
		randBase=rand() % 3;
	}

	#ifdef PRODUCT_ID_BARE
	if (ls.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) p1_start=true;
	if (ls.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) p2_start=true;
	#else
	if (ls.m_bCabinetLights[LIGHT_BUTTONS_LEFT]) p1_start=true;
	if (ls.m_bCabinetLights[LIGHT_BUTTONS_RIGHT])  p2_start=true;
	#endif




	if (neon)
	{
	
		//left

		if (ul && ll)
		{
			satellite_left_color[0]=SATELLITE_W;
			satellite_left_color[1]=SATELLITE_W;
			satellite_left_color[2]=SATELLITE_W;
		}
		if (ul && !ll)
		{
			satellite_left_color[0]=SATELLITE_R;
			satellite_left_color[1]=SATELLITE_R;
			satellite_left_color[2]=SATELLITE_R;
		}
		if (!ul && !ll)
		{
			satellite_left_color[0]=SATELLITE_B;
			satellite_left_color[1]=SATELLITE_B;
			satellite_left_color[2]=SATELLITE_B;
		}
		if (!ul && ll)
		{
			satellite_left_color[0]=SATELLITE_G;
			satellite_left_color[1]=SATELLITE_G;
			satellite_left_color[2]=SATELLITE_G;
		}



		//right
		if (ur && lr)
		{
			satellite_right_color[0]=SATELLITE_W;
			satellite_right_color[1]=SATELLITE_W;
			satellite_right_color[2]=SATELLITE_W;
		}
		if (ur && !lr)
		{
			satellite_right_color[0]=SATELLITE_R;
			satellite_right_color[1]=SATELLITE_R;
			satellite_right_color[2]=SATELLITE_R;
		}
		if (!ur && !lr)
		{
			satellite_right_color[0]=SATELLITE_B;
			satellite_right_color[1]=SATELLITE_B;
			satellite_right_color[2]=SATELLITE_B;
		}
		if (!ur && lr)
		{
			satellite_right_color[0]=SATELLITE_G;
			satellite_right_color[1]=SATELLITE_G;
			satellite_right_color[2]=SATELLITE_G;
		}


	}
	else
	{
		//left
		if (ul && ll)
		{
			satellite_left_color[0]=((neon_switch_count+0+randBase)%3)+1;
			satellite_left_color[1]=((neon_switch_count+1+randBase)%3)+1;
			satellite_left_color[2]=((neon_switch_count+2+randBase)%3)+1;
		}
		if (ul && !ll)
		{
			satellite_left_color[0]=((neon_switch_count+1+randBase)%3)+1;
			satellite_left_color[1]=((neon_switch_count+2+randBase)%3)+1;
			satellite_left_color[2]=((neon_switch_count+3+randBase)%3)+1;
		}
		/*if (!ul && !ll)
		{
			satellite_left_color[0]=((neon_switch_count+2+randBase)%3)+1;
			satellite_left_color[1]=((neon_switch_count+3+randBase)%3)+1;
			satellite_left_color[2]=((neon_switch_count+4+randBase)%3)+1;
		}*/
		if (!ul && ll)
		{
			satellite_left_color[0]=((neon_switch_count+2+randBase)%3)+1;
			satellite_left_color[1]=((neon_switch_count+3+randBase)%3)+1;
			satellite_left_color[2]=((neon_switch_count+4+randBase)%3)+1;
		}


		//right


		if (ur && lr)
		{
			satellite_right_color[0]=((neon_switch_count+0+randBase)%3)+1;
			satellite_right_color[1]=((neon_switch_count+1+randBase)%3)+1;
			satellite_right_color[2]=((neon_switch_count+2+randBase)%3)+1;
		}
		if (ur && !lr)
		{
			satellite_right_color[0]=((neon_switch_count+1+randBase)%3)+1;
			satellite_right_color[1]=((neon_switch_count+2+randBase)%3)+1;
			satellite_right_color[2]=((neon_switch_count+3+randBase)%3)+1;
		}
		/*
		if (!ur && !lr)
		{
			satellite_right_color[0]=((neon_switch_count+2+randBase)%3)+1;
			satellite_right_color[1]=((neon_switch_count+3+randBase)%3)+1;
			satellite_right_color[2]=((neon_switch_count+4+randBase)%3)+1;
		}
		*/
		if (!ur && lr)
		{
			satellite_right_color[0]=((neon_switch_count+2+randBase)%3)+1;
			satellite_right_color[1]=((neon_switch_count+3+randBase)%3)+1;
			satellite_right_color[2]=((neon_switch_count+4+randBase)%3)+1;
		}
	}

	
	if(p1_start)
	{
		satellite_left_color[0]=SATELLITE_R;
		satellite_left_color[1]=SATELLITE_R;
		satellite_left_color[2]=SATELLITE_R;
		monitor_color=SATELLITE_R;

		if(!p2_start)
		{
			satellite_right_color[0]=SATELLITE_G;
			satellite_right_color[1]=SATELLITE_G;
			satellite_right_color[2]=SATELLITE_G;
		}
	}

	if(p2_start)
	{
		satellite_right_color[0]=SATELLITE_B;
		satellite_right_color[1]=SATELLITE_B;
		satellite_right_color[2]=SATELLITE_B;
		monitor_color=SATELLITE_B;

		if(!p1_start)
		{
			satellite_left_color[0]=SATELLITE_G;
			satellite_left_color[1]=SATELLITE_G;
			satellite_left_color[2]=SATELLITE_G;
		}
	}

	if(p1_start&&p2_start)
	{
		monitor_color=SATELLITE_G;
	}


	//TODO:
	//ideas for the satellites -- probably not happened, requires a LOT of program specific querying. People like the more fun patterns anyway
	//PlayerState* p1;
	//int player2_index=GAMESTATE->GetNumHumanPlayers();//returns int of number of players, if 1 player, duplicate lifebar on satellite
	//if .... GAMESTATE->IsPlayerHot(PLAYER_1)//100% GREEN? for awesome
	//        GAMESTATE->IsPlayerDead//0% // for nothing
	//        GAMESTATE->IsPlayerInDanger//100% red for dying
	//else    //100% BLUE for ok...

	sendSatelliteSolid(monitor_color, satellite_left_color[0], satellite_left_color[1], satellite_left_color[2], satellite_right_color[0], satellite_right_color[1], satellite_right_color[2]);
}

void LightsDriver_Satellite::sendSatelliteTest(int u, int sli, int slm, int slo, int sri, int srm, int sro)
{
	acio_request[0]=0x70;//broadcast
	acio_request[1]=0x46;//payload size

	uint8_t test[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	//payload -- color array is satellite_colors[COLOR][LOCATION_FOR_COLOR][0], 0 represents data
	memcpy( &acio_request[2],  test, 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[12], &satellite_colors[sli][SATELLITE_I][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[22], &satellite_colors[slm][SATELLITE_M][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[32], &satellite_colors[slo][SATELLITE_O][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[42], &satellite_colors[sri][SATELLITE_I][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[52], &satellite_colors[srm][SATELLITE_M][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[62], &satellite_colors[sro][SATELLITE_O][0], 10 * sizeof( uint8_t ) );
	
	//checksum
	acio_request[72]=0;

	//EOF -- not nessecary but why not
	acio_request[73]=0;

	//write_acio_packet(assemble_init_packet_to_write(0,false));
	//ACIO::write_acio_packet(satellites,acio_request,assemble_init_packet_to_write(0,false));
	ACIO::write_acio_packet(satellites,acio_request, ACIO::assemble_init_packet_to_write(acio_request,NULL)   );
}

void LightsDriver_Satellite::sendSatelliteSolid(int u, int sli, int slm, int slo, int sri, int srm, int sro)
{
	acio_request[0]=0x70;//broadcast
	acio_request[1]=0x46;//payload size

	//payload -- color array is satellite_colors[COLOR][LOCATION_FOR_COLOR][0], 0 represents data
	memcpy( &acio_request[2],  &satellite_colors[ u ][SATELLITE_U][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[12], &satellite_colors[sli][SATELLITE_I][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[22], &satellite_colors[slm][SATELLITE_M][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[32], &satellite_colors[slo][SATELLITE_O][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[42], &satellite_colors[sri][SATELLITE_I][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[52], &satellite_colors[srm][SATELLITE_M][0], 10 * sizeof( uint8_t ) );
	memcpy( &acio_request[62], &satellite_colors[sro][SATELLITE_O][0], 10 * sizeof( uint8_t ) );
	
	//checksum
	acio_request[72]=0;

	//EOF -- not nessecary but why not
	acio_request[73]=0;

	//write_acio_packet(assemble_init_packet_to_write(0,false));
	//ACIO::write_acio_packet(satellites,acio_request,assemble_init_packet_to_write(0,false));
	ACIO::write_acio_packet(satellites,acio_request, ACIO::assemble_init_packet_to_write(acio_request,NULL)   );
}


//No need to look at these, they are trampolines and stubs
int LightsDriver_Satellite::SatelliteThread_Start(void *p)
{
	((LightsDriver_Satellite *)p)->SatelliteThreadMain();
	return 0;
}



void LightsDriver_Satellite::Set(const LightsState *ls)
{
	m_Lock.Lock();
	lightsUpdated=true;
	m_State = *ls;
	m_Lock.Unlock();
}

bool LightsDriver_Satellite::AreLightsUpdated()
{
	bool ret;
	m_Lock.Lock();
	ret=lightsUpdated;
	m_Lock.Unlock();
	return ret;
}

LightsState LightsDriver_Satellite::GetState()
{
	m_Lock.Lock();
	lightsUpdated=false;
	LightsState ret( m_State );
	m_Lock.Unlock();
	return ret;
}