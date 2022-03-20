#ifndef INPUT_HANDLER_WIN32_DDRIO_H
#define INPUT_HANDLER_WIN32_DDRIO_H

#include "InputHandler.h"
#include "RageThreads.h"
#include "arch/Lights/LightsDriver_Export.h"

static bool _ddriodll_loaded = false;

//we want to use a 
#define DDRIO_DEVICEID DEVICE_JOY1

enum p3io_light_bit {
	LIGHT_P1_MENU = 0x00,
	LIGHT_P2_MENU = 0x01,
	LIGHT_P2_LOWER_LAMP = 0x04,
	LIGHT_P2_UPPER_LAMP = 0x05,
	LIGHT_P1_LOWER_LAMP = 0x06,
	LIGHT_P1_UPPER_LAMP = 0x07,
};

enum hdxs_light_bit {
	LIGHT_HD_P1_START = 0x08,
	LIGHT_HD_P1_UP_DOWN = 0x09,
	LIGHT_HD_P1_LEFT_RIGHT = 0x0A,
	LIGHT_HD_P2_START = 0x0B,
	LIGHT_HD_P2_UP_DOWN = 0x0C,
	LIGHT_HD_P2_LEFT_RIGHT = 0x0D,
};

// the indexing starts from 0x20 if you're looking in geninput
enum hdxs_rgb_light_idx {
	LIGHT_HD_P1_SPEAKER_F_R = 0x00,
	LIGHT_HD_P1_SPEAKER_F_G = 0x01,
	LIGHT_HD_P1_SPEAKER_F_B = 0x02,
	LIGHT_HD_P2_SPEAKER_F_R = 0x03,
	LIGHT_HD_P2_SPEAKER_F_G = 0x04,
	LIGHT_HD_P2_SPEAKER_F_B = 0x05,
	LIGHT_HD_P1_SPEAKER_W_R = 0x06,
	LIGHT_HD_P1_SPEAKER_W_G = 0x07,
	LIGHT_HD_P1_SPEAKER_W_B = 0x08,
	LIGHT_HD_P2_SPEAKER_W_R = 0x09,
	LIGHT_HD_P2_SPEAKER_W_G = 0x0A,
	LIGHT_HD_P2_SPEAKER_W_B = 0x0B,
};

enum extio_light_bit {
	LIGHT_NEONS = 0x0E,

	LIGHT_P2_RIGHT = 0x13,
	LIGHT_P2_LEFT = 0x14,
	LIGHT_P2_DOWN = 0x15,
	LIGHT_P2_UP = 0x16,

	LIGHT_P1_RIGHT = 0x1B,
	LIGHT_P1_LEFT = 0x1C,
	LIGHT_P1_DOWN = 0x1D,
	LIGHT_P1_UP = 0x1E
};


class InputHandler_Win32_ddrio: public InputHandler
{
public:
	InputHandler_Win32_ddrio();
	~InputHandler_Win32_ddrio();

	RString GetDeviceSpecificInputString( const DeviceInput &di );
	void GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut );

private:
	RageThread InputThread;
	bool m_bShutdown;

	bool LoadDLL();
	bool MapFunctions();
	static int InputThread_Start( void *p );
	void InputThreadMain();

	void PushInputState(uint32_t newInput);

	bool IsLightChange(LightsState prevLS, LightsState newLS);
	void PushLightState(LightsState newLS);
};

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


#endif
