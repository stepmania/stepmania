#include "global.h"
#include "archutils/Xbox/GraphicsWindow.h"
#include "ProductInfo.h"
#include "RageLog.h"
#include "RageUtil.h"

static const CString g_sClassName = CString(PRODUCT_NAME) + " LowLevelWindow_Win32";

static VideoModeParams g_CurrentParams;
static bool g_bResolutionChanged = false;
static bool g_bHasFocus = true;
static bool g_bLastHasFocus = true;
static bool m_bWideWindowClass;

void GraphicsWindow::SetVideoModeParams( const VideoModeParams &params )
{
	g_CurrentParams = params;
}

const VideoModeParams &GraphicsWindow::GetParams()
{
	return g_CurrentParams;
}

void GraphicsWindow::Update()
{
	if( g_bResolutionChanged )
	{
		/* Let DISPLAY know that our resolution has changed. */
		DISPLAY->ResolutionChanged();
		g_bResolutionChanged = false;
	}
}

void GraphicsWindow::Initialize( bool bD3D ) { }
void GraphicsWindow::Shutdown() { }
CString GraphicsWindow::SetScreenMode( const VideoModeParams &p ) { return ""; }
void GraphicsWindow::CreateGraphicsWindow( const VideoModeParams &p ) { }
void GraphicsWindow::RecreateGraphicsWindow( const VideoModeParams &p ) { }
void GraphicsWindow::DestroyGraphicsWindow() { }
void GraphicsWindow::ConfigureGraphicsWindow( const VideoModeParams &p ) { }

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
