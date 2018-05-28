#include "global.h"
#include "InputHandler_Linux_Event.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "LinuxInputManager.h"
#include "GamePreferences.h" //needed for Axis Fix

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

REGISTER_INPUT_HANDLER_CLASS2( LinuxEvent, Linux_Event );


static DeviceButton LinuxKeyCodeToDeviceButton( int key )
{
#define MAP(a,b) case a: return b;
	switch( key )
	{
	MAP(KEY_ESC,DB_KEY_ESC)
	MAP(KEY_MINUS,DB_KEY_HYPHEN)
	MAP(KEY_EQUAL,DB_KEY_EQUAL)
	MAP(KEY_BACKSPACE,DB_KEY_BACK)
	MAP(KEY_TAB,DB_KEY_TAB)
	MAP(KEY_LEFTBRACE,DB_KEY_LBRACE)
	MAP(KEY_RIGHTBRACE,DB_KEY_RBRACE)
	MAP(KEY_ENTER,DB_KEY_ENTER)
	MAP(KEY_LEFTCTRL,DB_KEY_LCTRL)
	MAP(KEY_RIGHTCTRL,DB_KEY_RCTRL)
	MAP(KEY_SEMICOLON,DB_KEY_SEMICOLON)
	MAP(KEY_APOSTROPHE,DB_KEY_SQUOTE)
	MAP(KEY_GRAVE,DB_KEY_ACCENT)
	MAP(KEY_LEFTSHIFT,DB_KEY_LSHIFT)
	MAP(KEY_BACKSLASH,DB_KEY_BACKSLASH)
	MAP(KEY_COMMA,DB_KEY_COMMA)
	MAP(KEY_DOT,DB_KEY_PERIOD)
	MAP(KEY_SLASH,DB_KEY_SLASH)
	MAP(KEY_RIGHTSHIFT,DB_KEY_RSHIFT)
	MAP(KEY_KPASTERISK,DB_KEY_KP_ASTERISK)
	MAP(KEY_LEFTALT,DB_KEY_LALT)
	MAP(KEY_RIGHTALT,DB_KEY_RALT)
	MAP(KEY_SPACE,DB_KEY_SPACE)
	MAP(KEY_CAPSLOCK,DB_KEY_CAPSLOCK)
	MAP(KEY_NUMLOCK,DB_KEY_NUMLOCK)
	MAP(KEY_SCROLLLOCK,DB_KEY_SCRLLOCK)
	MAP(KEY_KPMINUS,DB_KEY_KP_HYPHEN)
	MAP(KEY_KPPLUS,DB_KEY_KP_PLUS)
	MAP(KEY_KPDOT,DB_KEY_KP_PERIOD)
	MAP(KEY_KPENTER,DB_KEY_KP_ENTER)
	MAP(KEY_KPSLASH,DB_KEY_KP_SLASH)
	MAP(KEY_HOME,DB_KEY_HOME)
	MAP(KEY_UP,DB_KEY_UP)
	MAP(KEY_PAGEUP,DB_KEY_PGUP)
	MAP(KEY_LEFT,DB_KEY_LEFT)
	MAP(KEY_RIGHT,DB_KEY_RIGHT)
	MAP(KEY_END,DB_KEY_END)
	MAP(KEY_DOWN,DB_KEY_DOWN)
	MAP(KEY_PAGEDOWN,DB_KEY_PGDN)
	MAP(KEY_INSERT,DB_KEY_INSERT)
	MAP(KEY_DELETE,DB_KEY_DEL)
	MAP(KEY_KPEQUAL,DB_KEY_KP_EQUAL)
	MAP(KEY_PAUSE,DB_KEY_PAUSE)
	MAP(KEY_LEFTMETA,DB_KEY_LMETA)
	MAP(KEY_RIGHTMETA,DB_KEY_RMETA)
	MAP(KEY_MENU,DB_KEY_MENU)
	MAP(KEY_FN,DB_KEY_FN)
	MAP(KEY_DOLLAR,DB_KEY_DOLLAR)
	MAP(KEY_QUESTION,DB_KEY_QUESTION)
	MAP(KEY_F1,DB_KEY_F1)
	MAP(KEY_F2,DB_KEY_F2)
	MAP(KEY_F3,DB_KEY_F3)
	MAP(KEY_F4,DB_KEY_F4)
	MAP(KEY_F5,DB_KEY_F5)
	MAP(KEY_F6,DB_KEY_F6)
	MAP(KEY_F7,DB_KEY_F7)
	MAP(KEY_F8,DB_KEY_F8)
	MAP(KEY_F9,DB_KEY_F9)
	MAP(KEY_F10,DB_KEY_F10)
	MAP(KEY_F11,DB_KEY_F11)
	MAP(KEY_F12,DB_KEY_F12)
	MAP(KEY_F13,DB_KEY_F13)
	MAP(KEY_F14,DB_KEY_F14)
	MAP(KEY_F15,DB_KEY_F15)
	MAP(KEY_F16,DB_KEY_F16)
	MAP(KEY_1,DB_KEY_C1)
	MAP(KEY_2,DB_KEY_C2)
	MAP(KEY_3,DB_KEY_C3)
	MAP(KEY_4,DB_KEY_C4)
	MAP(KEY_5,DB_KEY_C5)
	MAP(KEY_6,DB_KEY_C6)
	MAP(KEY_7,DB_KEY_C7)
	MAP(KEY_8,DB_KEY_C8)
	MAP(KEY_9,DB_KEY_C9)
	MAP(KEY_0,DB_KEY_C0)
	MAP(KEY_A,DB_KEY_CA)
	MAP(KEY_B,DB_KEY_CB)
	MAP(KEY_C,DB_KEY_CC)
	MAP(KEY_D,DB_KEY_CD)
	MAP(KEY_E,DB_KEY_CE)
	MAP(KEY_F,DB_KEY_CF)
	MAP(KEY_G,DB_KEY_CG)
	MAP(KEY_H,DB_KEY_CH)
	MAP(KEY_I,DB_KEY_CI)
	MAP(KEY_J,DB_KEY_CJ)
	MAP(KEY_K,DB_KEY_CK)
	MAP(KEY_L,DB_KEY_CL)
	MAP(KEY_M,DB_KEY_CM)
	MAP(KEY_N,DB_KEY_CN)
	MAP(KEY_O,DB_KEY_CO)
	MAP(KEY_P,DB_KEY_CP)
	MAP(KEY_Q,DB_KEY_CQ)
	MAP(KEY_R,DB_KEY_CR)
	MAP(KEY_S,DB_KEY_CS)
	MAP(KEY_T,DB_KEY_CT)
	MAP(KEY_U,DB_KEY_CU)
	MAP(KEY_V,DB_KEY_CV)
	MAP(KEY_W,DB_KEY_CW)
	MAP(KEY_X,DB_KEY_CX)
	MAP(KEY_Y,DB_KEY_CY)
	MAP(KEY_Z,DB_KEY_CZ)
	MAP(KEY_KP0,DB_KEY_KP_C0)
	MAP(KEY_KP1,DB_KEY_KP_C1)
	MAP(KEY_KP2,DB_KEY_KP_C2)
	MAP(KEY_KP3,DB_KEY_KP_C3)
	MAP(KEY_KP4,DB_KEY_KP_C4)
	MAP(KEY_KP5,DB_KEY_KP_C5)
	MAP(KEY_KP6,DB_KEY_KP_C6)
	MAP(KEY_KP7,DB_KEY_KP_C7)
	MAP(KEY_KP8,DB_KEY_KP_C8)
	MAP(KEY_KP9,DB_KEY_KP_C9)

	/*
	 * No corresponding key in linux-event-codes.h
	 * DB_KEY_EXCL
	 * DB_KEY_QUOTE
	 * DB_KEY_HASH
	 * DB_KEY_PERCENT
	 * DB_KEY_AMPER
	 * DB_KEY_LPAREN
	 * DB_KEY_RPAREN
	 * DB_KEY_ASTERISK
	 * DB_KEY_PLUS
	 * DB_KEY_COLON
	 * DB_KEY_LANGLE
	 * DB_KEY_RANGLE
	 * DB_KEY_AT
	 * DB_KEY_LBRACKET
	 * DB_KEY_RBRACKET
	 * DB_KEY_CARAT
	 * DB_KEY_UNDERSCORE
	 * DB_KEY_PIPE
	 * DB_KEY_PRTSC
	 */
	default: return DeviceButton_Invalid;
	}
	return DeviceButton_Invalid;
}

