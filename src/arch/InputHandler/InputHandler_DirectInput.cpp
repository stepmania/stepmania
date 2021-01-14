#include "global.h"
#include "InputHandler_DirectInput.h"

#include "RageUtil.h"
#include "RageLog.h"
#include "archutils/Win32/AppInstance.h"
#include "archutils/Win32/DirectXHelpers.h"
#include "archutils/Win32/ErrorStrings.h"
#include "archutils/Win32/GraphicsWindow.h"
#include "archutils/Win32/RegistryAccess.h"
#include "InputFilter.h"
#include "PrefsManager.h"
#include "GamePreferences.h" //needed for Axis Fix

#include "InputHandler_DirectInputHelper.h"

#ifdef NTDDI_WIN8 // Link to Xinput9_1_0.lib on Windows 8 SDK and above to ensure linkage to Xinput9_1_0.dll
#pragma comment(lib, "Xinput9_1_0.lib")
#else
#pragma comment(lib, "xinput.lib")
#endif

#include <XInput.h>
#include <WbemIdl.h>
#include <OleAuto.h>

// this may not be defined if we are using an older Windows SDK. (for instance, toolsetversion v140_xp does not define it)
// the number was taken from the documentation
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

REGISTER_INPUT_HANDLER_CLASS2( DirectInput, DInput );

static vector<DIDevice> Devices;
static vector<XIDevice> XDevices;

// Number of joysticks found:
static int g_iNumJoysticks;

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
static BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput)
{
	IWbemLocator*           pIWbemLocator = nullptr;
	IEnumWbemClassObject*   pEnumDevices = nullptr;
	IWbemClassObject*       pDevices[20] = { 0 };
	IWbemServices*          pIWbemServices = nullptr;
	BSTR                    bstrNamespace = nullptr;
	BSTR                    bstrDeviceID = nullptr;
	BSTR                    bstrClassName = nullptr;
	DWORD                   uReturned = 0;
	bool                    bIsXinputDevice = false;
	UINT                    iDevice = 0;
	VARIANT                 var;
	HRESULT                 hr;

	// CoInit if needed
	hr = CoInitialize(nullptr);
	bool bCleanupCOM = SUCCEEDED(hr);

	// Create WMI
	hr = CoCreateInstance(__uuidof(WbemLocator),
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWbemLocator),
		(LPVOID*)&pIWbemLocator);
	if (FAILED(hr) || pIWbemLocator == nullptr)
		goto LCleanup;

	bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == nullptr) goto LCleanup;
	bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (bstrClassName == nullptr) goto LCleanup;
	bstrDeviceID = SysAllocString(L"DeviceID");          if (bstrDeviceID == nullptr)  goto LCleanup;

	// Connect to WMI 
	hr = pIWbemLocator->ConnectServer(bstrNamespace, nullptr, nullptr, 0L,
		0L, nullptr, nullptr, &pIWbemServices);
	if (FAILED(hr) || pIWbemServices == nullptr)
		goto LCleanup;

	// Switch security level to IMPERSONATE. 
	CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

	hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, nullptr, &pEnumDevices);
	if (FAILED(hr) || pEnumDevices == nullptr)
		goto LCleanup;

	// Loop over all devices
	for (;; )
	{
		// Get 20 at a time
		hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
		if (FAILED(hr))
			goto LCleanup;
		if (uReturned == 0)
			break;

		for (iDevice = 0; iDevice<uReturned; iDevice++)
		{
			// For each device, get its device ID
			hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, nullptr, nullptr);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != nullptr)
			{
				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
				// This information can not be found from DirectInput 
				if (wcsstr(var.bstrVal, L"IG_"))
				{
					// If it does, then get the VID/PID from var.bstrVal
					DWORD dwPid = 0, dwVid = 0;
					WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
					if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
						dwVid = 0;
					WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
					if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
						dwPid = 0;

					// Compare the VID/PID to the DInput device
					DWORD dwVidPid = MAKELONG(dwVid, dwPid);
					if (dwVidPid == pGuidProductFromDirectInput->Data1)
					{
						bIsXinputDevice = true;
						goto LCleanup;
					}
				}
			}
			SAFE_RELEASE(pDevices[iDevice]);
		}
	}

LCleanup:
	if (bstrNamespace)
		SysFreeString(bstrNamespace);
	if (bstrDeviceID)
		SysFreeString(bstrDeviceID);
	if (bstrClassName)
		SysFreeString(bstrClassName);
	for (iDevice = 0; iDevice<20; iDevice++)
		SAFE_RELEASE(pDevices[iDevice]);
	SAFE_RELEASE(pEnumDevices);
	SAFE_RELEASE(pIWbemLocator);
	SAFE_RELEASE(pIWbemServices);

	if (bCleanupCOM)
		CoUninitialize();

	return bIsXinputDevice;
}

