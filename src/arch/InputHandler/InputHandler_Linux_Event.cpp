#include "global.h"
#include "InputHandler_Linux_Event.h"
#include "RageMath.hpp"
#include <array>

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

using std::vector;

REGISTER_INPUT_HANDLER_CLASS2( LinuxEvent, Linux_Event );

// Maps Joystick/Gamepad buttons from Linux HID to DeviceButton enum.
static DeviceButton HidKeyToButton(decltype(input_event::code) hid_key) {
	switch( hid_key ) {
#ifdef BTN_JOYSTICK
	case BTN_TRIGGER: return JOY_HAT_UP;
	case BTN_THUMB: return JOY_BUTTON_11;
	case BTN_THUMB2: return JOY_BUTTON_12;
	case BTN_TOP: return JOY_BUTTON_13;
	case BTN_TOP2: return JOY_BUTTON_14;
	case BTN_PINKIE: return JOY_Z_DOWN;
	case BTN_BASE: return JOY_BUTTON_16;
	case BTN_BASE2: return JOY_BUTTON_17;
	case BTN_BASE3: return JOY_BUTTON_18;
	case BTN_BASE4: return JOY_BUTTON_19;
	case BTN_BASE5: return JOY_BUTTON_20;
	case BTN_BASE6: return JOY_BUTTON_32;
#endif
#ifdef BTN_DPAD_UP
	case BTN_DPAD_UP: return JOY_UP;
	case BTN_DPAD_DOWN: return JOY_DOWN;
	case BTN_DPAD_LEFT: return JOY_LEFT;
	case BTN_DPAD_RIGHT: return JOY_RIGHT;
#endif
#ifdef BTN_GAMEPAD
	case BTN_SOUTH: return JOY_DOWN_2;
	case BTN_EAST: return JOY_RIGHT_2;
	case BTN_C: return JOY_AUX_4;
	case BTN_NORTH: return JOY_UP_2;
	case BTN_WEST: return JOY_LEFT_2;
	case BTN_Z: return JOY_Z_UP;
	case BTN_TL: return JOY_ROT_LEFT;
	case BTN_TR: return JOY_ROT_RIGHT;
	case BTN_TL2: return JOY_ROT_UP;
	case BTN_TR2: return JOY_ROT_DOWN;
	case BTN_SELECT: return JOY_AUX_1;
	case BTN_START: return JOY_AUX_2;
	case BTN_MODE: return JOY_AUX_3;
	case BTN_THUMBL: return JOY_HAT_LEFT;
	case BTN_THUMBR: return JOY_HAT_RIGHT;
#endif
#ifdef BTN_WHEEL
	case BTN_GEAR_DOWN: return JOY_ROT_Z_DOWN;
	case BTN_GEAR_UP: return JOY_ROT_Z_UP;
#endif
#ifdef BTN_0
	case BTN_0: return JOY_HAT_DOWN;
	case BTN_1: return JOY_BUTTON_1;
	case BTN_2: return JOY_BUTTON_2;
	case BTN_3: return JOY_BUTTON_3;
	case BTN_4: return JOY_BUTTON_4;
	case BTN_5: return JOY_BUTTON_5;
	case BTN_6: return JOY_BUTTON_6;
	case BTN_7: return JOY_BUTTON_7;
	case BTN_8: return JOY_BUTTON_8;
	case BTN_9: return JOY_BUTTON_9;
#endif
#ifdef BTN_TRIGGER_HAPPY
	case BTN_TRIGGER_HAPPY1: return JOY_BUTTON_21;
	case BTN_TRIGGER_HAPPY2: return JOY_BUTTON_22;
	case BTN_TRIGGER_HAPPY3: return JOY_BUTTON_23;
	case BTN_TRIGGER_HAPPY4: return JOY_BUTTON_24;
	case BTN_TRIGGER_HAPPY5: return JOY_BUTTON_25;
	case BTN_TRIGGER_HAPPY6: return JOY_BUTTON_26;
	case BTN_TRIGGER_HAPPY7: return JOY_BUTTON_27;
	case BTN_TRIGGER_HAPPY8: return JOY_BUTTON_28;
	case BTN_TRIGGER_HAPPY9: return JOY_BUTTON_29;
	case BTN_TRIGGER_HAPPY10: return JOY_BUTTON_30;
	case BTN_TRIGGER_HAPPY11: return JOY_BUTTON_31;
	case BTN_TRIGGER_HAPPY12: return JOY_BUTTON_32;
	// NOTE: Everything above this point has a unique
	// mapping. Everything below this point will conflict with
	// something above, so try to map with an likely unused button
	// value.
	case BTN_TRIGGER_HAPPY13: return JOY_BUTTON_13;
	case BTN_TRIGGER_HAPPY14: return JOY_BUTTON_14;
	case BTN_TRIGGER_HAPPY15: return JOY_BUTTON_15;
	case BTN_TRIGGER_HAPPY16: return JOY_BUTTON_16;
	case BTN_TRIGGER_HAPPY17: return JOY_BUTTON_17;
	case BTN_TRIGGER_HAPPY18: return JOY_BUTTON_18;
	case BTN_TRIGGER_HAPPY19: return JOY_BUTTON_19;
	case BTN_TRIGGER_HAPPY20: return JOY_BUTTON_20;
	case BTN_TRIGGER_HAPPY21: return JOY_BUTTON_1;
	case BTN_TRIGGER_HAPPY22: return JOY_BUTTON_2;
	case BTN_TRIGGER_HAPPY23: return JOY_BUTTON_3;
	case BTN_TRIGGER_HAPPY24: return JOY_BUTTON_4;
	case BTN_TRIGGER_HAPPY25: return JOY_BUTTON_5;
	case BTN_TRIGGER_HAPPY26: return JOY_BUTTON_6;
	case BTN_TRIGGER_HAPPY27: return JOY_BUTTON_7;
	case BTN_TRIGGER_HAPPY28: return JOY_BUTTON_8;
	case BTN_TRIGGER_HAPPY29: return JOY_BUTTON_9;
	case BTN_TRIGGER_HAPPY30: return JOY_ROT_UP;
	case BTN_TRIGGER_HAPPY31: return JOY_ROT_DOWN;
	case BTN_TRIGGER_HAPPY32: return JOY_ROT_LEFT;
	case BTN_TRIGGER_HAPPY33: return JOY_ROT_RIGHT;
	case BTN_TRIGGER_HAPPY34: return JOY_ROT_Z_UP;
	case BTN_TRIGGER_HAPPY35: return JOY_ROT_Z_DOWN;
	case BTN_TRIGGER_HAPPY36: return JOY_HAT_LEFT;
	case BTN_TRIGGER_HAPPY37: return JOY_HAT_RIGHT;
	case BTN_TRIGGER_HAPPY38: return JOY_HAT_UP;
	case BTN_TRIGGER_HAPPY39: return JOY_HAT_DOWN;
	case BTN_TRIGGER_HAPPY40: return JOY_AUX_4;
#endif
	default: return DeviceButton_Invalid;
	}
}

