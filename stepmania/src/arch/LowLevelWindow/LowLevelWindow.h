#ifndef LOW_LEVEL_WINDOW_H
#define LOW_LEVEL_WINDOW_H

/* This handles low-level operations that OGL 1.x doesn't give us.  Normally,
 * we use SDL.  Note that not all SDL operations go here; however, nothing
 * outside of this can assume that SDL has VIDEO initialized. */
class LowLevelWindow
{
public:
	virtual ~LowLevelWindow() { }

	virtual void *GetProcAddress(CString s) = 0;

	/* Set the video mode as close to the requested settings as possible.  They're
	 * hints only; it's better to get the wrong mode than to bail out. */
	virtual bool SetVideoMode( bool windowed, int width, int height, int bpp, int rate, bool vsync, CString sWindowTitle, CString sIconFile ) = 0;

	virtual void SwapBuffers() = 0;
	virtual void Update(float fDeltaTime) { }

	virtual bool IsWindowed() const = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual int GetBPP() const = 0;
};

#endif
