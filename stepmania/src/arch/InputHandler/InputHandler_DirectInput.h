#ifndef INPUTHANDLER_DIRECTINPUT_H
#define INPUTHANDLER_DIRECTINPUT_H

#include "InputHandler.h"
#include "RageThreads.h"

struct DIDevice;
class InputHandler_DInput: public InputHandler
{
	RageThread InputThread;
	bool shutdown;

	void UpdatePolled(DIDevice &device, const RageTimer &tm);
	void UpdateBuffered(DIDevice &device, const RageTimer &tm);
	void PollAndAcquireDevices();

	static int InputThread_Start( void *p ) { ((InputHandler_DInput *) p)->InputThreadMain();  return 0; }
	void InputThreadMain();

	void StartThread();
	void ShutdownThread();

public:
	InputHandler_DInput();
	~InputHandler_DInput();
	void GetDevicesAndDescriptions(vector<InputDevice>& vDevicesOut, vector<CString>& vDescriptionsOut);
	void Update(float fDeltaTime);
	void WindowReset();
};

#endif
