#include "global.h"
#include "ScreenDimensions.h"
#include "PrefsManager.h"
#include "LuaManager.h"
#include "LuaFunctions.h"
#include "ThemeManager.h"
#include "ThemeMetric.h"

static ThemeMetric<float> THEME_SCREEN_WIDTH("Common","ScreenWidth");
static ThemeMetric<float> THEME_SCREEN_HEIGHT("Common","ScreenHeight");

//
// The theme's logical resolution specifies the minimum screen width
// and the minimum screen height with a 4:3 aspect ratio.  Scale just one 
// of the dimensions up to meet the requested aspect ratio.
//

/* 
 * The theme resolution isn't necessarily 4:3; a natively widescreen
 * theme would have eg. 16:9 or 16:10.
 *
 * Note that "aspect ratio" here always means DAR (display aspect ratio: the
 * aspect ratio of the physical display); we don't care about the PAR (pixel
 * aspect ratio: the aspect ratio of a pixel).
 */
#define THEME_NATIVE_ASPECT (THEME_SCREEN_WIDTH/THEME_SCREEN_HEIGHT)

float ScreenDimensions::GetScreenAspectRatio()
{
	float fAspect = PREFSMAN->m_fDisplayAspectRatio;
	if( fAspect == ASPECT_AUTO )
	{
		/* Most PC monitor resolutions have square pixels (PAR 1:1), so the DAR
		 * is simply width:height.  1280x1024 is an exception; treat it as 4:3,
		 * not 5:4. */
		if( PREFSMAN->m_iDisplayWidth == 1280 && PREFSMAN->m_iDisplayHeight == 1024 )
			fAspect = 4.0f/3.0f;
		else
			fAspect = PREFSMAN->m_iDisplayWidth / (float)PREFSMAN->m_iDisplayHeight;
	}
	return fAspect;
}

float ScreenDimensions::GetThemeAspectRatio()
{
	return THEME_NATIVE_ASPECT;
}

float ScreenDimensions::GetScreenWidth()
{
	float fAspect = GetScreenAspectRatio();
	float fScale = 1;
	if( fAspect > THEME_NATIVE_ASPECT )
		fScale = fAspect / THEME_NATIVE_ASPECT;
	ASSERT( fScale >= 1 );
	return THEME_SCREEN_WIDTH * fScale;
}

float ScreenDimensions::GetScreenHeight()
{
	float fAspect = GetScreenAspectRatio();
	float fScale = 1;
	if( fAspect < THEME_NATIVE_ASPECT )
		fScale = THEME_NATIVE_ASPECT / fAspect;
	ASSERT( fScale >= 1 );
	return THEME_SCREEN_HEIGHT * fScale;
}

void ScreenDimensions::ReloadMetricsAndUpdateLua()
{
	/* Important: explicitly refresh cached metrics that we use. */
	THEME_SCREEN_WIDTH.Read();
	THEME_SCREEN_HEIGHT.Read();

	LUA->SetGlobal( "SCREEN_WIDTH", (int) SCREEN_WIDTH );
	LUA->SetGlobal( "SCREEN_HEIGHT", (int) SCREEN_HEIGHT );
	LUA->SetGlobal( "SCREEN_LEFT", (int) SCREEN_LEFT );
	LUA->SetGlobal( "SCREEN_RIGHT", (int) SCREEN_RIGHT );
	LUA->SetGlobal( "SCREEN_TOP", (int) SCREEN_TOP );
	LUA->SetGlobal( "SCREEN_BOTTOM", (int) SCREEN_BOTTOM );
	LUA->SetGlobal( "SCREEN_CENTER_X", (int) SCREEN_CENTER_X );
	LUA->SetGlobal( "SCREEN_CENTER_Y", (int) SCREEN_CENTER_Y );
}

LuaFunction_NoArgs( GetScreenAspectRatio,	ScreenDimensions::GetScreenAspectRatio() );
LuaFunction_NoArgs( GetThemeAspectRatio,	ScreenDimensions::GetThemeAspectRatio() );


/*
 * (c) 2001-2002 Chris Danford
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
