#ifndef INPUT_HANDLER_WIN32_PUMP_H
#define INPUT_HANDLER_WIN32_PUMP_H 1

#include "InputHandler.h"

class InputHandler_Win32_Pump: public InputHandler
{
	struct dev_t; /* MYOB */

	dev_t *dev;

public:
	void Update(float fDeltaTime);
	InputHandler_Win32_Pump();
	~InputHandler_Win32_Pump();
};



#endif
