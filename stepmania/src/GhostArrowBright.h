/*
-----------------------------------------------------------------------------
 File: GhostArrowBright.h

 Desc: Class used to represent a color arrow on the screen.

 Copyright (c) 2001 Ben Norstrom.  All rights reserved.
-----------------------------------------------------------------------------
*/


class GhostArrowBright;

#ifndef _GhostArrowBright_H_
#define _GhostArrowBright_H_


#include "Sprite.h"
#include "Steps.h"


class GhostArrowBright : public Sprite
{
public:
	GhostArrowBright();

	void  SetBeat( const float fSongBeat );
	void  Step( TapStepScore score );

	float m_fVisibilityCountdown;
};

#endif 
