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
	bool SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync );
	void SwapBuffers();
	void ResolutionChanged(int width, int height) { CurrentWidth = width; CurrentHeight = height; }

	bool IsWindowed() const { return Windowed; }
	int GetWidth() const { return CurrentWidth; }
	int GetHeight() const { return CurrentHeight; }
	int GetBPP() const { return CurrentBPP; }
};
#undef ARCH_LOW_LEVEL_WINDOW
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_SDL

#endif
