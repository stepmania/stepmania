#ifndef LOW_LEVEL_WINDOW_SDL_H
#define LOW_LEVEL_WINDOW_SDL_H

#include "LowLevelWindow.h"

class LowLevelWindow_SDL: public LowLevelWindow
{
	bool Windowed;
	int CurrentHeight, CurrentWidth, CurrentBPP;

public:
	LowLevelWindow_SDL();
	~LowLevelWindow_SDL();
	void *GetProcAddress(CString s);
	bool SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync, CString sWindowTitle, CString sIconFile );
	void SwapBuffers();
	void Update(float fDeltaTime);

	bool IsWindowed() const { return Windowed; }
	int GetWidth() const { return CurrentWidth; }
	int GetHeight() const { return CurrentHeight; }
	int GetBPP() const { return CurrentBPP; }
};
#undef ARCH_LOW_LEVEL_WINDOW
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_SDL

#endif