static RString BustypeToString( int iBus )
{
	switch( iBus )
	{
//	case BUS_ADB:
//	case BUS_AMIGA: return "amiga input";
	case BUS_BLUETOOTH: return "Bluetooth";
	case BUS_GAMEPORT: return "gameport";
//	case BUS_HIL:
//	case BUS_HOST:
//	case BUS_I2C:
	case BUS_I8042: return "keyboard";
	case BUS_ISA: return "ISA";
	case BUS_ISAPNP: return "ISAPNP";
	case BUS_PARPORT: return "parallel port";
	case BUS_PCI: return "PCI";
	case BUS_RS232: return "serial port";
	case BUS_USB: return "USB";
	case BUS_XTKBD: return "XT keyboard";
	default: return ssprintf("unknown bus %x", iBus);
	}
}

struct EventDevice
{
	EventDevice();
	~EventDevice();
	bool Open( RString sFile, InputDevice dev );
	bool IsOpen() const { return m_iFD != -1; }
	void Close()
	{
		if( m_iFD != -1 )
			close( m_iFD );
		m_iFD = -1;
	}

	int m_iFD;
	RString m_sPath;
	RString m_sName;
	InputDevice m_Dev;

	int aiAbsMin[ABS_MAX];
	int aiAbsMax[ABS_MAX];
	DeviceButton aiAbsMappingHigh[ABS_MAX];
	DeviceButton aiAbsMappingLow[ABS_MAX];
};

