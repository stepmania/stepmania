#include "global.h"
#include "ScreenDimensions.h"
#include "PrefsManager.h"

ThemeMetric<float> THEME_SCREEN_WIDTH("Common","ScreenWidth");
ThemeMetric<float> THEME_SCREEN_HEIGHT("Common","ScreenHeight");

//
// The theme's logical resolution specifies the minimum screen width
// and the minimum screen height with a 4:3 aspect ratio.  Scale just one 
// of the dimensions up to meet the requested aspect ratio.
//

/* 
 * XXX: The theme resolution isn't necessarily 4:3; a natively widescreen
 * theme would have eg. 16:9 or 16:10.
 *
 * Note that "aspect ratio" here always means DAR; we don't care about the SAR.
 */
#define ASPECT_4_TO_3 (4/3.0f)

static float GetAspect()
{
	float fAspect = PREFSMAN->m_fDisplayAspectRatio;
	if( fAspect == ASPECT_AUTO )
		fAspect = PREFSMAN->m_iDisplayWidth / (float)PREFSMAN->m_iDisplayHeight;
	return fAspect;
}

float ScreenWidth()
{
	float fAspect = GetAspect();
	float fScale = 1;
	if( fAspect > ASPECT_4_TO_3 )
		fScale = fAspect / ASPECT_4_TO_3;
	ASSERT( fScale >= 1 );
	return THEME_SCREEN_WIDTH * fScale;
}

float ScreenHeight()
{
	float fAspect = GetAspect();
	float fScale = 1;
	if( fAspect < ASPECT_4_TO_3 )
		fScale = ASPECT_4_TO_3 / fAspect;
	ASSERT( fScale >= 1 );
	return THEME_SCREEN_HEIGHT * fScale;
}


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
