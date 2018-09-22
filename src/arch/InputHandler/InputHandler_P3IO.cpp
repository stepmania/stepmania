/*
Holy shit this bs is using libusb0. WTF is that bullshit about. Not only does the API differ from libusb1, the actual data returned differs a bit!!
This is based on my libusb1 program i use to funnel data over UDP to a remote controller.

Some notes:
If using on a real ddr pc, the xp version of zadig has absolute SHIT libusb0 drivers. Libusb1 can use the winusb backend (potential reason for inconsistencies?).
You'll get a reaping error with that version of the driver. Instead, and I have NO idea why this even works, but it does, install the libusbk driver
for your p3io. Yup... somehow the winusb approximation library actually works with this... go figure

Secondly, the ibutton library has some com port shit in it. Half (66% of an HD cabinet) of DDR's lights are controlled over serial.
But SOME of them are controlled over USB. Dear god konami get your act together and commit! I guess they did... everything is serial over usb with modern shit... lmao
Anyway, this ibutton library com port driver does NOT work! Theres notes alluding to that. So, I found this awesome MIT licensed
multiplatform library I threw in arch/COM. Should let people actually throw ddr hardware in a mac or raspberry pi and use it or something haha.

Like hell do I understand cmake or anything like that so I'm keeping my changes self contained. Should be a simple addition to whatever fork that way.
Finally, I do not understand the button system in this at all, so I am throwing keyboard keys out there. Would someone be so kind as to convert this
to a joystick or special DDR input device so I can disable the keyboard buttons entirely so asshats cant press f1 for credits
this depends on io/p3io.ccp/h -- see there for additional details. Sadly libusb based approach is NOT portable to sm5. Someone want to port to windows fileio?

*****IMPORTANT!*****
usb bulk write and read commands are in their own thread to not bog interrupt reading. I do not know the consequences of doing this
libusb says the file descriptors are thread safe but you may not get the notification data arrived? Thing is, are the descriptors device or endpoint specific?
My questions on the ML went unanswered so I leave this as an experiment to test on my lab rats... But it seems to be fine
*/
#include "global.h"
#include "RageLog.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "arch/COM/serial.h"

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

// required I/O routines
#include "LightsManager.h"
#include "InputHandler_P3IO.h"


static bool HDCabinet = false;

#ifdef PRODUCT_ID_BARE
#ifndef STDSTRING_H
#include <array>
#endif
REGISTER_INPUT_HANDLER_CLASS(P3IO);
#else
REGISTER_INPUT_HANDLER(P3IO);
#endif

bool InputHandler_P3IO::s_bInitialized = false;
uint8_t usb_light_message[] = { 0, 0, 0, 0, 0 };

static Preference<PSTRING> m_sCabinetType("P3IOCabinetType", "SD");



//This is the old p3io -> UDP network packet definition... nothing special to see here folks... probably leave this alone...if it aint broke dont fix it!
//model keys after network packet structure:
// Read this as MOST SIGNIFICANT BIT to LEAST SIGNIFICANT BIT per byte
// udlrudlr  -- p1 and p2 pad
// UDLRUDLR  -- p1 and p2 control panel
// stcC12XX  -- service test coin1 coin2 start1 start2 ? ?
#ifdef PRODUCT_ID_BARE
void InputHandler_P3IO::GetDevicesAndDescriptions(std::vector<InputDeviceInfo>& vDevicesOut)
{
	if (m_bFoundUSBDevice)
	{
		vDevicesOut.push_back(InputDeviceInfo(DEVICE_KEYBOARD, "P3IO"));
	}
}