static vector<EventDevice *> g_apEventDevices;

/* Return true if the numbered event device exists.  sysfs may not always be
 * there; return false if we don't know. */
static bool EventDeviceExists( int iNum )
{
	RString sDir = ssprintf( "/sys/class" );
	struct stat st;
	if( stat(sDir, &st) == -1 )
		return true;

	RString sFile = ssprintf( "/sys/class/input/event%i", iNum );
	return stat(sFile, &st) == 0;
}

static bool BitIsSet( const uint8_t *pArray, uint32_t iBit )
{
	return !!(pArray[iBit/8] & (1<<(iBit%8)));
}

EventDevice::EventDevice()
{
	m_iFD = -1;
}

bool EventDevice::Open( RString sFile, InputDevice dev )
{
	m_sPath = sFile;
	m_Dev = dev;
	m_iFD = open( sFile, O_RDWR );
	if( m_iFD == -1 )
	{
		// HACK: Let the caller handle errno.
		return false;
	}

	static bool bLogged = false;
	if( !bLogged )
	{
		bLogged = true;
		int iVersion;
		if( ioctl(m_iFD, EVIOCGVERSION, &iVersion) == -1 )
			LOG->Warn( "ioctl(EVIOCGVERSION): %s", strerror(errno) );
		else
			LOG->Info( "Event driver: v%i.%i.%i", (iVersion >> 16) & 0xFF, (iVersion >> 8) & 0xFF, iVersion & 0xFF ); 
	}

	char szName[1024];
	if( ioctl(m_iFD, EVIOCGNAME(sizeof(szName)), szName) == -1 )
	{
		LOG->Warn( "ioctl(EVIOCGNAME): %s", strerror(errno) );
		
		m_sName = "(unknown)";
	}
	else
	{
		m_sName = szName;
	}

	input_id DevInfo;
	if( ioctl(m_iFD, EVIOCGID, &DevInfo) == -1 )
	{
		LOG->Warn( "ioctl(EVIOCGID): %s", strerror(errno) );
	}
	else
	{
		LOG->Info( "Input device: %s: %s device, ID %04x:%04x, version %x: %s", sFile.c_str(),
			BustypeToString(DevInfo.bustype).c_str(), DevInfo.vendor, DevInfo.product,
			DevInfo.version, m_sName.c_str() );
	}

	uint8_t iABSMask[ABS_MAX/8 + 1];
	memset( iABSMask, 0, sizeof(iABSMask) );
	if( ioctl(m_iFD, EVIOCGBIT(EV_ABS, sizeof(iABSMask)), iABSMask) < 0 )
		LOG->Warn( "ioctl(EVIOCGBIT(EV_ABS)): %s", strerror(errno) );

	if( BitIsSet(iABSMask, ABS_X) || BitIsSet(iABSMask, ABS_THROTTLE) || BitIsSet(iABSMask, ABS_WHEEL) )
	{
		LOG->Info( "    Mapping %s as a joystick", m_sName.c_str() );
	}
	else if( DevInfo.bustype == BUS_USB ||
	         DevInfo.bustype == BUS_XTKBD )
	{
		// TODO: Is there any way to actually check for a keybard?
		// Anything on BUS_XTKBD should be a keyboard
		// Anything on BUS_USB which provides input, and isn't an analogue device may be a keyboard
		// Anything on BUS_BLUETOOTH could be a keyboard. These are ignored for now.
		LOG->Info( "    Mapping %s as a keyboard", m_sName.c_str() );
		m_Dev = DEVICE_KEYBOARD;
	}

	uint8_t iKeyMask[KEY_MAX/8 + 1];
	memset( iKeyMask, 0, sizeof(iKeyMask) );
	if( ioctl(m_iFD, EVIOCGBIT(EV_KEY, sizeof(iKeyMask)), iKeyMask) < 0 )
		LOG->Warn( "ioctl(EVIOCGBIT(EV_KEY)): %s", strerror(errno) );

	uint8_t iEventTypes[EV_MAX/8];
	memset( iEventTypes, 0, sizeof(iEventTypes) );
	if( ioctl(m_iFD, EVIOCGBIT(0, EV_MAX), iEventTypes) == -1 )
		LOG->Warn( "ioctl(EV_MAX): %s", strerror(errno) );

	{
		vector<RString> setEventTypes;

		if( BitIsSet(iEventTypes, EV_SYN) )		setEventTypes.push_back( "syn" );
		if( BitIsSet(iEventTypes, EV_KEY) )		setEventTypes.push_back( "key" );
		if( BitIsSet(iEventTypes, EV_REL) )		setEventTypes.push_back( "rel" );
		if( BitIsSet(iEventTypes, EV_ABS) )		setEventTypes.push_back( "abs" );
		if( BitIsSet(iEventTypes, EV_MSC) )		setEventTypes.push_back( "misc" );
		if( BitIsSet(iEventTypes, EV_SW) )		setEventTypes.push_back( "sw" );
		if( BitIsSet(iEventTypes, EV_LED) )		setEventTypes.push_back( "led" );
		if( BitIsSet(iEventTypes, EV_SND) )		setEventTypes.push_back( "snd" );
		if( BitIsSet(iEventTypes, EV_REP) )		setEventTypes.push_back( "rep" );
		if( BitIsSet(iEventTypes, EV_FF) )		setEventTypes.push_back( "ff" );
		if( BitIsSet(iEventTypes, EV_PWR) )		setEventTypes.push_back( "pwr" );
		if( BitIsSet(iEventTypes, EV_FF_STATUS) )	setEventTypes.push_back( "ff_status" );

		LOG->Info( "    Event types: %s", join(", ", setEventTypes).c_str() );
	}

	int iTotalKeys = 0;
	for( int i = 0; i < KEY_MAX; ++i )
	{
		if( !BitIsSet(iKeyMask, i) )
			continue;
		++iTotalKeys;
	}

	int iTotalAxes = 0;
	const DeviceButton iExtraAxes[] = { JOY_LEFT_2, JOY_UP_2, JOY_AUX_1, JOY_AUX_3 };
	int iNextExtraAxis = 0;
	for( int i = 0; i < ABS_MAX; ++i )
	{
		if( !BitIsSet(iABSMask, i) )
			continue;
		struct input_absinfo absinfo;
		if( ioctl(m_iFD, EVIOCGABS(i), &absinfo) < 0 )
		{
			LOG->Warn( "ioctl(EVIOCGABS): %s", strerror(errno) );
			continue;
		}

		//LOG->Info( "    Axis %i: min: %i; max: %i; fuzz: %i; flat: %i",
		//		i, absinfo.minimum, absinfo.maximum, absinfo.fuzz, absinfo.flat );
		aiAbsMin[i] = absinfo.minimum;
		aiAbsMax[i] = absinfo.maximum;
		aiAbsMappingHigh[i] = enum_add2(JOY_RIGHT, 2*i);
		aiAbsMappingLow[i] = enum_add2(JOY_LEFT, 2*i);

		if( i == ABS_X )
		{
			aiAbsMappingHigh[i] = JOY_RIGHT;
			aiAbsMappingLow[i] = JOY_LEFT;
		}
		else if( i == ABS_Y )
		{
			aiAbsMappingHigh[i] = JOY_DOWN;
			aiAbsMappingLow[i] = JOY_UP;
		}
		else if( i == ABS_Z )
		{
			aiAbsMappingHigh[i] = JOY_Z_DOWN;
			aiAbsMappingLow[i] = JOY_Z_UP;
		}
		else if( i == ABS_RX )
		{
			aiAbsMappingHigh[i] = JOY_ROT_RIGHT;
			aiAbsMappingLow[i] = JOY_ROT_LEFT;
		}
		else if( i == ABS_RY )
		{
			aiAbsMappingHigh[i] = JOY_ROT_DOWN;
			aiAbsMappingLow[i] = JOY_ROT_UP;
		}
		else if( i == ABS_RZ )
		{
			aiAbsMappingHigh[i] = JOY_ROT_Z_DOWN;
			aiAbsMappingLow[i] = JOY_ROT_Z_UP;
		}
		else if( i == ABS_HAT0X )
		{
			aiAbsMappingHigh[i] = JOY_HAT_RIGHT;
			aiAbsMappingLow[i] = JOY_HAT_LEFT;
		}
		else if( i == ABS_HAT0Y )
		{
			aiAbsMappingHigh[i] = JOY_HAT_UP;
			aiAbsMappingLow[i] = JOY_HAT_DOWN;
		}
		else
		{
			if( iNextExtraAxis < (int) ARRAYLEN(iExtraAxes) )
			{
				aiAbsMappingLow[i] = iExtraAxes[iNextExtraAxis];
				aiAbsMappingHigh[i] = enum_add2( aiAbsMappingLow[i], 1 );
				++iNextExtraAxis;
			}
		}

		++iTotalAxes;
	}
	LOG->Info( "    Total keys: %i; total axes: %i", iTotalKeys, iTotalAxes );

	return true;
}

