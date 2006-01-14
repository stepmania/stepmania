#ifndef LOW_LEVEL_WINDOW_COCOA_H
#define LOW_LEVEL_WINDOW_COCOA_H

#include "LowLevelWindow.h"
#include "RageDisplay.h"

#ifndef __OBJC__
typedef void *id;
#endif

typedef const struct __CFDictionary *CFDictionaryRef;

class LowLevelWindow_Cocoa : public LowLevelWindow
{
	VideoModeParams mCurrentParams;
	id mWindow;
	id mView;
	id mFullScreenContext;
	bool mSharingContexts;
	CFDictionaryRef mCurrentDisplayMode;
	
public:
	LowLevelWindow_Cocoa();
	~LowLevelWindow_Cocoa();
	void *GetProcAddress( CString s );
	CString TryVideoMode( const VideoModeParams& p, bool& newDeviceOut );	
	void GetDisplayResolutions( DisplayResolutions &dr ) const;
	void SwapBuffers();
	
	VideoModeParams GetActualVideoModeParams() const { return mCurrentParams; }
private:
	void ShutDownFullScreen();
	int ChangeDisplayMode( const VideoModeParams& p );
	void SetActualParamsFromMode( CFDictionaryRef mode );
};

#ifdef ARCH_LOW_LEVEL_WINDOW
#error "More than one LowLevelWindow selected!"
#endif
#define ARCH_LOW_LEVEL_WINDOW LowLevelWindow_Cocoa

#endif

/*
 * (c) 2005-2006 Steve Checkoway
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
