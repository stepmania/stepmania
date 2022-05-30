#include "global.h"
#include "InputHandler_Win32_ddrio.h"

#include "RageLog.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include <windows.h>
#include <process.h>
#include "PrefsManager.h"

typedef int (*thread_create_t)(
	int (*proc)(void*), void* ctx, uint32_t stack_sz, unsigned int priority);
typedef void (*thread_join_t)(int thread_id, int* result);
typedef void (*thread_destroy_t)(int thread_id);

typedef void (*log_formatter_t)(const char* module, const char* fmt, ...);

typedef bool (*DDRIO_IO_INIT)(thread_create_t thread_create,
	thread_join_t thread_join,
	thread_destroy_t thread_destro
	);
static DDRIO_IO_INIT ddrio_io_init;

typedef int (*DDRIO_READ_PAD)();
static DDRIO_READ_PAD ddrio_io_read_pad;

typedef int (*DDRIO_SETLIGHTS_P3IO)(uint32_t lights);
static DDRIO_SETLIGHTS_P3IO ddrio_set_lights_p3io;

typedef int (*DDRIO_SETLIGHTS_EXTIO)(uint32_t lights);
static DDRIO_SETLIGHTS_EXTIO ddrio_set_lights_extio;

typedef int (*DDRIO_SETLIGHTS_HDXSPANEL)(uint32_t lights);
static DDRIO_SETLIGHTS_HDXSPANEL ddrio_set_lights_hdxs_panel;

typedef int (*DDRIO_FINI)();
static DDRIO_FINI ddrio_fini;

HINSTANCE hDDRIOdll = nullptr;

REGISTER_INPUT_HANDLER_CLASS2( ddrio, Win32_ddrio );


struct shim_ctx {
	HANDLE barrier;
	int (*proc)(void*);
	void* ctx;
};

static unsigned int crt_thread_shim(void* outer_ctx)
{
	LOG->Trace("crt_thread_shim");

	struct shim_ctx* sctx = (struct shim_ctx*)outer_ctx;
	int (*proc)(void*);
	void* inner_ctx;

	proc = sctx->proc;
	inner_ctx = sctx->ctx;

	SetEvent(sctx->barrier);

	return proc(inner_ctx);
}


int crt_thread_create(
	int (*proc)(void*), void* ctx, uint32_t stack_sz, unsigned int priority)
{

	LOG->Trace("crt_thread_create");

	struct shim_ctx sctx;
	uintptr_t thread_id;

	sctx.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);
	sctx.proc = proc;
	sctx.ctx = ctx;

	thread_id = _beginthreadex(NULL, stack_sz, crt_thread_shim, &sctx, 0, NULL);

	WaitForSingleObject(sctx.barrier, INFINITE);
	CloseHandle(sctx.barrier);

	LOG->Trace("crt_thread_create %d", thread_id);

	return (int)thread_id;

}

void crt_thread_destroy(int thread_id)
{
	LOG->Trace("crt_thread_destroy %d", thread_id);

	CloseHandle((HANDLE)(uintptr_t)thread_id);
}


void crt_thread_join(int thread_id, int* result)
{
	LOG->Trace("crt_thread_join %d", thread_id);

	WaitForSingleObject((HANDLE)(uintptr_t)thread_id, INFINITE);

	if (result)
	{
		GetExitCodeThread((HANDLE)(uintptr_t)thread_id, (DWORD*)result);
	}
}



int ddrio_filter(unsigned int, struct _EXCEPTION_POINTERS*)
{
	return EXCEPTION_EXECUTE_HANDLER;
}

bool InputHandler_Win32_ddrio::LoadDLL()
{
	hDDRIOdll = LoadLibrary("ddrio.dll");

	if (hDDRIOdll == nullptr)
	{
		MessageBox(nullptr, "Could not load ddrio.dll. Ensure it is in the Program directory, the proper C++ Visual C++ Redistributable Runtimes dependencies are installed, you have matched your architectures (x86/x64), and any additional dll dependencies of ddrio.dll are available.", "ERROR", MB_OK);
		return false;
	}

	_ddriodll_loaded = true;

	return true;
}

