/*
-----------------------------------------------------------------------------
 File: RageSound.h

 Desc: Sound effects library (currently a wrapper around Bass Sound Library).

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _RAGESOUND_H_
#define _RAGESOUND_H_


#include "bass/bass.h"


#define NUM_STREAMS		16


class RageSound
{
public:
	RageSound( HWND hWnd );
	~RageSound();


private:
	HWND		m_hWndApp;	// this is set on GRAPHICS_Create()

};


typedef RageSound* LPRageSound;


extern LPRageSound			SOUND;	// global and accessable from anywhere in our program


#endif