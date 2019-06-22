#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H

#include <windows.h>
#include "DisplaySpec.h"
class VideoModeParams;
class DisplayResolution;

/** @brief Sets up a window for OpenGL/D3D. */
namespace GraphicsWindow
{
	/** @brief Set up, and create a hidden window.
	 *
	 * This only needs to be called once. */
	void Initialize( bool bD3D );

	/** @brief Shut down completely. */
	void Shutdown();

	/** @brief Set the display mode.
	 *
	 * p will not be second-guessed, except to try disabling the refresh rate setting. */
	RString SetScreenMode( const VideoModeParams &p );

	/** @brief Create the window.
	 *
	 * This also updates VideoModeParams (returned by GetParams). */
	void CreateGraphicsWindow( const VideoModeParams &p, bool bForceRecreateWindow = false );
	void DestroyGraphicsWindow();

	void GetDisplaySpecs( DisplaySpecs &out );

	const VideoModeParams &GetParams();
	HDC GetHDC();
	void Update();

	HWND GetHwnd();

	//dwm functions for vista+
	static HINSTANCE hInstanceDwmapi = nullptr;
	static HRESULT(WINAPI* PFN_DwmIsCompositionEnabled)(BOOL*);
	static HRESULT (WINAPI* PFN_DwmFlush)(VOID);
};

#endif

/*
 * (c) 2004 Glenn Maynard
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