bool InputHandler_Win32_ddrio::MapFunctions()
{
	__try
	{
		//pull the functions we need out of the dll.
		ddrio_io_init = (DDRIO_IO_INIT)GetProcAddress(hDDRIOdll, "ddr_io_init");
		ddrio_io_read_pad = (DDRIO_READ_PAD)GetProcAddress(hDDRIOdll, "ddr_io_read_pad");
		ddrio_fini = (DDRIO_FINI)GetProcAddress(hDDRIOdll, "ddr_io_fini");

		//these are technically optional depending on the device in question.
		ddrio_set_lights_p3io = (DDRIO_SETLIGHTS_P3IO)GetProcAddress(hDDRIOdll, "ddr_io_set_lights_p3io");
		ddrio_set_lights_extio = (DDRIO_SETLIGHTS_P3IO)GetProcAddress(hDDRIOdll, "ddr_io_set_lights_extio");
		ddrio_set_lights_hdxs_panel = (DDRIO_SETLIGHTS_P3IO)GetProcAddress(hDDRIOdll, "ddr_io_set_lights_hdxs_panel");
	}
	__except (ddrio_filter(GetExceptionCode(), GetExceptionInformation()))
	{
		MessageBox(nullptr, "Could not map functions in ddrio.dll", "ERROR", MB_OK);
		FreeLibrary(hDDRIOdll);
	}

	return true;
}

InputHandler_Win32_ddrio::InputHandler_Win32_ddrio()
{
	if (!_ddriodll_loaded && LoadDLL())
	{
		_ddriodll_loaded = MapFunctions();
	}

	bool ddrInit = false;

	thread_create_t thread_impl_create = crt_thread_create;
	thread_join_t thread_impl_join = crt_thread_join;
	thread_destroy_t thread_impl_destroy = crt_thread_destroy;


	if (ddrio_io_init != nullptr)
	{
		ddrInit = ddrio_io_init(thread_impl_create, thread_impl_join, thread_impl_destroy);
	}

	if (!ddrInit)
	{
		LOG->Warn("Could not init ddrio.dll");
	}

	m_bShutdown = false;

	if(_ddriodll_loaded && ddrInit && PREFSMAN->m_bThreadedInput )
	{
		InputThread.SetName( "ddrio thread" );
		InputThread.Create( InputThread_Start, this );
	}
}

InputHandler_Win32_ddrio::~InputHandler_Win32_ddrio()
{
	if( InputThread.IsCreated() )
	{
		m_bShutdown = true;
		LOG->Trace( "Shutting down ddrio thread ..." );
		InputThread.Wait();
		LOG->Trace( "ddrio thread shut down." );

		
		if (ddrio_fini != nullptr)
		{
			LOG->Trace("calling ddrio dll fini");
			ddrio_fini();
			LOG->Trace("ddrio dll complete");
		}
		
	}
}

