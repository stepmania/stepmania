#ifndef INPUT_HANDLER_WIN32_PUMP_H
#define INPUT_HANDLER_WIN32_PUMP_H 1

#include "InputHandler.h"

class PumpPadDevice: RageInputFeeder
{
	struct dev_t; /* MYOB */

	dev_t *dev;

public:
	void Update();
	PumpPadDevice();
	~PumpPadDevice();
};



#endif
