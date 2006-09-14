/* ScreenBookkeeping - Show coin drop stats. */

#ifndef SCREEN_BOOKKEEPING_H
#define SCREEN_BOOKKEEPING_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"

const int NUM_BOOKKEEPING_COLS = 4;

class ScreenBookkeeping : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual ~ScreenBookkeeping();

	virtual void Update( float fDelta );
	virtual void Input( const InputEventPlus &input );

	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	virtual void MenuStart( const InputEventPlus &input );
	virtual void MenuBack( const InputEventPlus &input );
	virtual void MenuCoin( const InputEventPlus &input );

private:
	enum View { VIEW_LAST_DAYS, VIEW_LAST_WEEKS, VIEW_DAY_OF_WEEK, VIEW_HOUR_OF_DAY, NUM_VIEWS };
	
	void ChangeView( View newView );

	View m_View;
	BitmapText	m_textAllTime;
	BitmapText	m_textTitle;
	BitmapText	m_textData[NUM_BOOKKEEPING_COLS];
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
