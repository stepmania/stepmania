/* ScreenStage - Shows the stage number. */

#ifndef SCREEN_STAGE_H
#define SCREEN_STAGE_H

#include "Screen.h"
#include "Transition.h"
#include "Sprite.h"
#include "Character.h"
#include "BitmapText.h"
#include "Banner.h"

class ScreenStage : public Screen
{
public:
	ScreenStage( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void Update( float fDeltaTime );
	virtual void MenuBack( PlayerNumber pn );

private:
	Transition	m_In, m_Out, m_Back;
	BGAnimation				m_Background;
	BGAnimation				m_Overlay; // overlays all elements except bitmaptexts
	Banner			m_Banner;
	BitmapText		m_SongTitle;
	BitmapText		m_Artist;

	// elements that cannot be created with BGAnimation
	Sprite m_sprCharacterIcon[NUM_PLAYERS];

	bool			m_bZeroDeltaOnNextUpdate;
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
