/* Win32 helper - set up a window for OpenGL/D3D */
#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H

#include <windows.h>
struct VideoModeParams; // for VideoModeParams

namespace GraphicsWindow
{
	/* Set up, and create a hidden window.  This only needs to be called once. */
	void Initialize();

	/* Shut down completely. */
	void Shutdown();

	void SetVideoModeParams( const VideoModeParams &p );
	CString SetScreenMode( const VideoModeParams &p );
	void CreateGraphicsWindow( const VideoModeParams &p );
	void RecreateGraphicsWindow( const VideoModeParams &p );
	void DestroyGraphicsWindow();
	void ConfigureGraphicsWindow( const VideoModeParams &p );

	LRESULT CALLBACK GraphicsWindow_WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void GetParams( VideoModeParams &paramsOut );
	HDC GetHDC();
	void Update();

	HWND GetHwnd();
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