static BOOL CALLBACK EnumDevicesCallback( const DIDEVICEINSTANCE *pdidInstance, void *pContext )
{
	DIDevice device;

	LOG->Info( "DInput: Enumerating device - Type: 0x%08X Instance Name: \"%s\" Product Name: \"%s\"", pdidInstance->dwDevType, pdidInstance->tszInstanceName, pdidInstance->tszProductName );

	switch( GET_DIDEVICE_TYPE(pdidInstance->dwDevType) )
	{
		case DI8DEVTYPE_JOYSTICK:
		case DI8DEVTYPE_GAMEPAD:
		case DI8DEVTYPE_DRIVING:
		case DI8DEVTYPE_FLIGHT:
		case DI8DEVTYPE_1STPERSON:
		case DI8DEVTYPE_DEVICECTRL:
		case DI8DEVTYPE_SCREENPOINTER:
		case DI8DEVTYPE_REMOTE:
		case DI8DEVTYPE_SUPPLEMENTAL:
		{
			device.type = device.JOYSTICK;
			break;
		}

		case DI8DEVTYPE_KEYBOARD: device.type = device.KEYBOARD; break;
		case DI8DEVTYPE_MOUSE: device.type = device.MOUSE; break;
		default: LOG->Info( "DInput: Unrecognized device ignored." ); return DIENUM_CONTINUE;
	}

	device.JoystickInst = *pdidInstance;

	switch( device.type )
	{
	case DIDevice::JOYSTICK:
		if( g_iNumJoysticks == NUM_JOYSTICKS )
			return DIENUM_CONTINUE;

		if( IsXInputDevice( &pdidInstance->guidProduct ) )
			return DIENUM_CONTINUE;

		device.dev = enum_add2( DEVICE_JOY1, g_iNumJoysticks );
		g_iNumJoysticks++;
		break;

	case DIDevice::KEYBOARD:
		device.dev = DEVICE_KEYBOARD;
		break;

	case DIDevice::MOUSE:
		device.dev = DEVICE_MOUSE;
		break;
	}

	Devices.push_back(device);

	return DIENUM_CONTINUE;
}

static void CheckForDirectInputDebugMode()
{
	int iVal;
	if( RegistryAccess::GetRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\DirectInput", "emulation", iVal) )
	{
		if( iVal & 0x8 )
			LOG->Warn("DirectInput keyboard debug mode appears to be enabled. This reduces\n"
					  "input timing accuracy significantly. Disabling this is strongly recommended." );
	}
}

static BOOL CALLBACK CountDevicesCallback( const DIDEVICEINSTANCE *pdidInstance, void *pContext )
{
	(*(int*)pContext)++;
	return DIENUM_CONTINUE;
}

static int GetNumHidDevices()
{
	int i = 0;
	RegistryAccess::GetRegValue( "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\HidUsb\\Enum", "Count", i, false );	// don't warn on error
	return i;
}

static int GetNumJoysticksSlow()
{
	int iCount = 0;
	HRESULT hr = g_dinput->EnumDevices( DI8DEVCLASS_GAMECTRL, CountDevicesCallback, &iCount, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
	{
		LOG->Warn( hr_ssprintf(hr, "g_dinput->EnumDevices") );
	}
	return iCount;
}

InputHandler_DInput::InputHandler_DInput()
{
	LOG->Trace( "InputHandler_DInput::InputHandler_DInput()" );

	CheckForDirectInputDebugMode();

	m_bShutdown = false;
	g_iNumJoysticks = 0;

	// find xinput joysticks first
	for( DWORD i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		XINPUT_STATE state;
		ZeroMemory( &state, sizeof(XINPUT_STATE) );

		if (XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			XIDevice xdevice;
			xdevice.m_sName = ssprintf("XInput Device %u", i + 1);
			xdevice.dev = enum_add2( InputDevice::DEVICE_JOY1, g_iNumJoysticks );
			xdevice.m_dwXInputSlot = i;
			g_iNumJoysticks++;

			XDevices.push_back(xdevice);
		}
	}
	LOG->Info( "Found %u XInput devices.", XDevices.size() );

	AppInstance inst;
	HRESULT hr = DirectInput8Create(inst.Get(), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *) &g_dinput, nullptr);
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: DirectInputCreate") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_KEYBOARD)" );
	hr = g_dinput->EnumDevices( DI8DEVCLASS_KEYBOARD, EnumDevicesCallback, nullptr, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_JOYSTICK)" );
	hr = g_dinput->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, nullptr, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	// mouse
	LOG->Trace( "InputHandler_DInput: IDirectInput::EnumDevices(DIDEVTYPE_MOUSE)" );
	hr = g_dinput->EnumDevices( DI8DEVCLASS_POINTER, EnumDevicesCallback, nullptr, DIEDFL_ATTACHEDONLY );
	if( hr != DI_OK )
		RageException::Throw( hr_ssprintf(hr, "InputHandler_DInput: IDirectInput::EnumDevices") );

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].Open() )
			continue;

		Devices.erase( Devices.begin() + i );
		i--;
		continue;
	}

	LOG->Info( "Found %u DirectInput devices:", Devices.size() );
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		LOG->Info( "   %d: '%s' axes: %d, hats: %d, buttons: %d (%s)",
			i,
			Devices[i].m_sName.c_str(),
			Devices[i].axes,
			Devices[i].hats,
			Devices[i].buttons,
			Devices[i].buffered? "buffered": "unbuffered" );
	}

	m_iLastSeenNumHidDevices = GetNumHidDevices();
	m_iNumTimesLeftToPollForJoysticksChanged = 0;
	m_iLastSeenNumJoysticks = GetNumJoysticksSlow();

	StartThread();
}

