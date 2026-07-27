#include "global.h"
#include "InputHandler_Linux_Joystick.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "LinuxInputManager.h"
#include "RageInputDevice.h" // NUM_JOYSTICKS

#include <cerrno>
#include <set>
#include <vector>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/joystick.h>


REGISTER_INPUT_HANDLER_CLASS2( LinuxJoystick, Linux_Joystick );

InputHandler_Linux_Joystick::InputHandler_Linux_Joystick()
{
	m_bDevicesChanged = false;

	LOG->Trace( "InputHandler_Linux_Joystick::InputHandler_Linux_Joystick" );

	if( LINUXINPUT == nullptr ) LINUXINPUT = new LinuxInputManager;
	LINUXINPUT->InitDriver(this);

	if( !m_files.empty() ) // LinuxInputManager found at least one valid joystick for us
		StartThread();
}

InputHandler_Linux_Joystick::~InputHandler_Linux_Joystick()
{
	if( m_InputThread.IsCreated() )
		StopThread();

	for( auto& f : m_files ) {
		if( f.fd != -1 ) close(f.fd);
	}
	m_files.clear();
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
		FileDescriptor f;
		f.fd = open( dev, O_RDONLY );
		if(f.fd != -1)
		{
			char szName[1024];
			ZERO( szName );
			if( ioctl(f.fd, JSIOCGNAME(sizeof(szName)), szName) < 0 )
				f.description = ssprintf( "Unknown joystick at %s", dev.c_str() );
			else
				f.description = szName;

			LOG->Info("LinuxJoystick: Opened %s", dev.c_str() );
			m_bDevicesChanged = true;
			ret = true;

			m_files.push_back(f);
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

		for(size_t i = 0; i < m_files.size(); ++i)
		{
			if (m_files[i].fd < 0)
				continue;

			FD_SET(m_files[i].fd, &fdset);
			max_fd = std::max(max_fd, m_files[i].fd);
		}

		if(max_fd == -1)
			break;

		struct timeval zero = {0,100000};
		if( select(max_fd+1, &fdset, nullptr, nullptr, &zero) <= 0 )
			continue;
		RageTimer now;

		for(size_t i = 0; i < m_files.size(); ++i)
		{
			if( m_files[i].fd == -1 )
				continue;

			if(!FD_ISSET(m_files[i].fd, &fdset))
				continue;

			js_event event;
			int ret = read(m_files[i].fd, &event, sizeof(event));

			if(ret == -1)
			{
				LOG->Warn("Error reading from joystick %zu: %s; disabled", i, strerror(errno));
				close(m_files[i].fd);
				m_files[i].fd = -1;
				continue;
			}

			if(ret != sizeof(event))
			{
				LOG->Warn("Unexpected packet (size %i != %i) from joystick %zu; disabled", ret, (int)sizeof(event), i);
				close(m_files[i].fd);
				m_files[i].fd = -1;
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
				ButtonPressed( DeviceInput(id, neg, std::max(-l, 0.0f), now) );
				ButtonPressed( DeviceInput(id, pos, std::max(+l, 0.0f), now) );
				break;
			}

			default:
				LOG->Warn("Unexpected packet (type %i) from joystick %zu; disabled", event.type, i);
				close(m_files[i].fd);
				m_files[i].fd = -1;
				continue;
			}

		}

	}

	InputHandler::UpdateTimer();
}

void InputHandler_Linux_Joystick::GetDevicesAndDescriptions( std::vector<InputDeviceInfo>& vDevicesOut )
{
	// HACK: If IH_Linux_Joystick is constructed before IH_Linux_Event, our thread won't be started
	// as part of the constructor. This isn't called until all InputHandlers have been constructed,
	// and is (hopefully) in the same thread as TryDevice... so doublecheck our thread now.
	if( m_files.empty() && !m_InputThread.IsCreated() ) StartThread();

	for(size_t i = 0; i < m_files.size(); ++i)
	{
		if (m_files[i].fd < 0)
			continue;

		vDevicesOut.push_back( InputDeviceInfo(InputDevice(DEVICE_JOY1+i), m_files[i].description) );
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
