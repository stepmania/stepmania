#ifndef LOW_LEVEL_WINDOW_NULL_H
#define LOW_LEVEL_WINDOW_NULL_H

#include "LowLevelWindow.h"

class LowLevelWindow_Null: public LowLevelWindow
{
	RageDisplay::VideoModeParams CurrentParams;
public:
	LowLevelWindow_Null() {};
	~LowLevelWindow_Null() {};
	void *GetProcAddress(CString s) { return NULL; };
	bool TryVideoMode( RageDisplay::VideoModeParams p, bool &bNewDeviceOut ) { return false; };
	void SwapBuffers() {};
	void Update(float fDeltaTime) {};

	RageDisplay::VideoModeParams GetVideoModeParams() const { return CurrentParams; }
};
#undef ARCH_LOW_LEVEL_WINDOW
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_Null

#endif