EventDevice::~EventDevice()
{
	Close();
}

InputHandler_Linux_Event::InputHandler_Linux_Event()
	: m_bShutdown(true)
	, m_bDevicesChanged(false)
	, m_bHaveKeyboard(false)
	, m_NextDevice(DEVICE_JOY10)
{
	if(LINUXINPUT == NULL) LINUXINPUT = new LinuxInputManager;
	LINUXINPUT->InitDriver(this);

	if( ! g_apEventDevices.empty() ) // LinuxInputManager found at least one valid device for us
		StartThread();
}
	
InputHandler_Linux_Event::~InputHandler_Linux_Event()
{
	if( m_InputThread.IsCreated() ) StopThread( true );

	for( int i = 0; i < (int) g_apEventDevices.size(); ++i )
		delete g_apEventDevices[i];
	g_apEventDevices.clear();
}

void InputHandler_Linux_Event::StartThread()
{
	if( m_bShutdown == false ) return;
	m_bShutdown = false;
	
	if( m_InputThread.IsCreated() == false )
	{
		m_InputThread.SetName( "Event input thread" );
		m_InputThread.Create( InputThread_Start, this );
	}
	else
	{
		m_InputThread.Resume();
	}
}

void InputHandler_Linux_Event::StopThread( bool shutdown )
{
	if( m_bShutdown == true ) return;
	if( shutdown )
	{
		m_bShutdown = true;
		LOG->Trace( "Shutting down joystick thread ..." );
		m_InputThread.Wait();
		LOG->Trace( "Joystick thread shut down." );
	}
	else if( m_InputThread.IsCreated() )
	{
		m_InputThread.Halt();
	}
}

