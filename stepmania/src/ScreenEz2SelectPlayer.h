/* ScreenEz2SelectPlayer - "Player Select Screen" for Ez2dancer */

#ifndef SCREENEZ2SELECTPLAYER_H
#define SCREENEZ2SELECTPLAYER_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "RageSound.h"

class ScreenEz2SelectPlayer : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual ~ScreenEz2SelectPlayer();
	
	/* Public Function Prototypes */
	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuDown( const InputEventPlus &input );
	virtual void MenuStart( const InputEventPlus &input );
	virtual void MenuBack( const InputEventPlus &input );
private:
	void TweenOffScreen();
	void TweenOnScreen();
	
	Sprite	m_sprJoinMessage[NUM_PLAYERS];
	Sprite	m_sprJoinFrame[NUM_PLAYERS];
};

#endif

/*
 * (c) 2002-2003 "Frieza"
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