#ifndef STDSTRING_H //rstring doesnt exist...
std::array<DeviceButton, P3IO_NUM_STATES> button_list =
{
#else //  STDSTRING_H
const DeviceButton InputHandler_P3IO::button_list[P3IO_NUM_STATES] =
#endif

#else
void InputHandler_P3IO::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<PSTRING>& vDescriptionsOut)
{
	if (m_bFoundUSBDevice)
	{
		vDevicesOut.push_back(InputDevice(DEVICE_KEYBOARD));
		vDescriptionsOut.push_back("P3IO");
	}
}
const int InputHandler_P3IO::button_list[P3IO_NUM_STATES] =
#endif
{
	//player 1 pad

	KEY_UP,					// DANCE_BUTTON_UP,
	KEY_DOWN,				// DANCE_BUTTON_DOWN,
	KEY_LEFT,				// DANCE_BUTTON_LEFT,
	KEY_RIGHT,				// DANCE_BUTTON_RIGHT,

	//player 2 pad
	KEY_KP_C8,				// DANCE_BUTTON_UP,
	KEY_KP_C2,				// DANCE_BUTTON_DOWN,
	KEY_KP_C4,				// DANCE_BUTTON_LEFT,
	KEY_KP_C6,				// DANCE_BUTTON_RIGHT,

	//player 1 CP
	KEY_HOME,				// DANCE_BUTTON_MENUUP
	KEY_END,				// DANCE_BUTTON_MENUDOWN
	KEY_DEL,				// DANCE_BUTTON_MENULEFT
	KEY_PGDN,				// DANCE_BUTTON_MENURIGHT

	//player 2 CP
	KEY_KP_HYPHEN,			// DANCE_BUTTON_MENUUP
	KEY_KP_PLUS,			// DANCE_BUTTON_MENUDOWN
	KEY_KP_SLASH,			// DANCE_BUTTON_MENULEFT
	KEY_KP_ASTERISK,		// DANCE_BUTTON_MENURIGHT

	//buttons for admin functions
	KEY_F1,					// DANCE_BUTTON_COIN -- mimic service on a ddr machine
	KEY_SCRLLOCK,			// OPERATOR / TEST BUTTON
	KEY_F1,					// DANCE_BUTTON_COIN
	KEY_F1,					// DANCE_BUTTON_COIN

	//p1 and p2 start buttons
	KEY_ENTER,				// DANCE_BUTTON_START p1
	KEY_KP_ENTER,			// DANCE_BUTTON_START p2

#ifdef PRODUCT_ID_BARE
#ifndef STDSTRING_H //stepmania 5 master
}
};
#else //stepmania 5.1-new
};
#endif 
#else//oitg
};

#endif





InputHandler_P3IO::InputHandler_P3IO()
{
	m_bFoundUSBDevice = false;

	LOG->Info("Checking P3IO cabinet type...");

	//LOG->Info("Preference is %s",m_sCabinetType.Get().c_str());

	if (strcasecmp(m_sCabinetType.Get().c_str(), "HD") == 0)
	{
		HDCabinet = true;
		LOG->Info("P3IO Cabinet Type set to HD.");
	}
	else if (strcasecmp(m_sCabinetType.Get().c_str(), "SD") == 0)
	{
		LOG->Info("P3IO Cabinet Type set to SD.");
	}
	else
	{
		// This message is more of a formality than anything.
		// The InputHandler logic is the same for P2IO/P3IO.
		LOG->Info("P3IO Cabinet Type set to P2IO Compatibility Mode");
	}


	for (int i = 0; i<P3IO_NUM_STATES; i++)
	{
		previousStates[i] = 0;
	}

	/* if a handler has already been created (e.g. by ScreenArcadeStart)
	* and it has claimed the board, don't try to claim it again. */

	if (s_bInitialized)
	{
		LOG->Warn("InputHandler_P3IO: Redundant driver loaded. Disabling...");
		return;
	}

	// attempt to open the I/O device
	if (!Board.Open())
	{
		LOG->Warn("InputHandler_P3IO: could not establish a connection with the I/O device.");
		return;
	}

	LOG->Info("Opened P3IO USB board.");


	s_bInitialized = true;
	m_bFoundUSBDevice = true;
	m_bShutdown = false;


	
	USBThread.SetName("P3IO USB Interrupt thread");
	USBThread.Create(USBThread_Start, this);

	USBBulkThread.SetName("P3IO USB Bulk thread");
	
	LOG->Info("detect P3IO bulk thread.");
	//update lights appropriatly for each cabinet type -- this prevents doing a compare operation every loop -- do it once to make the thread
	if (HDCabinet)
	{
		USBBulkThread.Create(USBBulkThreadHD_Start, this);
	}
	else
	{
		USBBulkThread.Create(USBBulkThreadSD_Start, this);
	}

}

