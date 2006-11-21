#ifndef SCREEN_SET_TIME_H
#define SCREEN_SET_TIME_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "RageSound.h"

enum SetTimeSelection
{ 
	year, 
	month, 
	day, 
	hour, 
	minute, 
	second, 
	NUM_SetTimeSelection,
	SetTimeSelection_Invalid
};

class ScreenSetTime : public ScreenWithMenuElements
{
public:
	virtual void Init();

	virtual void Update( float fDelta );
	virtual void Input( const InputEventPlus &input );

	virtual void MenuUp( const InputEventPlus &input );
	virtual void MenuDown( const InputEventPlus &input );
	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	virtual void MenuStart( const InputEventPlus &input );
	virtual void MenuSelect( const InputEventPlus &input );
	virtual void MenuBack( const InputEventPlus &input );

private:
	SetTimeSelection m_Selection;
	time_t m_TimeOffset;
	void ChangeSelection( int iDirection );
	void ChangeValue( int iDirection );

	RageSound	m_soundChangeValue;
	RageSound	m_soundChangeSelection;
	BitmapText	m_textTitle[NUM_SetTimeSelection];
	BitmapText	m_textValue[NUM_SetTimeSelection];
	BitmapText	m_textDayOfWeek;
};

#endif

/*
 * (c) 2004 Chris Danford
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