RString InputHandler_Win32_ddrio::GetDeviceSpecificInputString( const DeviceInput &di )
{
	switch (di.button)
	{
		case JOY_BUTTON_1:  return "ddrio P1 Menu Up";
		case JOY_BUTTON_2:  return "ddrio P1 Menu Down";
		case JOY_BUTTON_3:  return "ddrio P2 Menu Up";
		case JOY_BUTTON_4:  return "ddrio P2 Menu Down";
		case JOY_BUTTON_5:  return "ddrio Test";
		case JOY_BUTTON_6:  return "ddrio Coin";
		case JOY_BUTTON_7:  return "ddrio Service";
		case JOY_BUTTON_8:  return "ddrio UNUSED 8";
		case JOY_BUTTON_9:  return "ddrio P2 Start";
		case JOY_BUTTON_10: return "ddrio P2 Pad Up";
		case JOY_BUTTON_11: return "ddrio P2 Pad Down";
		case JOY_BUTTON_12: return "ddrio P2 Pad Left";
		case JOY_BUTTON_13: return "ddrio P2 Pad Right";
		case JOY_BUTTON_14: return "ddrio UNUSED 14";
		case JOY_BUTTON_15: return "ddrio P2 Menu Left";
		case JOY_BUTTON_16: return "ddrio P2 Menu Right";
		case JOY_BUTTON_17: return "ddrio P1 Start";
		case JOY_BUTTON_18: return "ddrio P1 Pad Up";
		case JOY_BUTTON_19: return "ddrio P1 Pad Down";
		case JOY_BUTTON_20: return "ddrio P1 Pad Left";
		case JOY_BUTTON_21: return "ddrio P1 Pad Right";
		case JOY_BUTTON_22: return "ddrio UNUSED 22";
		case JOY_BUTTON_23: return "ddrio P1 Menu Left";
		case JOY_BUTTON_24: return "ddrio P1 Menu Right";
		case JOY_BUTTON_25: return "ddrio UNUSED 25";
		case JOY_BUTTON_26: return "ddrio UNUSED 26";
		case JOY_BUTTON_27: return "ddrio UNUSED 27";
		case JOY_BUTTON_28: return "ddrio UNUSED 28";
		case JOY_BUTTON_29: return "ddrio UNUSED 29";
		case JOY_BUTTON_30: return "ddrio UNUSED 30";
		case JOY_BUTTON_31: return "ddrio UNUSED 31";
		case JOY_BUTTON_32: return "ddrio UNUSED 32";
	}

	return InputHandler::GetDeviceSpecificInputString(di);
}

void InputHandler_Win32_ddrio::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	// We use a joystick device so we can get automatic input mapping
	vDevicesOut.push_back(InputDeviceInfo(InputDevice(DDRIO_DEVICEID), "ddrio"));
}

int InputHandler_Win32_ddrio::InputThread_Start( void *p )
{
	((InputHandler_Win32_ddrio *) p)->InputThreadMain();
	return 0;
}

void InputHandler_Win32_ddrio::InputThreadMain()
{
	uint32_t prevInput = 0, newInput = 0;
	LightsState prevLS = { 0 };
	LightsState newLS = { 0 };

	while (!m_bShutdown)
	{
		InputHandler::UpdateTimer();

		//Btools API states that this method will sleep/halt
		//so we can safely loop around it in a thread.
		newInput = ddrio_io_read_pad();

		if (prevInput != newInput)
		{
			PushInputState(newInput);
		}

		newLS = LightsDriver_Export::GetState();

		if (IsLightChange(prevLS, newLS))
		{
			PushLightState(newLS);
		}

		prevLS = newLS;
		prevInput = newInput;
	}
}

void InputHandler_Win32_ddrio::PushInputState(uint32_t newInput)
{
	for (int i = 0; i < 32; i++)
	{
		bool pressed = (newInput & (1 << i));

		//return from ddrio is active high.
		DeviceInput di(DDRIO_DEVICEID, enum_add2(JOY_BUTTON_1, i), pressed);

		//If we're in a thread, our timestamp is accurate.
		if (InputThread.IsCreated())
			di.ts.Touch();

		ButtonPressed(di);
	}
}

bool InputHandler_Win32_ddrio::IsLightChange(LightsState prevLS, LightsState newLS)
{
	FOREACH_CabinetLight(light)
	{
		if (prevLS.m_bCabinetLights[light] != newLS.m_bCabinetLights[light])
		{
			return true;
		}
	}

	for (int gc = 0; gc < NUM_GameController; gc++)
	{
		FOREACH_ENUM(GameButton, gb)
		{
			if (prevLS.m_bGameButtonLights[gc][gb] != newLS.m_bGameButtonLights[gc][gb])
			{
				return true;
			}
		}
	}
	
	return false;
}