void InputHandler_DInput::StartThread()
{
	ASSERT( !m_InputThread.IsCreated() );
	if( PREFSMAN->m_bThreadedInput )
	{
		m_InputThread.SetName( "DirectInput thread" );
		m_InputThread.Create( InputThread_Start, this );
	}
}

void InputHandler_DInput::ShutdownThread()
{
	m_bShutdown = true;
	if( m_InputThread.IsCreated() )
	{
		LOG->Trace( "Shutting down DirectInput thread ..." );
		m_InputThread.Wait();
		LOG->Trace( "DirectInput thread shut down." );
	}
	m_bShutdown = false;
}

InputHandler_DInput::~InputHandler_DInput()
{
	XDevices.clear();

	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
		Devices[i].Close();

	Devices.clear();
	g_dinput->Release();
	g_dinput = nullptr;
}

void InputHandler_DInput::WindowReset()
{
	// We need to reopen keyboards.
	ShutdownThread();

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].type != Devices[i].KEYBOARD )
			continue;

		Devices[i].Close();

		// We lose buffered inputs here, so we need to clear all pressed keys.
		INPUTFILTER->ResetDevice( Devices[i].dev );

		bool ret = Devices[i].Open();

		// Reopening it should succeed.
		ASSERT( ret );
	}

	StartThread();
}

#define HAT_UP_MASK 1
#define HAT_DOWN_MASK 2
#define HAT_LEFT_MASK 4
#define HAT_RIGHT_MASK 8

static int TranslatePOV(DWORD value)
{
	const int HAT_VALS[] =
	{
		HAT_UP_MASK,
		HAT_UP_MASK   | HAT_RIGHT_MASK,
		HAT_RIGHT_MASK,
		HAT_DOWN_MASK | HAT_RIGHT_MASK,
		HAT_DOWN_MASK,
		HAT_DOWN_MASK | HAT_LEFT_MASK,
		HAT_LEFT_MASK,
		HAT_UP_MASK   | HAT_LEFT_MASK
	};

	if( LOWORD(value) == 0xFFFF )
		return 0;

	// Round the value up:
	value += 4500 / 2;
	value %= 36000;
	value /= 4500;

	if( value >= 8 )
		return 0; // shouldn't happen

	return HAT_VALS[value];
}

static HRESULT GetDeviceState( LPDIRECTINPUTDEVICE8 dev, int size, void *ptr )
{
	HRESULT hr = dev->GetDeviceState( size, ptr );
	if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
	{
		hr = dev->Acquire();
		if( hr != DI_OK )
		{
			LOG->Trace( hr_ssprintf(hr, "?") );
			return hr;
		}

		hr = dev->GetDeviceState( size, ptr );
	}

	return hr;
}

/* This doesn't take a timestamp; instead, we let InputHandler::ButtonPressed
 * figure it out. Be sure to call InputHandler::Update() between each poll. */