InputHandler_P3IO::~InputHandler_P3IO()
{


	m_bShutdown = true;

	if (USBThread.IsCreated())
	{
		LOG->Trace("Shutting down P3IO USB Interrupt thread...");
		USBThread.Wait();
		LOG->Trace("P3IO USB interrupt  thread shut down.");
	}

	if (USBBulkThread.IsCreated())
	{
		LOG->Trace("Shutting down P3IO USB Bulk thread...");
		USBBulkThread.Wait();
		LOG->Trace("P3IO USB Bulk thread shut down.");
	}


	/* Reset all USB lights to off and close it */
	if (m_bFoundUSBDevice)
	{
		memset(usb_light_message, 0, 5 * sizeof(uint8_t));// zero out the message
		Board.writeLights(usb_light_message);
		Board.Close();

		s_bInitialized = false;
	}
}



//TRAMPOLINES FOR SD/HD CABINET BULK THREAD SELECTION
int InputHandler_P3IO::USBBulkThreadHD_Start(void *p)
{
	((InputHandler_P3IO *)p)->USBBulkThreadMainHD();
	return 0;
}
int InputHandler_P3IO::USBBulkThreadSD_Start(void *p)
{
	((InputHandler_P3IO *)p)->USBBulkThreadMainSD();
	return 0;
}
void InputHandler_P3IO::USBBulkThreadMainHD()
{
	red_top_count=0;
	red_bottom_count=0;
	blue_top_count=0;
	blue_bottom_count=0;
	neon_count=0;
	while (!m_bShutdown)
	{
		UpdateLightsUSBHD();
	}
}
void InputHandler_P3IO::USBBulkThreadMainSD()
{
	while (!m_bShutdown)
	{
		UpdateLightsUSBSD();
	}
}
//END TRAMPOLINES FOR SD/HD CABINET BULK THREAD SELECTION



//INTERRUPT THREAD SETUP
int InputHandler_P3IO::USBThread_Start(void *p)
{
	((InputHandler_P3IO *)p)->USBThreadMain();
	return 0;
}
void InputHandler_P3IO::USBThreadMain()
{
	// boost this thread priority past the priority of the binary;
	// if we don't, we might lose input data (e.g. coins) during loads.
#ifdef WIN32
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
	{
		LOG->Warn("Failed to set P3IO thread priority: %d", GetLastError());
	}
		

	/* Enable priority boosting. */
	SetThreadPriorityBoost(GetCurrentThread(), FALSE);
#else
#ifndef PRODUCT_ID_BARE
	HOOKS->BoostThreadPriority();
#endif
#endif


	while (!m_bShutdown)
	{


		// read our input data (and handle I/O errors)
		//LOG->Info( "P3IO::about to process" );
		if (!Board.interruptRead(packetBuffer))
		{
			LOG->Warn("P3IO disconnected? Trying to reconnect");
			Board.Reconnect();
			usleep(100000);
			continue;
		}

		// update the I/O state with the data we've read
		ProcessDataFromDriver();
	}
	//thread will exit and priority wont matter
}
//END INTERRUPT THREAD SETUP


void InputHandler_P3IO::ProcessDataFromDriver()
{
	//LOG->Info( "P3IO::Processing data from driver" );
	int i = 0;
	for (i = 0; i<P3IO_NUM_STATES; i++)
	{
		int byte = i / 8;
		int bit = i % 8;
		if (BYTE_BIT_IS_SET_M_TO_L(packetBuffer[byte], bit) != previousStates[i])
		{
			previousStates[i] = !previousStates[i];
			//LOG->Info( "P3IO input state of button %d changed to %d",i,previousStates[i] );
#ifdef PRODUCT_ID_BARE
#ifndef STDSTRING_H
			ButtonPressed(DeviceInput(DEVICE_KEYBOARD, button_list[i], previousStates[i]));
#else
			ButtonPressed(DeviceInput(DEVICE_KEYBOARD, button_list[i], previousStates[i]));

#endif

#else
			ButtonPressed(DeviceInput(DEVICE_KEYBOARD, button_list[i]), previousStates[i]);
#endif
		}
	}
	InputHandler::UpdateTimer();
}