void InputHandler_Win32_ddrio::PushLightState(LightsState newLS)
{
	uint32_t p3io = 0;
	uint32_t hdxs = 0;
	uint32_t extio = 0;

	//lighting state has already been verified to have changed in this method, so create the new one from scratch.

	if (newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) p3io |= (1 << LIGHT_P1_MENU);
	if (newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) p3io |= (1 << LIGHT_P2_MENU);
	if (newLS.m_bCabinetLights[LIGHT_MARQUEE_LR_RIGHT]) p3io |= (1 << LIGHT_P2_LOWER_LAMP);
	if (newLS.m_bCabinetLights[LIGHT_MARQUEE_UP_RIGHT]) p3io |= (1 << LIGHT_P2_UPPER_LAMP);
	if (newLS.m_bCabinetLights[LIGHT_MARQUEE_LR_LEFT]) p3io |= (1 << LIGHT_P1_LOWER_LAMP);
	if (newLS.m_bCabinetLights[LIGHT_MARQUEE_UP_LEFT]) p3io |= (1 << LIGHT_P1_UPPER_LAMP);

	//hdxs device has one light for both MenuLeft and MenuRight as well as MenuUp and MenuDown.
	if (newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_START]) hdxs |= (1 << LIGHT_HD_P1_START);
	if (newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_MENUUP] || newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_MENUDOWN]) hdxs |= (1 << LIGHT_HD_P1_UP_DOWN);
	if (newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_MENULEFT] || newLS.m_bGameButtonLights[GameController_1][GAME_BUTTON_MENURIGHT]) hdxs |= (1 << LIGHT_HD_P1_LEFT_RIGHT);
	if (newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_START]) hdxs |= (1 << LIGHT_HD_P2_START);
	if (newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_MENUUP] || newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_MENUDOWN]) hdxs |= (1 << LIGHT_HD_P2_UP_DOWN);
	if (newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_MENULEFT] || newLS.m_bGameButtonLights[GameController_2][GAME_BUTTON_MENURIGHT]) hdxs |= (1 << LIGHT_HD_P2_LEFT_RIGHT);

	if (newLS.m_bGameButtonLights[GameController_2][DANCE_BUTTON_RIGHT]) extio |= (1 << LIGHT_P2_RIGHT);
	if (newLS.m_bGameButtonLights[GameController_2][DANCE_BUTTON_LEFT]) extio |= (1 << LIGHT_P2_LEFT);
	if (newLS.m_bGameButtonLights[GameController_2][DANCE_BUTTON_DOWN]) extio |= (1 << LIGHT_P2_DOWN);
	if (newLS.m_bGameButtonLights[GameController_2][DANCE_BUTTON_UP]) extio |= (1 << LIGHT_P2_UP);

	if (newLS.m_bGameButtonLights[GameController_1][DANCE_BUTTON_RIGHT]) extio |= (1 << LIGHT_P1_RIGHT);
	if (newLS.m_bGameButtonLights[GameController_1][DANCE_BUTTON_LEFT]) extio |= (1 << LIGHT_P1_LEFT);
	if (newLS.m_bGameButtonLights[GameController_1][DANCE_BUTTON_DOWN]) extio |= (1 << LIGHT_P1_DOWN);
	if (newLS.m_bGameButtonLights[GameController_1][DANCE_BUTTON_UP]) extio |= (1 << LIGHT_P1_UP);

	if (newLS.m_bCabinetLights[LIGHT_BASS_LEFT] || newLS.m_bCabinetLights[LIGHT_BASS_RIGHT]) extio |= (1 << LIGHT_NEONS);

	//all of these functions are technically optional,
	//so check to see if we were able to find them in this specific dll before calling a nullptr
	if(ddrio_set_lights_p3io != nullptr)
		ddrio_set_lights_p3io(p3io);

	if (ddrio_set_lights_extio != nullptr)
		ddrio_set_lights_extio(extio);

	if (ddrio_set_lights_hdxs_panel != nullptr)
		ddrio_set_lights_hdxs_panel(hdxs);
}



/*
 * (c) 2022 din
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