void InputHandler_DInput::UpdatePolled( DIDevice &device, const RageTimer &tm )
{
	switch( device.type )
	{
	default:
		FAIL_M(ssprintf("Unsupported DI device type: %i", device.type));
	case DIDevice::KEYBOARD:
		{
			unsigned char keys[256];

			HRESULT hr = GetDeviceState( device.Device, 256, keys );
			if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
				return;

			if( hr != DI_OK )
			{
				LOG->MapLog( "UpdatePolled", hr_ssprintf(hr, "Failures on polled keyboard update") );
				return;
			}

			for( int k = 0; k < 256; ++k )
			{
				const DeviceButton key = (DeviceButton) device.Inputs[k].num;
				ButtonPressed( DeviceInput(device.dev, key, !!(keys[k] & 0x80) ) );
			}
		}
		break;
	case DIDevice::JOYSTICK:
		{
			DIJOYSTATE state;

			HRESULT hr = GetDeviceState(device.Device, sizeof(state), &state);
			if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
				return;

			// Set each known axis, button and POV.
			for( unsigned i = 0; i < device.Inputs.size(); ++i )
			{
				const input_t &in = device.Inputs[i];
				const InputDevice dev = device.dev;

				switch(in.type)
				{
					case input_t::BUTTON:
					{
						DeviceInput di( dev, enum_add2(JOY_BUTTON_1, in.num), !!state.rgbButtons[in.ofs - DIJOFS_BUTTON0], tm );
						ButtonPressed( di );
						break;
					}

					case input_t::AXIS:
					{
						DeviceButton neg = DeviceButton_Invalid, pos = DeviceButton_Invalid;
						int val = 0;
						if( in.ofs == DIJOFS_X )
							{ neg = JOY_LEFT; pos = JOY_RIGHT; val = state.lX; }
						else if( in.ofs == DIJOFS_Y )
							{ neg = JOY_UP; pos = JOY_DOWN; val = state.lY; }
						else if( in.ofs == DIJOFS_Z )
							{ neg = JOY_Z_UP; pos = JOY_Z_DOWN; val = state.lZ; }
						else if( in.ofs == DIJOFS_RX )
							{ neg = JOY_ROT_LEFT; pos = JOY_ROT_RIGHT; val = state.lRx; }
						else if( in.ofs == DIJOFS_RY )
							{ neg = JOY_ROT_UP; pos = JOY_ROT_DOWN;	val = state.lRy; }
						else if( in.ofs == DIJOFS_RZ )
							{ neg = JOY_ROT_Z_UP; pos = JOY_ROT_Z_DOWN; val = state.lRz; }
						else if( in.ofs == DIJOFS_SLIDER(0) )
							{ neg = JOY_AUX_1; pos = JOY_AUX_2;	val = state.rglSlider[0]; }
						else if( in.ofs == DIJOFS_SLIDER(1) )
							{ neg = JOY_AUX_3; pos = JOY_AUX_4;	val = state.rglSlider[1]; }
						else LOG->MapLog( "unknown input", 
											"Controller '%s' is returning an unknown joystick offset, %i",
											device.m_sName.c_str(), in.ofs );

						if( neg != DeviceButton_Invalid )
						{
							float l = SCALE( int(val), 0.0f, 100.0f, 0.0f, 1.0f );
							ButtonPressed( DeviceInput(dev, neg, max(-l,0), tm) );
							ButtonPressed( DeviceInput(dev, pos, max(+l,0), tm) );
						}

						break;
					}

					case input_t::HAT:
						if( in.num == 0 )
						{
							const int pos = TranslatePOV( state.rgdwPOV[in.ofs - DIJOFS_POV(0)] );
							ButtonPressed( DeviceInput(dev, JOY_HAT_UP, !!(pos & HAT_UP_MASK), tm) );
							ButtonPressed( DeviceInput(dev, JOY_HAT_DOWN, !!(pos & HAT_DOWN_MASK), tm) );
							ButtonPressed( DeviceInput(dev, JOY_HAT_LEFT, !!(pos & HAT_LEFT_MASK), tm) );
							ButtonPressed( DeviceInput(dev, JOY_HAT_RIGHT, !!(pos & HAT_RIGHT_MASK), tm) );
						}

						break;
				}
			}
		}
		break;
	case DIDevice::MOUSE:
		// Not 100% perfect (or tested much) yet. -aj
		DIMOUSESTATE state;

		HRESULT hr = GetDeviceState(device.Device, sizeof(state), &state);
		if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
			return;

		// reset mousewheel
		ButtonPressed( DeviceInput(device.dev, MOUSE_WHEELUP, 0, tm) );
		ButtonPressed( DeviceInput(device.dev, MOUSE_WHEELDOWN, 0, tm) );

		for( unsigned i = 0; i < device.Inputs.size(); ++i )
		{
			const input_t &in = device.Inputs[i];
			const InputDevice dev = device.dev;

			switch(in.type)
			{
				case input_t::BUTTON:
				{
					DeviceInput di( dev, enum_add2(MOUSE_LEFT, in.num), !!state.rgbButtons[in.ofs - DIMOFS_BUTTON0], tm );
					ButtonPressed( di );
					break;
				}
				case input_t::AXIS:
				{
					DeviceButton neg = DeviceButton_Invalid, pos = DeviceButton_Invalid;
					int val = 0;
					if( in.ofs == DIMOFS_X )
						{ neg = MOUSE_X_LEFT; pos = MOUSE_X_RIGHT; val = state.lX; }
					else if( in.ofs == DIMOFS_Y )
						{ neg = MOUSE_Y_UP; pos = MOUSE_Y_DOWN; val = state.lY; }
					else if( in.ofs == DIMOFS_Z )
					{
						neg = MOUSE_WHEELDOWN; pos = MOUSE_WHEELUP;
						val = state.lZ;
						//LOG->Trace("MouseWheel polled: %i",val);
						INPUTFILTER->UpdateMouseWheel(val);
						if( val == 0 )
						{
							// release all
							ButtonPressed( DeviceInput(dev, pos, 0, tm) );
							ButtonPressed( DeviceInput(dev, neg, 0, tm) );
						}
						else if( int(val) > 0 )
						{
							// positive values: WheelUp
							ButtonPressed( DeviceInput(dev, pos, 1, tm) );
							ButtonPressed( DeviceInput(dev, neg, 0, tm) );
						}
						else if( int(val) < 0 )
						{
							// negative values: WheelDown
							ButtonPressed( DeviceInput(dev, neg, 1, tm) );
							ButtonPressed( DeviceInput(dev, pos, 0, tm) );
						}
					}
					else LOG->MapLog( "unknown input", 
											"Mouse '%s' is returning an unknown mouse offset, %i",
											device.m_sName.c_str(), in.ofs );
					break;
				}
			}
		}
		break;
	}
}

