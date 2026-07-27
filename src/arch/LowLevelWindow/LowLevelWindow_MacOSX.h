#ifndef LOW_LEVEL_WINDOW_MACOSX_H
#define LOW_LEVEL_WINDOW_MACOSX_H

#include "LowLevelWindow.h"
#include "RageDisplay.h"

#include <cstdint>

#include <objc/objc.h>

typedef const struct __CFDictionary *CFDictionaryRef;
typedef std::uint32_t CGDirectDisplayID;

class LowLevelWindow_MacOSX : public LowLevelWindow
{
	VideoModeParams m_CurrentParams;
	id m_WindowDelegate;
	id m_Context;
	id m_BGContext;
	CFDictionaryRef m_CurrentDisplayMode;
	CGDirectDisplayID m_DisplayID;

public:
	LowLevelWindow_MacOSX();
	~LowLevelWindow_MacOSX();
	void *GetProcAddress( RString s );
	RString TryVideoMode( const VideoModeParams& p, bool& newDeviceOut );
	void GetDisplaySpecs( DisplaySpecs &specs ) const;

	void SwapBuffers();
	void Update();

	const ActualVideoModeParams GetActualVideoModeParams() const { return m_CurrentParams; }

	bool SupportsRenderToTexture() const { return true; }
	RenderTarget *CreateRenderTarget();

	bool SupportsThreadedRendering() { return m_BGContext; }
	void BeginConcurrentRendering();

private:
	void ShutDownFullScreen();
	int ChangeDisplayMode( const VideoModeParams& p );
	void SetActualParamsFromMode( CFDictionaryRef mode );
};

#ifdef ARCH_LOW_LEVEL_WINDOW
#error "More than one LowLevelWindow selected!"
#endif
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_MacOSX

#endif

/*
 * (c) 2005-2006, 2008 Steve Checkoway
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
