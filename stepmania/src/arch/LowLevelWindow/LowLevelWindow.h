#ifndef LOW_LEVEL_WINDOW_H
#define LOW_LEVEL_WINDOW_H

/* This handles low-level operations that OGL 1.x doesn't give us.  Normally,
 * we use SDL.  Note that not all SDL operations go here; however, nothing
 * outside of this can assume that SDL has VIDEO initialized. */

#include "RageDisplay.h" // for RageDisplay::VideoModeParams

class LowLevelWindow
{
public:
	virtual ~LowLevelWindow() { }

	virtual void *GetProcAddress(CString s) = 0;

	// Return "" if mode change was successful, otherwise an error message.
	// bNewDeviceOut is set true if a new device was created and textures
	// need to be reloaded.
	virtual CString TryVideoMode( RageDisplay::VideoModeParams p, bool &bNewDeviceOut ) = 0;

	virtual void SwapBuffers() = 0;
	virtual void Update(float fDeltaTime) { }

	virtual RageDisplay::VideoModeParams GetVideoModeParams() const = 0;
};

#endif