void InputHandler_DInput::UpdateBuffered( DIDevice &device, const RageTimer &tm )
{
	DWORD numevents;
	DIDEVICEOBJECTDATA evtbuf[INPUT_QSIZE];

	numevents = INPUT_QSIZE;
	HRESULT hr = device.Device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), evtbuf, &numevents, 0 );
	if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
	{
		INPUTFILTER->ResetDevice( device.dev );
		return;
	}

	if( hr != DI_OK )
	{
		LOG->Trace( hr_ssprintf(hr, "UpdateBuffered: IDirectInputDevice2_GetDeviceData") );
		return;
	}

	if( GetForegroundWindow() != GraphicsWindow::GetHwnd() )
	{
		// Discard input when not focused, and release all keys.
		INPUTFILTER->ResetDevice( device.dev );
		return;
	}

	// reset mousewheel
	DeviceInput diUp = DeviceInput(device.dev, MOUSE_WHEELUP, 0.0f, tm);
	ButtonPressed( diUp );
	DeviceInput diDown = DeviceInput(device.dev, MOUSE_WHEELDOWN, 0.0f, tm);
	ButtonPressed( diDown );

	for( int i = 0; i < (int) numevents; ++i )
	{
		for(unsigned j = 0; j < device.Inputs.size(); ++j)
		{
			const input_t &in = device.Inputs[j];
			const InputDevice dev = device.dev;

			if( evtbuf[i].dwOfs != in.ofs )
				continue;

			switch( in.type )
			{
				case input_t::KEY:
					ButtonPressed( DeviceInput(dev, (DeviceButton) in.num, !!(evtbuf[i].dwData & 0x80), tm) );
					break;

				case input_t::BUTTON:
					if(dev == DEVICE_MOUSE)
					{
						DeviceButton mouseInput = DeviceButton_Invalid;

						if( in.ofs == DIMOFS_BUTTON0 ) mouseInput = MOUSE_LEFT;
						else if( in.ofs == DIMOFS_BUTTON1 ) mouseInput = MOUSE_RIGHT;
						else if( in.ofs == DIMOFS_BUTTON2 ) mouseInput = MOUSE_MIDDLE;
						else LOG->MapLog( "unknown input", 
								 "Mouse '%s' is returning an unknown mouse offset [button], %i",
								 device.m_sName.c_str(), in.ofs );
						ButtonPressed( DeviceInput(dev, mouseInput, !!evtbuf[i].dwData, tm) );
					}
					else
						ButtonPressed( DeviceInput(dev, enum_add2(JOY_BUTTON_1, in.num), !!evtbuf[i].dwData, tm) );
					break;

				case input_t::AXIS:
				{
					DeviceButton up = DeviceButton_Invalid, down = DeviceButton_Invalid;
					if(dev == DEVICE_MOUSE)
					{
						float l = int(evtbuf[i].dwData);
						POINT cursorPos;
						GetCursorPos(&cursorPos);
						// convert screen coordinates to client
						ScreenToClient(GraphicsWindow::GetHwnd(), &cursorPos);

						if( in.ofs == DIMOFS_X ) INPUTFILTER->UpdateCursorLocation((float)cursorPos.x,(float)cursorPos.y);
						else if( in.ofs == DIMOFS_Y ) INPUTFILTER->UpdateCursorLocation((float)cursorPos.x,(float)cursorPos.y);
						else if( in.ofs == DIMOFS_Z )
						{
							// positive values: WheelUp
							// negative values: WheelDown
							INPUTFILTER->UpdateMouseWheel(l);
							{
								up = MOUSE_WHEELUP; down = MOUSE_WHEELDOWN;
								float fWheelDelta = l;
								//l = SCALE( int(evtbuf[i].dwData), -WHEEL_DELTA, WHEEL_DELTA, 1.0f, -1.0f );
								if( l > 0 )
								{
									DeviceInput diUp = DeviceInput(dev, up, 1.0f, tm);
									DeviceInput diDown = DeviceInput(dev, down, 0.0f, tm);
									// This if statement used to be a while loop.  But Kevin
									// reported that scrolling the mouse wheel locked up input.
									// I assume that fWheelDelta was some absurdly large value,
									// causing an infinite loop.  Changing the while loop to an
									// if fixed the input lockup.  Anything that wants to
									// imitate multiple presses when the wheel is scrolled will
									// have to do it manually. -Kyz
									if( fWheelDelta >= WHEEL_DELTA )
									{
										ButtonPressed( diUp );
										ButtonPressed( diDown );
										INPUTFILTER->UpdateMouseWheel(fWheelDelta);
										fWheelDelta -= WHEEL_DELTA;
									}
								}
								else if( l < 0 )
								{
									DeviceInput diDown = DeviceInput(dev, down, 1.0f, tm);
									DeviceInput diUp = DeviceInput(dev, up, 0.0f, tm);
									// See comment for the l > 0 case. -Kyz
									if( fWheelDelta <= -WHEEL_DELTA )
									{
										ButtonPressed( diDown );
										ButtonPressed( diUp );
										INPUTFILTER->UpdateMouseWheel(fWheelDelta);
										fWheelDelta += WHEEL_DELTA;
									}
								}
								else
								{
									DeviceInput diUp = DeviceInput(dev, up, 0.0f, tm);
									ButtonPressed( diUp );
									DeviceInput diDown = DeviceInput(dev, down, 0.0f, tm);
									ButtonPressed( diDown );
								}
							}
						}
						else LOG->MapLog( "unknown input", 
										 "Mouse '%s' is returning an unknown mouse offset [axis], %i",
										 device.m_sName.c_str(), in.ofs );
					}
					else
					{
						// joystick
						if( in.ofs == DIJOFS_X ) { up = JOY_LEFT; down = JOY_RIGHT; }
						else if( in.ofs == DIJOFS_Y ) { up = JOY_UP; down = JOY_DOWN; }
						else if( in.ofs == DIJOFS_Z ) { up = JOY_Z_UP; down = JOY_Z_DOWN; }
						else if( in.ofs == DIJOFS_RX ) { up = JOY_ROT_UP; down = JOY_ROT_DOWN; }
						else if( in.ofs == DIJOFS_RY ) { up = JOY_ROT_LEFT; down = JOY_ROT_RIGHT; }
						else if( in.ofs == DIJOFS_RZ ) { up = JOY_ROT_Z_UP; down = JOY_ROT_Z_DOWN; }
						else if( in.ofs == DIJOFS_SLIDER(0) ) { up = JOY_AUX_1; down = JOY_AUX_2; }
						else if( in.ofs == DIJOFS_SLIDER(1) ) { up = JOY_AUX_3; down = JOY_AUX_4; }
						else LOG->MapLog( "unknown input", 
										 "Controller '%s' is returning an unknown joystick offset, %i",
										 device.m_sName.c_str(), in.ofs );
						
						float l = SCALE( int(evtbuf[i].dwData), 0.0f, 100.0f, 0.0f, 1.0f );
						if(GamePreferences::m_AxisFix)
						{
						  ButtonPressed( DeviceInput(dev, up, (l == 0) || (l == -1), tm) );
						  ButtonPressed( DeviceInput(dev, down,(l == 0) || (l == 1), tm) );

						}
						else
						{
						  ButtonPressed( DeviceInput(dev, up, max(-l,0), tm) );
						  ButtonPressed( DeviceInput(dev, down, max(+l,0), tm) ); 
						}
					}
					break;
				}

				case input_t::HAT:
				{
					const int pos = TranslatePOV( evtbuf[i].dwData );
					ButtonPressed( DeviceInput(dev, JOY_HAT_UP, !!(pos & HAT_UP_MASK), tm) );
					ButtonPressed( DeviceInput(dev, JOY_HAT_DOWN, !!(pos & HAT_DOWN_MASK), tm) );
					ButtonPressed( DeviceInput(dev, JOY_HAT_LEFT, !!(pos & HAT_LEFT_MASK), tm) );
					ButtonPressed( DeviceInput(dev, JOY_HAT_RIGHT, !!(pos & HAT_RIGHT_MASK), tm) );
				}
			}
		}
	}
}

