#include "global.h"
#include "archutils/Xbox/GraphicsWindow.h"
#include "ProductInfo.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageDisplay.h"

static const RString g_sClassName = RString(PRODUCT_ID) + " LowLevelWindow_Win32";

static VideoModeParams g_CurrentParams;
static bool g_bResolutionChanged = false;
static bool g_bHasFocus = true;
static bool g_bLastHasFocus = true;
static bool m_bWideWindowClass;
bool isPALSystem();
void setScreenResolutionValues();

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

void GraphicsWindow::Initialize( bool bD3D )
{
	/* We do the parameters initialisation here. No need to use the INI,
	   because if we changed resolutions, it wouldn't keep up. */

	g_CurrentParams.bpp = 32;
	g_CurrentParams.windowed=false;
	
	isPALSystem();
	setScreenResolutionValues();
}

bool isPALSystem()
{
	if(XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I)
	{
		g_CurrentParams.interlaced=true;

		/* Get supported video flags. */
		DWORD VideoFlags = XGetVideoFlags();

		/* Set pal60 if available. */
		if( VideoFlags & XC_VIDEO_FLAGS_PAL_60Hz )
			g_CurrentParams.rate = 60;
		else
			g_CurrentParams.rate = 50;

		g_CurrentParams.PAL = true;
		return true;
	}
	g_CurrentParams.rate = 60;
	g_CurrentParams.PAL = false;
	return false;
}
void setScreenResolutionValues()
{
	DWORD CurrentVideoFlags = XGetVideoFlags();
	int heightStandards;

	// Preventive definition. Changed only if needed.
	g_CurrentParams.interlaced=false;

	switch(isPALSystem()){
		case true:
			heightStandards = 576;
			break;
		case false:
			heightStandards = 480;
			g_CurrentParams.interlaced=false;
			break;
	}

	switch (CurrentVideoFlags){

		case XC_VIDEO_FLAGS_HDTV_480p:
			g_CurrentParams.width=720;
			g_CurrentParams.height=480;
			break;

		case XC_VIDEO_FLAGS_HDTV_720p:
			g_CurrentParams.width=1280;
			g_CurrentParams.height=720;
			break;

		case XC_VIDEO_FLAGS_HDTV_1080i:
			g_CurrentParams.width=1920;
			g_CurrentParams.height=1080;
			g_CurrentParams.interlaced=true;
			break;

		case XC_VIDEO_FLAGS_WIDESCREEN:
			g_CurrentParams.width=720;
			g_CurrentParams.height=heightStandards;
			break;

		case XC_VIDEO_FLAGS_LETTERBOX:
		default:
			g_CurrentParams.width=640;
			g_CurrentParams.height=heightStandards;
			break;
	}
}

void GraphicsWindow::Shutdown() { }
RString GraphicsWindow::SetScreenMode( const VideoModeParams &p ) { return ""; }
void GraphicsWindow::CreateGraphicsWindow( const VideoModeParams &p ) { }
void GraphicsWindow::RecreateGraphicsWindow( const VideoModeParams &p ) { }
void GraphicsWindow::DestroyGraphicsWindow() { }
void GraphicsWindow::ConfigureGraphicsWindow( const VideoModeParams &p ) { }

/*
 * (c) 2004 Glenn Maynard, Renaud Lepage
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
