#include "global.h"
#include "InputHandler_Linux_PIUIO.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <set>

REGISTER_INPUT_HANDLER_CLASS2( PIUIO, Linux_PIUIO );

InputHandler_Linux_PIUIO::InputHandler_Linux_PIUIO()
{
	LOG->Trace( "InputHandler_Linux_PIUIO::InputHandler_Linux_PIUIO" );

	fd = open( "/dev/piuio0", O_RDONLY );
	if( fd < 0 )
	{
		LOG->Warn( "Couldn't open PIUIO device: %s", strerror(errno) );
		return;
	}

	struct stat st;
	if( fstat( fd, &st ) == -1 )
	{
		LOG->Warn( "Couldn't stat PIUIO device: %s", strerror(errno) );
		close(fd);
		return;
	}

	if( !S_ISCHR( st.st_mode ) )
	{
		LOG->Warn( "Ignoring /dev/piuio0: not a character device" );
		close(fd);
		return;
	}

	LOG->Info("Opened PIUIO device for input");

	m_bShutdown = false;

	m_InputThread.SetName( "PIUIO thread" );
	m_InputThread.Create( InputThread_Start, this );
}

InputHandler_Linux_PIUIO::~InputHandler_Linux_PIUIO()
{
	if( m_InputThread.IsCreated() )
	{
		m_bShutdown = true;
		LOG->Trace( "Shutting down PIUIO thread ..." );
		m_InputThread.Wait();
		LOG->Trace( "PIUIO thread shut down." );
	}

	if (fd >= 0)
		close(fd);
}

int InputHandler_Linux_PIUIO::InputThread_Start( void *p )
{
	((InputHandler_Linux_PIUIO *) p)->InputThread();
	return 0;
}

void InputHandler_Linux_PIUIO::InputThread()
{
	unsigned char inputs[32];
	while( !m_bShutdown )
	{
		int ret = read(fd, &inputs, sizeof(inputs));
		if (ret != sizeof(inputs))
		{
			LOG->Warn("Unexpected packet (size %i != %i) from PIUIO", ret, (int)sizeof(inputs));
			continue;
		}
		RageTimer now;

		InputDevice id = InputDevice(DEVICE_JOY1);

		int i;
		for (i = 8; i < 32; i++)
			inputs[i % 8] &= inputs[i];
		for (i = 0; i < 8; i++)
			lastInputs[i] ^= inputs[i];
		for (i = 0; i < 64; i++)
			if (lastInputs[i / 8] & (128 >> (i % 8)))
				ButtonPressed(DeviceInput(id, enum_add2(JOY_BUTTON_1, i),
						!(inputs[i / 8] & (128 >> (i % 8))), now));
		for (i = 0; i < 8; i++)
			lastInputs[i] = inputs[i];
	}

	InputHandler::UpdateTimer();
}

void InputHandler_Linux_PIUIO::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	vDevicesOut.push_back( InputDeviceInfo(InputDevice(DEVICE_PIUIO), "PIUIO") );
}

/*
 * Written by Devin J. Pohly, 2012.  Based on code from Input_Linux_Joystick,
 * the copyright and license for which is reproduced below.
 *
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