const short XINPUT_GAMEPAD_THUMB_MIN = MINSHORT;
const short XINPUT_GAMEPAD_THUMB_MAX = MAXSHORT;

void InputHandler_DInput::UpdateXInput( XIDevice &device, const RageTimer &tm )
{
	using std::max;

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	if (XInputGetState(device.m_dwXInputSlot, &state) == ERROR_SUCCESS)
	{
		// map joysticks
		float lx = 0.f;
		float ly = 0.f;
		if (sqrt(pow(state.Gamepad.sThumbLX, 2) + pow(state.Gamepad.sThumbLY, 2)) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			lx = SCALE(state.Gamepad.sThumbLX + 0.f, XINPUT_GAMEPAD_THUMB_MIN + 0.f, XINPUT_GAMEPAD_THUMB_MAX + 0.f, -1.0f, 1.0f);
			ly = SCALE(state.Gamepad.sThumbLY + 0.f, XINPUT_GAMEPAD_THUMB_MIN + 0.f, XINPUT_GAMEPAD_THUMB_MAX + 0.f, -1.0f, 1.0f);
		}
		ButtonPressed(DeviceInput(device.dev, JOY_LEFT, max(-lx, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_RIGHT, max(+lx, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_UP, max(+ly, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_DOWN, max(-ly, 0.f), tm));

		float rx = 0.f;
		float ry = 0.f;
		if (sqrt(pow(state.Gamepad.sThumbRX, 2) + pow(state.Gamepad.sThumbRY, 2)) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			rx = SCALE(state.Gamepad.sThumbRX + 0.f, XINPUT_GAMEPAD_THUMB_MIN + 0.f, XINPUT_GAMEPAD_THUMB_MAX + 0.f, -1.0f, 1.0f);
			ry = SCALE(state.Gamepad.sThumbRY + 0.f, XINPUT_GAMEPAD_THUMB_MIN + 0.f, XINPUT_GAMEPAD_THUMB_MAX + 0.f, -1.0f, 1.0f);
		}
		ButtonPressed(DeviceInput(device.dev, JOY_LEFT_2, max(-rx, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_RIGHT_2, max(+rx, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_UP_2, max(+ry, 0.f), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_DOWN_2, max(-ry, 0.f), tm));

		// map buttons
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_1, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_2, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_3, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_X), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_4, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_5, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_START), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_6, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_7, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_8, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_9, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_10, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER), tm));
		
		// map triggers to buttons
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_11, !!(state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_BUTTON_12, !!(state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD), tm));

		// map hat buttons
		ButtonPressed(DeviceInput(device.dev, JOY_HAT_UP, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_HAT_DOWN, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_HAT_LEFT, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT), tm));
		ButtonPressed(DeviceInput(device.dev, JOY_HAT_RIGHT, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT), tm));
	}
	else
	{
		INPUTFILTER->ResetDevice(device.dev);
		return;
	}
}


