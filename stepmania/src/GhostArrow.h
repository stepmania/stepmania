/*
-----------------------------------------------------------------------------
 File: GhostArrow.h

 Desc: Class used to represent a color arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


class GhostArrow;

#ifndef _GhostArrow_H_
#define _GhostArrow_H_


#include "Sprite.h"
#include "Pattern.h"


class GhostArrow : public Sprite
{
public:
	GhostArrow();

	virtual void Update( float fDeltaTime );

	void  SetBeat( const float fSongBeat );
	void  Step( TapNoteScore score );

	float m_fVisibilityCountdown;
};

#endif 
