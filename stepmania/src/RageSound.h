//-----------------------------------------------------------------------------
// File: RageSound.h
//
// Desc: Sound effects library (currently a wrapper around Bass Sound Library).
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _RAGESOUND_H_
#define _RAGESOUND_H_


#include "bass/bass.h"


#define NUM_STREAMS		16

#define EFFECT_CHANNEL_1	0
#define EFFECT_CHANNEL_2	1
#define EFFECT_CHANNEL_3	2
#define EFFECT_CHANNEL_4	3
#define EFFECT_CHANNEL_5	4
#define ANOUNCER_CHANNEL	13
#define CROWD_CHANNEL		14
#define MUSIC_CHANNEL		15


class RageSound
{
public:
	RageSound( HWND hWnd );
	~RageSound();
	
	HSAMPLE LoadSound( const CString sFileName );
	VOID	UnloadSound( HSAMPLE hSample );
	VOID	Play( HSAMPLE hSample );
	VOID	Stop( HSAMPLE hSample );

private:
	HWND		m_hWndApp;	// this is set on GRAPHICS_Create()
//	HSAMPLE		m_hSample[NUM_STREAMS];
};


typedef RageSound* LPRageSound;


extern LPRageSound			SOUND;	// global and accessable from anywhere in our program


#endif