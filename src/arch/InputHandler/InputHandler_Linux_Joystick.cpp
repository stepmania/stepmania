#include "global.h"
#include "InputHandler_Linux_Joystick.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "LinuxInputManager.h"
#include "RageInputDevice.h" // NUM_JOYSTICKS

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/joystick.h>

#include <set>

REGISTER_INPUT_HANDLER_CLASS2( LinuxJoystick, Linux_Joystick );

InputHandler_Linux_Joystick::InputHandler_Linux_Joystick()
{
	m_bDevicesChanged = false;
	
	LOG->Trace( "InputHandler_Linux_Joystick::InputHandler_Linux_Joystick" );
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
		fds[i] = -1;
	
	m_iLastFd = 0;

	if( LINUXINPUT == nullptr ) LINUXINPUT = new LinuxInputManager;
	LINUXINPUT->InitDriver(this);

	if( fds[0] != -1 ) // LinuxInputManager found at least one valid joystick for us
		StartThread();
}

InputHandler_Linux_Joystick::~InputHandler_Linux_Joystick()
{
	if( m_InputThread.IsCreated() )
		StopThread();

	for(int i = 0; i < NUM_JOYSTICKS; ++i)
		if(fds[i] != -1) close(fds[i]);
}

void InputHandler_Linux_Joystick::StartThread()
{
	m_bShutdown = false;
	m_InputThread.SetName( "Joystick thread" );
	m_InputThread.Create( InputThread_Start, this );
}

void InputHandler_Linux_Joystick::StopThread()
{
	m_bShutdown = true;
	LOG->Trace( "Shutting down joystick thread ..." );
	m_InputThread.Wait();
	LOG->Trace( "Joystick thread shut down." );
}

bool InputHandler_Linux_Joystick::TryDevice(RString dev)
{
	struct stat st;
	if( stat( dev, &st ) == -1 )
		{ LOG->Warn( "LinuxJoystick: Couldn't stat %s: %s", dev.c_str(), strerror(errno) ); return false; }

	if( !S_ISCHR( st.st_mode ) )
		{ LOG->Warn( "LinuxJoystick: Ignoring %s: not a character device", dev.c_str() ); return false; }
	
	bool ret = false;
	bool hotplug = false;
	if( m_InputThread.IsCreated() ) { StopThread(); hotplug = true; }
	/* Thread is stopped! DO NOT RETURN */
	{
		fds[m_iLastFd] = open( dev, O_RDONLY );

		if(fds[m_iLastFd] != -1)
		{
			char szName[1024];
			ZERO( szName );
			if( ioctl(fds[m_iLastFd], JSIOCGNAME(sizeof(szName)), szName) < 0 )
				m_sDescription[m_iLastFd] = ssprintf( "Unknown joystick at %s", dev.c_str() );
			else
				m_sDescription[m_iLastFd] = szName;

			LOG->Info("LinuxJoystick: Opened %s", dev.c_str() );
			m_iLastFd++;
			m_bDevicesChanged = true;
			ret = true;
		}
		else LOG->Warn("LinuxJoystick: Failed to open %s: %s", dev.c_str(), strerror(errno) );
	}
	if( hotplug ) StartThread();
	
	return ret;
}

int InputHandler_Linux_Joystick::InputThread_Start( void *p )
{
	((InputHandler_Linux_Joystick *) p)->InputThread();
	return 0;
}

void InputHandler_Linux_Joystick::InputThread()
{
	while( !m_bShutdown )
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

		struct timeval zero = {0,100000};
		if( select(max_fd+1, &fdset, nullptr, nullptr, &zero) <= 0 )
			continue;
		RageTimer now;

		for(int i = 0; i < NUM_JOYSTICKS; ++i)
		{
			if( fds[i] == -1 )
				continue;

			if(!FD_ISSET(fds[i], &fdset))
				continue;

			js_event event;
			int ret = read(fds[i], &event, sizeof(event));

			if(ret == -1)
			{
				LOG->Warn("Error reading from joystick %i: %s; disabled", i, strerror(errno));
				close(fds[i]);
				fds[i] = -1;
				continue;
			}

			if(ret != sizeof(event))
			{
				LOG->Warn("Unexpected packet (size %i != %i) from joystick %i; disabled", ret, (int)sizeof(event), i);
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
				ButtonPressed( DeviceInput(id, enum_add2(JOY_BUTTON_1, iNum), event.value, now) );
				break;
			}

			case JS_EVENT_AXIS: {
				DeviceButton neg = enum_add2(JOY_LEFT, 2*event.number);
				DeviceButton pos = enum_add2(JOY_RIGHT, 2*event.number);
                                float l = SCALE( int(event.value), 0.0f, 32767, 0.0f, 1.0f );
				ButtonPressed( DeviceInput(id, neg, max(-l,0), now) );
				ButtonPressed( DeviceInput(id, pos, max(+l,0), now) );
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

void InputHandler_Linux_Joystick::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut )
{
	// HACK: If IH_Linux_Joystick is constructed before IH_Linux_Event, our thread won't be started
	// as part of the constructor. This isn't called until all InputHandlers have been constructed,
	// and is (hopefully) in the same thread as TryDevice... so doublecheck our thread now.
	if( fds[0] != -1 && !m_InputThread.IsCreated() ) StartThread();
	
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
	{
		if (fds[i] < 0)
			continue;

		vDevicesOut.push_back( InputDeviceInfo(InputDevice(DEVICE_JOY1+i), m_sDescription[i]) );
	}
	m_bDevicesChanged = false;
}

/*
 * (c) 2003-2004 Glenn Maynard
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
