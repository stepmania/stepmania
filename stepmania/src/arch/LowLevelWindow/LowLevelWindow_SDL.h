#ifndef LOW_LEVEL_WINDOW_SDL_H
#define LOW_LEVEL_WINDOW_SDL_H

#include "LowLevelWindow.h"

class LowLevelWindow_SDL: public LowLevelWindow
{
	RageDisplay::VideoModeParams CurrentParams;

public:
	LowLevelWindow_SDL();
	~LowLevelWindow_SDL();
	void *GetProcAddress(CString s);
	bool TryVideoMode( RageDisplay::VideoModeParams p, bool &bNewDeviceOut );
	void SwapBuffers();
	void Update(float fDeltaTime);

	RageDisplay::VideoModeParams GetVideoModeParams() const { return CurrentParams; }
};
#undef ARCH_LOW_LEVEL_WINDOW
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_SDL

#endif
