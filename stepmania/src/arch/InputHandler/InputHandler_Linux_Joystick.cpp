#include "global.h"
#include "InputHandler_Linux_Joystick.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <sys/types.h>
#include <linux/joystick.h>

#include <set>

static const char *Paths[InputHandler_Linux_Joystick::NUM_JOYSTICKS] =
{
	"/dev/js0",
	"/dev/js1",
	"/dev/input/js0",
	"/dev/input/js1",
};

InputHandler_Linux_Joystick::InputHandler_Linux_Joystick()
{
	LOG->Trace( "InputHandler_Linux_Joystick::InputHandler_Linux_Joystick" );
	
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
		fds[i] = -1;

	/* We check both eg. /dev/js0 and /dev/input/js0.  If both exist, they're probably
	 * the same device; keep track of device IDs so we don't open the same joystick
	 * twice. */
	set< pair<int,int> > devices;
	
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
	{
		struct stat st;
		if( stat( Paths[i], &st ) == -1 )
		{
			if( errno != ENOENT )
				LOG->Warn( "Couldn't stat %s: %s", Paths[i], strerror(errno) );
			continue;
		}

		if( !S_ISCHR( st.st_mode ) )
		{
			LOG->Warn( "Ignoring %s: not a character device", Paths[i] );
			continue;
		}

		pair<int,int> dev( major(st.st_rdev), minor(st.st_rdev) );
		if( devices.find(dev) != devices.end() )
			continue; /* dupe */
		devices.insert( dev );

		fds[i] = open( Paths[i], O_RDONLY );

		if(fds[i] != -1)
		{
			char szName[1024];
			ZERO( szName );
			if( ioctl(fds[i], JSIOCGNAME(sizeof(szName)), szName) < 0 )
				m_sDescription[i] = ssprintf( "Unknown joystick at %s", Paths[i] );
			else
				m_sDescription[i] = szName;

			LOG->Info("Opened %s", Paths[i]);
		}
	}
}
	
InputHandler_Linux_Joystick::~InputHandler_Linux_Joystick()
{
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
		if(fds[i] != -1) close(fds[i]);
}

void InputHandler_Linux_Joystick::Update(float fDeltaTime)
{
	while(1)
	{
		fd_set fdset;
		FD_ZERO(&fdset);
		int max_fd = -1;
		
		for(int i = 0; i < NUM_JOYSTICKS; ++i)
		{
			if (fds[i] < 0)
				continue;

			FD_SET(fds[i], &fdset);
			max_fd = max(max_fd, fds[i]);
		}

		if(max_fd == -1)
			break;

		struct timeval zero = {0,0};
		if ( select(max_fd+1, &fdset, NULL, NULL, &zero) <= 0 )
			break;

		for(int i = 0; i < NUM_JOYSTICKS; ++i)
		{
			if( fds[i] == -1 )
				continue;

			if(!FD_ISSET(fds[i], &fdset))
				continue;

			js_event event;
			int ret = read(fds[i], &event, sizeof(event));
			if(ret != sizeof(event))
			{
				LOG->Warn("Unexpected packet (size %i != %i) from joystick %i; disabled", ret, sizeof(event), i);
				close(fds[i]);
				fds[i] = -1;
				continue;
			}

			InputDevice id = InputDevice(DEVICE_JOY1 + i);

			event.type &= ~JS_EVENT_INIT;
			switch (event.type) {
			case JS_EVENT_BUTTON: {
				int iNum = event.number;
				// In 2.6.11 using an EMS USB2, the event number for P1 Tri (the first button)
				// is being reported as 32 instead of 0.  Correct for this.
				wrap( iNum, 32 );	// max number of joystick buttons.  Make this a constant?
				ButtonPressed(DeviceInput(id, JOY_1 + iNum), event.value);
				break;
			}
				
			case JS_EVENT_AXIS: {
				JoystickButton neg = (JoystickButton)(JOY_LEFT+2*event.number);
				JoystickButton pos = (JoystickButton)(JOY_RIGHT+2*event.number);
				ButtonPressed(DeviceInput(id, neg), event.value < -16000);
				ButtonPressed(DeviceInput(id, pos), event.value > +16000);
				break;
			}
				
			default:
				LOG->Warn("Unexpected packet (type %i) from joystick %i; disabled", event.type, i);
				close(fds[i]);
				fds[i] = -1;
				continue;
			}

		}

	}

	InputHandler::UpdateTimer();
}

void InputHandler_Linux_Joystick::GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut)
{
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
	{
		if (fds[i] < 0)
			continue;

		vDevicesOut.push_back( InputDevice(DEVICE_JOY1+i) );
		vDescriptionsOut.push_back( m_sDescription[i] );
	}
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
