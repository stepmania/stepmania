/*
-----------------------------------------------------------------------------
 File: HoldGhostArrow.h

 Desc: Class used to represent a color arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


class HoldGhostArrow;

#ifndef _HoldGhostArrow_H_
#define _HoldGhostArrow_H_


#include "Sprite.h"
#include "NoteMetadata.h"


class HoldGhostArrow : public Sprite
{
public:
	HoldGhostArrow();

	virtual void Update( float fDeltaTime );

	void  SetBeat( const float fSongBeat );
	void  Step();

	bool m_bWasSteppedOnLastFrame;
	float m_fHeatLevel;	// effects brightness of electricity - between 0 and 1
};

#endif 