static std::string BustypeToString( int iBus )
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
	default: return fmt::sprintf("unknown bus %x", iBus);
	}
}

struct EventDevice
{
	EventDevice();
	~EventDevice();
	bool Open( std::string sFile, InputDevice dev );
	bool IsOpen() const { return m_iFD != -1; }
	void Close()
	{
		if( m_iFD != -1 )
			close( m_iFD );
		m_iFD = -1;
	}

	int m_iFD;
	std::string m_sPath;
	std::string m_sName;
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
	auto sDir = fmt::sprintf( "/sys/class" );
	struct stat st;
	if( stat(sDir.c_str(), &st) == -1 )
		return true;

	auto sFile = fmt::sprintf( "/sys/class/input/event%i", iNum );
	return stat(sFile.c_str(), &st) == 0;
}

static bool BitIsSet( const uint8_t *pArray, uint32_t iBit )
{
	return !!(pArray[iBit/8] & (1<<(iBit%8)));
}

EventDevice::EventDevice()
{
	m_iFD = -1;
}

bool EventDevice::Open( std::string sFile, InputDevice dev )
{
	m_sPath = sFile;
	m_Dev = dev;
	m_iFD = open( sFile.c_str(), O_RDWR );
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

	uint8_t iKeyMask[KEY_MAX/8 + 1];
	memset( iKeyMask, 0, sizeof(iKeyMask) );
	if( ioctl(m_iFD, EVIOCGBIT(EV_KEY, sizeof(iKeyMask)), iKeyMask) < 0 )
		LOG->Warn( "ioctl(EVIOCGBIT(EV_KEY)): %s", strerror(errno) );

	if( !BitIsSet(iABSMask, ABS_X) && !BitIsSet(iABSMask, ABS_THROTTLE) && !BitIsSet(iABSMask, ABS_WHEEL) )
	{
		// Okay, this input device doesn't seem to be an
		// analog joystick, nor is it even trying to fake
		// it. However, maybe it's a gamepad that only
		// provides buttons (e.g., Cobalt Flux USB input)?
		bool found_gamepad_button = false;
		for( int i = 0; i <= KEY_MAX; ++i ) {
			if( BitIsSet(iKeyMask, i) && HidKeyToButton(i) != DeviceButton_Invalid ) {
				found_gamepad_button = true;
				break;
			}
		}
		if( !found_gamepad_button ) {
			LOG->Info( "    Not a joystick nor a gamepad; ignored" );
			Close();
			return false;
		}
		LOG->Info( "    No analog inputs, but has gamepad buttons; ok" );
	}

	uint8_t iEventTypes[EV_MAX/8];
	memset( iEventTypes, 0, sizeof(iEventTypes) );
	if( ioctl(m_iFD, EVIOCGBIT(0, EV_MAX), iEventTypes) == -1 )
		LOG->Warn( "ioctl(EV_MAX): %s", strerror(errno) );

	{
		vector<std::string> setEventTypes;

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

		LOG->Info( "    Event types: %s", Rage::join(", ", setEventTypes).c_str() );
	}

	int iTotalKeys = 0;
	for( int i = 0; i <= KEY_MAX; ++i )
	{
		if( !BitIsSet(iKeyMask, i) )
			continue;
		++iTotalKeys;
	}

	int iTotalAxes = 0;
	std::array<DeviceButton, 4> const iExtraAxes = {
		JOY_LEFT_2,
		JOY_UP_2,
		JOY_AUX_1,
		JOY_AUX_3
	};
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
			if( iNextExtraAxis < iExtraAxes.size() )
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
{
	m_NextDevice = DEVICE_JOY10;
	m_bDevicesChanged = false;

	if(LINUXINPUT == nullptr) LINUXINPUT = new LinuxInputManager;
	LINUXINPUT->InitDriver(this);

	if( ! g_apEventDevices.empty() ) // LinuxInputManager found at least one valid device for us
		StartThread();
}

InputHandler_Linux_Event::~InputHandler_Linux_Event()
{
	if( m_InputThread.IsCreated() ) StopThread();

	for( int i = 0; i < (int) g_apEventDevices.size(); ++i )
		delete g_apEventDevices[i];
	g_apEventDevices.clear();
}

void InputHandler_Linux_Event::StartThread()
{
	m_bShutdown = false;
	m_InputThread.SetName( "Event input thread" );
	m_InputThread.Create( InputThread_Start, this );
}

void InputHandler_Linux_Event::StopThread()
{
	m_bShutdown = true;
	LOG->Trace( "Shutting down joystick thread ..." );
	m_InputThread.Wait();
	LOG->Trace( "Joystick thread shut down." );
}

bool InputHandler_Linux_Event::TryDevice(std::string devfile)
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
	using std::max;
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
		if( select(iMaxFD+1, &fdset, nullptr, nullptr, &zero) <= 0 )
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
				const DeviceButton button = HidKeyToButton( event.code );
				if( button != DeviceButton_Invalid ) {
					ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, button, event.value != 0, now) );
				}
				break;
			}

			case EV_ABS: {
				ASSERT_M( event.code < ABS_MAX, fmt::sprintf("%i", event.code) );
				DeviceButton neg = g_apEventDevices[i]->aiAbsMappingLow[event.code];
				DeviceButton pos = g_apEventDevices[i]->aiAbsMappingHigh[event.code];

				float l = Rage::scale( static_cast<float>(event.value), static_cast<float>(g_apEventDevices[i]->aiAbsMin[event.code]), static_cast<float>(g_apEventDevices[i]->aiAbsMax[event.code]), -1.0f, 1.0f );
				if (GamePreferences::m_AxisFix)
				{
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, neg, (l < -0.5)||((l > 0.0001)&&(l < 0.5)), now) ); //Up if between 0.0001 and 0.5 or if less than -0.5
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, pos, (l > 0.5)||((l > 0.0001)&&(l < 0.5)) , now) ); //Down if between 0.0001 and 0.5 or if more than 0.5
				}
				else
				{
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, neg, max(-l,0.f), now) );
				  ButtonPressed( DeviceInput(g_apEventDevices[i]->m_Dev, pos, max(+l,0.f), now) );
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
