#pragma once
/*
-----------------------------------------------------------------------------
 File: RageSound.h

 Desc: Sound effects library (currently a wrapper around Bass Sound Library).

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "bass/bass.h"


#define NUM_STREAMS		16


class RageSound
{
public:
	RageSound( HWND hWnd );
	~RageSound();

	float GetPlayLatency() { return m_info.latency / 1000.0f; };

	void PlayOnceStreamed( CString sPath );

private:
	HWND		m_hWndApp;	// this is set on GRAPHICS_Create()
	BASS_INFO	m_info;
};



extern RageSound*		SOUND;	// global and accessable from anywhere in our program
