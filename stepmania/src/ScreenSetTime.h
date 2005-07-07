#ifndef SCREEN_SET_TIME_H
#define SCREEN_SET_TIME_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"

enum SetTimeSelection
{ 
	year, 
	month, 
	day, 
	hour, 
	minute, 
	second, 
	NUM_SET_TIME_SELECTIONS 
};

class ScreenSetTime : public ScreenWithMenuElements
{
public:
	ScreenSetTime( CString sName );
	virtual void Init();

	virtual void Update( float fDelta );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	virtual void MenuUp( PlayerNumber pn );
	virtual void MenuDown( PlayerNumber pn );
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuSelect( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

private:
	SetTimeSelection m_Selection;
	time_t m_TimeOffset;
	void ChangeSelection( int iDirection );
	void ChangeValue( int iDirection );

	BitmapText	m_textTitle[NUM_SET_TIME_SELECTIONS];
	BitmapText	m_textValue[NUM_SET_TIME_SELECTIONS];
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