void InputHandler_DInput::PollAndAcquireDevices( bool bBuffered )
{
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( Devices[i].buffered != bBuffered )
			continue;

		HRESULT hr = Devices[i].Device->Poll();
		if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		{
			INPUTFILTER->ResetDevice( Devices[i].dev );

			/* This will fail with "access denied" on the keyboard if we don't
			 * have focus. */
			hr = Devices[i].Device->Acquire();
			if( hr != DI_OK )
				continue;

			Devices[i].Device->Poll();
		}
	}
}

void InputHandler_DInput::Update()
{
	// Handle polled devices. Handle buffered, too, if there's no input thread to do it.
	PollAndAcquireDevices( false );
	if( !m_InputThread.IsCreated() )
		PollAndAcquireDevices( true );

	for( unsigned i = 0; i < XDevices.size(); ++i )
		UpdateXInput( XDevices[i], RageZeroTimer );

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
		{
			UpdatePolled( Devices[i], RageZeroTimer );
		}
		else if( !m_InputThread.IsCreated() )
		{
			// If we have an input thread, it'll handle buffered devices.
			UpdateBuffered( Devices[i], RageZeroTimer );
		}
	}

	InputHandler::UpdateTimer();
}

const float POLL_FOR_JOYSTICK_CHANGES_LENGTH_SECONDS = 15.0f;
const float POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS = 0.25f;

bool InputHandler_DInput::DevicesChanged()
{
	/* GetNumJoysticksSlow() blocks DirectInput for a while even if called from a
	 * different thread, so we can't poll with it. GetNumHidDevices() is fast,
	 * but sometimes the DirectInput joysticks haven't updated by  the time the
	 * HID registry value changes. So, poll using GetNumHidDevices(). When that
	 * changes, poll using GetNumJoysticksSlow() for a little while to give
	 * DirectInput time to catch up. On this XP machine (which? -aj), it takes
	 * 2-10 DirectInput polls (0.5-2.5 seconds) to catch a newly installed device
	 * after the registry value changes, and catches non-new plugged/unplugged
	 * devices on the first DirectInputPoll. Note that this "poll for N seconds"
	 * method will not work if the Add New Hardware wizard halts device
	 * installation to wait for a driver. Most of the joysticks people would want
	 * to use don't prompt for a driver though, and the wizard adds them pretty quickly. */

	int iOldNumHidDevices = m_iLastSeenNumHidDevices;
	m_iLastSeenNumHidDevices = GetNumHidDevices();
	if( iOldNumHidDevices != m_iLastSeenNumHidDevices )
	{
		LOG->Warn( "HID devices changes" );
		m_iNumTimesLeftToPollForJoysticksChanged = (int)(POLL_FOR_JOYSTICK_CHANGES_LENGTH_SECONDS / POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS);
	}

	if( m_iNumTimesLeftToPollForJoysticksChanged > 0 )
	{
		static RageTimer timerPollJoysticks;
		if( timerPollJoysticks.Ago() >= POLL_FOR_JOYSTICK_CHANGES_EVERY_SECONDS )
		{
			m_iNumTimesLeftToPollForJoysticksChanged--;
			timerPollJoysticks.Touch();
			LOG->Warn( "polling for joystick changes" );

			int iOldNumJoysticks = m_iLastSeenNumJoysticks;
			m_iLastSeenNumJoysticks = GetNumJoysticksSlow();
			if( iOldNumJoysticks != m_iLastSeenNumJoysticks )
			{
				LOG->Warn( "joysticks changed" );
				m_iNumTimesLeftToPollForJoysticksChanged = 0;
				return true;
			}
		}
	}

	return false;
}

