#ifndef INPUT_HANDLER_LINUX_TTY_H
#define INPUT_HANDLER_LINUX_TTY_H 1

#include "InputHandler.h"

class InputHandler_Linux_tty: public InputHandler
{
	int fd;
	static void OnCrash(int);
	
public:
	void Update(float fDeltaTime);
	InputHandler_Linux_tty();
	~InputHandler_Linux_tty();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
};

#endif

