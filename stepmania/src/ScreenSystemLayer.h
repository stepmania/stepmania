/* ScreenSystemLayer -  
 * This screen is drawn on top of everything else, and receives updates,
 * but not input.
 */

#ifndef ScreenSystemLayer_H
#define ScreenSystemLayer_H

#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"

const int NUM_SKIPS_TO_SHOW = 5;

class ScreenSystemLayer : public Screen
{
public:
	ScreenSystemLayer();
	virtual void Init();
	void SystemMessage( const CString &sMessage );
	void SystemMessageNoAnimate( const CString &sMessage );
	void ReloadCreditsText();
	void RefreshCreditsMessages();
	void Update( float fDeltaTime );

private:
	BitmapText m_textStats;
	BitmapText m_textMessage;
	BitmapText m_textCredits[NUM_PLAYERS];
	BitmapText m_textTime;
	BitmapText m_textSkips[NUM_SKIPS_TO_SHOW];
	int m_LastSkip;
	Quad m_SkipBackground;

	RageTimer SkipTimer;
	void AddTimestampLine( const CString &txt, const RageColor &color );
	void UpdateTimestampAndSkips();
	CString GetCreditsMessage( PlayerNumber pn ) const;
};


#endif

/*
 * (c) 2001-2004 Chris Danford
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