void InputHandler_DInput::InputThreadMain()
{
	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		LOG->Warn(werr_ssprintf(GetLastError(), "Failed to set DirectInput thread priority"));

	// Enable priority boosting.
	SetThreadPriorityBoost( GetCurrentThread(), FALSE );

	vector<DIDevice*> BufferedDevices;
	HANDLE Handle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			continue;

		BufferedDevices.push_back( &Devices[i] );

		Devices[i].Device->Unacquire();
		HRESULT hr = Devices[i].Device->SetEventNotification( Handle );
		if( FAILED(hr) )
			LOG->Warn( "IDirectInputDevice2_SetEventNotification failed on %i", i );
		Devices[i].Device->Acquire();
	}

	while( !m_bShutdown )
	{
		CHECKPOINT;
		if( BufferedDevices.size() )
		{
			// Update buffered devices.
			PollAndAcquireDevices( true );

			int ret = WaitForSingleObjectEx( Handle, 50, true );
			if( ret == -1 )
			{
				LOG->Trace( werr_ssprintf(GetLastError(), "WaitForSingleObjectEx failed") );
				continue;
			}

			/* Update devices even if no event was triggered, since this also
			 * checks for focus loss. */
			RageTimer now;
			for( unsigned i = 0; i < BufferedDevices.size(); ++i )
				UpdateBuffered( *BufferedDevices[i], now );
		}
		CHECKPOINT;

		// If we have no buffered devices, we didn't delay at WaitForMultipleObjectsEx.
		if( BufferedDevices.size() == 0 )
			usleep( 50000 );
		CHECKPOINT;
	}
	CHECKPOINT;

	for( unsigned i = 0; i < Devices.size(); ++i )
	{
		if( !Devices[i].buffered )
			continue;

		Devices[i].Device->Unacquire();
		Devices[i].Device->SetEventNotification(nullptr);
	}

	CloseHandle(Handle);
}

void InputHandler_DInput::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	for( unsigned i=0; i < XDevices.size(); ++i )
		vDevicesOut.push_back( InputDeviceInfo(XDevices[i].dev, XDevices[i].m_sName ) );

	for( unsigned i=0; i < Devices.size(); ++i )
		vDevicesOut.push_back( InputDeviceInfo(Devices[i].dev, Devices[i].m_sName) );
}

static wchar_t ScancodeAndKeysToChar( DWORD scancode, unsigned char keys[256] )
{
	static HKL layout = GetKeyboardLayout(0); // 0 == current thread
	UINT vk = MapVirtualKeyEx( scancode, 1, layout );

	static bool bInitialized = false;

	typedef int (WINAPI TOUNICODEEX)( IN UINT wVirtKey, IN UINT wScanCode, IN CONST BYTE *lpKeyState, OUT LPWSTR pwszBuff, IN int cchBuff, IN UINT wFlags, IN HKL dwhkl );
	static TOUNICODEEX *pToUnicodeEx;

	if( !bInitialized )
	{
		bInitialized = true;
		HMODULE hModule = GetModuleHandle( "user32.dll" );
		pToUnicodeEx = (TOUNICODEEX *) GetProcAddress( hModule, "ToUnicodeEx" );
	}

	unsigned short result[2]; // ToAscii writes a max of 2 chars
	ZERO( result );

	if( pToUnicodeEx != nullptr )
	{
		int iNum = pToUnicodeEx( vk, scancode, keys, (LPWSTR)result, 2, 0, layout );
		if( iNum == 1 )
			return result[0];
	}
	else
	{
		int iNum = ToAsciiEx( vk, scancode, keys, result, 0, layout );
		// iNum == 2 will happen only for dead keys. See MSDN for ToAsciiEx.
		if( iNum == 1 )
		{
			RString s = RString()+(char)result[0];
			return ConvertCodepageToWString( s, CP_ACP )[0];
		}
	}

	return '\0';
}

wchar_t InputHandler_DInput::DeviceButtonToChar( DeviceButton button, bool bUseCurrentKeyModifiers )
{
	// ToAsciiEx maps these keys to a character. They shouldn't be mapped to any character.
	switch( button )
	{
	case KEY_ESC:
	case KEY_TAB:
	case KEY_ENTER:
	case KEY_BACK:
		return '\0';
	}

	for (DIDevice const &d : Devices)
	{
		if( d.type != DIDevice::KEYBOARD )
			continue;

		for (input_t const &i : d.Inputs)
		{
			if( button != i.num )
				continue;

			unsigned char keys[256];
			ZERO( keys );
			if( bUseCurrentKeyModifiers )
				GetKeyboardState(keys);
			// todo: handle Caps Lock -freem
			wchar_t c = ScancodeAndKeysToChar( i.ofs, keys );
			if( c )
				return c;
		}
	}

	return InputHandler::DeviceButtonToChar( button, bUseCurrentKeyModifiers );
}

/*
 * (c) 2003-2004 Glenn Maynard
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