//BULK LIGHT UPDATES FOR SD AND HD CABINETS
void InputHandler_P3IO::UpdateLightsUSBSD()
{
	//simulate lights not updated here...
	memset(usb_light_message, 0, 5 * sizeof(uint8_t));// zero out the message
	
	//these are dumb, dont bother abstracting
#ifdef PRODUCT_ID_BARE
	LightsState ls = LightsDriver_Export::GetState();
	//i am convinced the actual button lights are controlled over serial on HD cabinets
	if (ls.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) usb_light_message[0] |= USB_OUT_P1_PANEL_LR;  //on hd cabinet buttons left LR is red spotlights satellite bottom
	if (ls.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) usb_light_message[0] |= USB_OUT_P1_PANEL_UD; //on hd cabinet buttons left UD is red spotlights marquee top
	if (ls.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) usb_light_message[0] |= USB_OUT_P2_PANEL_LR;//on hd cabinet buttons right LR is blue spotlights satellite bottom
	if (ls.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) usb_light_message[0] |= USB_OUT_P2_PANEL_UD;//on hd cabinet buttons right UD is blue spotlights marquee top

	//the following do not control the spotlights in HD mode... see above.. other lights are over serial COM2, see extio lights driver
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) usb_light_message[0] |= USB_OUT_MARQUEE_LR; //in hd mode this is different
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) usb_light_message[0] |= USB_OUT_MARQUEE_UR; //in hd mode this is different
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) usb_light_message[0] |= USB_OUT_MARQUEE_LL; //in hd mode this is different
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) usb_light_message[0] |= USB_OUT_MARQUEE_UL; //in hd mode this is different
#else
	
	const LightsState *ls = LightsDriver_External::Get();
	//i am convinced the actual button lights are controlled over serial on HD cabinets
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_LEFT]) usb_light_message[0] |= USB_OUT_P1_PANEL_LR;  //on hd cabinet buttons left LR is red spotlights satellite bottom
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_LEFT]) usb_light_message[0] |= USB_OUT_P1_PANEL_UD; //on hd cabinet buttons left UD is red spotlights marquee top
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_RIGHT]) usb_light_message[0] |= USB_OUT_P2_PANEL_LR;//on hd cabinet buttons right LR is blue spotlights satellite bottom
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_RIGHT]) usb_light_message[0] |= USB_OUT_P2_PANEL_UD;//on hd cabinet buttons right UD is blue spotlights marquee top

	//the following do not control the spotlights in HD mode... see above.. other lights are over serial COM2, see extio lights driver
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) usb_light_message[0] |= USB_OUT_MARQUEE_LR; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) usb_light_message[0] |= USB_OUT_MARQUEE_UR; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) usb_light_message[0] |= USB_OUT_MARQUEE_LL; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) usb_light_message[0] |= USB_OUT_MARQUEE_UL; //in hd mode this is different

#endif
	
	usleep(16666); //hack until I thread this better? CALLING THREAD sleeps for 16.6 ms
	//~60 updates a second max (not needed but curbs talking).
	//to do better, i need to access the same device as input if I make this its own driver, but i cant have a static driver member...
	

	Board.writeLights(usb_light_message); 
}


void InputHandler_P3IO::UpdateLightsUSBHD()
{
	//simulate lights not updated here...
	memset(usb_light_message, 0, 5 * sizeof(uint8_t));// zero out the message
	uint8_t hdxb[]={0,0,0,0,0,0,0,0,0,0,0,0,0};

	bool lr_right = false;
	bool up_right = false;
	bool lr_left = false;
	bool up_left = false;
	bool buttons_left=false;
	bool buttons_right=false;
	bool neons=false;

	//abstract based on SM version
#ifdef PRODUCT_ID_BARE
	LightsState ls = LightsDriver_Export::GetState();
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) lr_right=true; //in hd mode this is different -- see above for why I do this
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) up_right=true; //in hd mode this is different
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) lr_left=true; //in hd mode this is different
	if (ls.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) up_left=true; //in hd mode this is different
	if (ls.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) buttons_left=true;
	if (ls.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) buttons_right=true;
	if (ls.m_bCabinetLights[LIGHT_BASS_LEFT] || ls.m_bCabinetLights[LIGHT_BASS_RIGHT]) neons=true;
