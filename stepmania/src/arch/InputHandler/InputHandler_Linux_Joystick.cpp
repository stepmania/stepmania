#include "global.h"
#include "InputHandler_Linux_Joystick.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/types.h>
#include <linux/joystick.h>

static const char *Paths[InputHandler_Linux_Joystick::NUM_JOYSTICKS] =
{
	"/dev/js0",
	"/dev/js1",
};

InputHandler_Linux_Joystick::InputHandler_Linux_Joystick()
{
	for(int i = 0; i < NUM_JOYSTICKS; ++i)
		fds[i] = -1;

	for(int i = 0; i < NUM_JOYSTICKS; ++i)
	{
		fds[i] = open(Paths[i], O_RDONLY);

		if(fds[i] != -1)
			LOG->Info("Opened %s", Paths[i]);
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
			return;

		struct timeval zero = {0,0};
		if ( select(max_fd+1, &fdset, NULL, NULL, &zero) <= 0 )
			return;

		for(int i = 0; i < NUM_JOYSTICKS; ++i)
		{
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
				ButtonPressed(DeviceInput(id, JOY_1 + event.number), event.value);
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
		vDescriptionsOut.push_back( ssprintf("Joystick %i", i) );
	}
}