bool InputHandler_Linux_Event::TryDevice(RString devfile)
{
	EventDevice* pDev = new EventDevice;
	if( pDev->Open(devfile, m_NextDevice) )
	{
		bool hotplug = false;
		if( m_InputThread.IsCreated() ) { StopThread(); hotplug = true; }
		/* Thread is stopped! DO NOT RETURN */
		{
			g_apEventDevices.push_back( pDev );
		}
		if( hotplug ) StartThread();

		// Checked by InputHandler_X11
		if( pDev->m_Dev == DEVICE_KEYBOARD ) m_bHaveKeyboard = true;

		m_NextDevice = enum_add2(m_NextDevice, 1);
		m_bDevicesChanged = true;
		return true;
	}
	else
	{
		delete pDev;
		// This is likely to fail; most systems still forbid ALL eventNN regardless
		// of their type. Info it anyway, it could be useful for end-user
		// troubleshooting.
		LOG->Info("LinuxEvent: Couldn't open %s: %s.", devfile.c_str(), strerror(errno) );
		return false;
	}
}

int InputHandler_Linux_Event::InputThread_Start( void *p )
{
	((InputHandler_Linux_Event *) p)->InputThread();
	return 0;
}

void InputHandler_Linux_Event::InputThread()
{
	while( !m_bShutdown )
	{
		fd_set fdset;
		FD_ZERO( &fdset );
		int iMaxFD = -1;

		for( int i = 0; i < (int) g_apEventDevices.size(); ++i )
		{
			int iFD = g_apEventDevices[i]->m_iFD;
			if( !g_apEventDevices[i]->IsOpen() )
				continue;

			FD_SET( iFD, &fdset );
			iMaxFD = max( iMaxFD, iFD );
		}

		if( iMaxFD == -1 )
			break;

		struct timeval zero = {0,100000};
		if( select(iMaxFD+1, &fdset, NULL, NULL, &zero) <= 0 )
			continue;
		RageTimer now;

		for( int i = 0; i < (int) g_apEventDevices.size(); ++i )
		{
			if( !g_apEventDevices[i]->IsOpen() )
				continue;

			if( !FD_ISSET(g_apEventDevices[i]->m_iFD, &fdset) )
				continue;

			input_event event;
			int ret = read( g_apEventDevices[i]->m_iFD, &event, sizeof(event) );
			if( ret == -1 )
			{
				LOG->Warn( "Error reading from %s: %s; disabled", g_apEventDevices[i]->m_sPath.c_str(), strerror(errno) );
				g_apEventDevices[i]->Close();
				continue;
			}

			if( ret != sizeof(event) )
			{
				LOG->Warn("Unexpected packet (size %i != %i) from joystick %i; disabled", ret, (int)sizeof(event), i);
				g_apEventDevices[i]->Close();
				continue;
			}

			switch (event.type) {
			case EV_KEY: {
				int iNum;
				if (event.code >= BTN_JOYSTICK && event.code <= BTN_JOYSTICK + 0xf) {
					// These guys have arbitrary names, but the kernel code in hid-input.c maps exactly 0xf of them.
					iNum = event.code - BTN_JOYSTICK;
				} else if (event.code >= BTN_TRIGGER_HAPPY1 && event.code <= BTN_TRIGGER_HAPPY40) {
					// Actually, we only have 32 buttons defined.
					iNum = event.code - BTN_TRIGGER_HAPPY1 + 0x10;
				} else {
					// If the button number is >40+0xf, it gets mapped to a code with no #define.
					// I don't know if this is appropriate at all, but what else to do?
					iNum = event.code;
				}
				if( g_apEventDevices[i]->m_Dev == DEVICE_KEYBOARD )
				{
					ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, LinuxKeyCodeToDeviceButton(iNum), event.value != 0, now) );
				}
				else
				{
					wrap( iNum, 32 );	// max number of joystick buttons.  Make this a constant?
					ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, enum_add2(JOY_BUTTON_1, iNum), event.value != 0, now) );
				}
				break;
			}
				
			case EV_ABS: {
				ASSERT_M( event.code < ABS_MAX, ssprintf("%i", event.code) );
				DeviceButton neg = g_apEventDevices[i]->aiAbsMappingLow[event.code];
				DeviceButton pos = g_apEventDevices[i]->aiAbsMappingHigh[event.code];

				float l = SCALE( int(event.value), (float) g_apEventDevices[i]->aiAbsMin[event.code], (float) g_apEventDevices[i]->aiAbsMax[event.code], -1.0f, 1.0f );
				if (GamePreferences::m_AxisFix)
				{
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, neg, (l < -0.5)||((l > 0.0001)&&(l < 0.5)), now) ); //Up if between 0.0001 and 0.5 or if less than -0.5
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, pos, (l > 0.5)||((l > 0.0001)&&(l < 0.5)) , now) ); //Down if between 0.0001 and 0.5 or if more than 0.5
				}
				else
				{
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, neg, max(-l,0), now) );
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, pos, max(+l,0), now) );
				}
				break;
			}
			}

		}

	}

	InputHandler::UpdateTimer();
}

void InputHandler_Linux_Event::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	for( unsigned i = 0; i < g_apEventDevices.size(); ++i )
	{
		EventDevice *pDev = g_apEventDevices[i];
                vDevicesOut.push_back( InputDeviceInfo(pDev->m_Dev, pDev->m_sName) );
	}
	
	m_bDevicesChanged = false;
}

bool InputHandler_Linux_Event::HasKeyboard() const
{
	return m_bHaveKeyboard;
}

/*
 * (c) 2003-2008 Glenn Maynard
 * (c) 2013 Ben "root" Anderson
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