#else
	const LightsState *ls = LightsDriver_External::Get();
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) lr_right=true; //in hd mode this is different -- see above for why I do this
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) up_right=true; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) lr_left=true; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) up_left=true; //in hd mode this is different
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_LEFT]) buttons_left=true;
	if (ls->m_bCabinetLights[LIGHT_BUTTONS_RIGHT]) buttons_right=true;
	if (ls->m_bCabinetLights[LIGHT_BASS_LEFT] || ls->m_bCabinetLights[LIGHT_BASS_RIGHT]) neons=true;
#endif

	//decide later of I want to hard reset a channel to zero or fade it out...

	//red bottom lights
	if (lr_right)
	{
		usb_light_message[0] |= USB_OUT_P1_PANEL_LR; //in hd mode this is different -- see above for why I do this
		red_bottom_count+=1;
	}
	else
	{
		if (red_bottom_count>1)
			red_bottom_count-=2;
		else
			red_bottom_count=0;
	}

	//blue bottom lights
	if (up_right)
	{
		usb_light_message[0] |= USB_OUT_P2_PANEL_LR; //in hd mode this is different
		blue_bottom_count+=1;
	}
	else
	{
		if (blue_bottom_count>1)
			blue_bottom_count-=2;
		else
			blue_bottom_count=0;
	}

	//red top lights
	if (lr_left)
	{
			usb_light_message[0] |= USB_OUT_P1_PANEL_UD; //in hd mode this is different
			red_top_count+=1;
	}
	else
	{
		if (red_top_count>1)
			red_top_count-=2;
		else
			red_top_count=0;
	}

	//blue top lights
	if (up_left)
	{
		usb_light_message[0] |= USB_OUT_P2_PANEL_UD; //in hd mode this is differnt
		blue_top_count+=1;
	}
	else
	{
		if (blue_top_count>1)
			blue_top_count-=2;
		else
			blue_top_count=0;
	}

	//neon channel which I use to generate greens
	if (neons)
	{
		neon_count+=4;
	}
	else
	{
		if (neon_count>3)
			neon_count-=4;
		else
			neon_count=0;
	}

	//light up the buttons on the panel
	if (buttons_left){
		hdxb[1] |= 0x80;
		hdxb[1] |= 0x80; //magic wand, make my speaker RED!
		hdxb[2] |= 0x80;
		hdxb[3] |= 0x80;
	}

	if (buttons_right){
		hdxb[4] |= 0x80;
		hdxb[4] |= 0x80; //magic wand, make my speaker RED!
		hdxb[5] |= 0x80;
		hdxb[6] |= 0x80;
	}

	red_top_count=capAt(0x7F,red_top_count);
	red_bottom_count=capAt(0x7F,red_bottom_count);
	blue_top_count=capAt(0x7F,blue_top_count);
	blue_bottom_count=capAt(0x7F,blue_bottom_count);
	neon_count=capAt(0x7F,neon_count);


	//yeah.. not gonna do this yet...
	
	//order of speaker lights is GRB:: P1 Upper, p1 lower, P2 upper, P2 lower
	//actually may be rgb....

	//what is the mysterious byte 0? el byto mysteriouso~~
	hdxb[0]=0;//?
	//p1 up:
	hdxb[1]|=neon_count;//G
	hdxb[2]|=red_top_count;//R
	hdxb[3]|=blue_top_count;//B
	//p2 up:
	hdxb[4]|=neon_count;//G
	hdxb[5]|=red_top_count;//R
	hdxb[6]|=blue_top_count;//B
	//p1 down
	hdxb[7]|=neon_count;//G
	hdxb[8]|=red_bottom_count;//R
	hdxb[9]|=blue_bottom_count;//B
	//p2 down
	hdxb[10]|=neon_count;//G
	hdxb[11]|=red_bottom_count;//R
	hdxb[12]|=blue_bottom_count;//B
	

	usleep(16666); //hack until I thread this better? trying to limit hd lights to 30fps

	//to do better, i need to access the same device as input if I make this its own driver, but i cant have a static driver member...
	Board.writeLights(usb_light_message);

	usleep(16666);
	Board.writeHDXB(hdxb,0xd);
	//now do speakers and button panel

}
//END BULK LIGHT UPDATES FOR SD AND HD CABINETS

uint8_t InputHandler_P3IO::capAt(uint8_t cap,uint8_t var)
{
	if (var > cap) return cap;
	return var;
}
