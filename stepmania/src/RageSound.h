#pragma once
/*
-----------------------------------------------------------------------------
 File: RageSound.h

 Desc: Sound effects library (currently a wrapper around Bass Sound Library).

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "bass/bass.h"


#define NUM_STREAMS		16


class RageSound
{
public:
	RageSound();
	~RageSound();

	float GetPlayLatency() { return m_info.latency / 1000.0f; };	// latency between when Play() is called and sound starts coming out

	void PlayOnceStreamed( CString sPath );
	void PlayOnceStreamedFromDir( CString sDir );

private:
	BASS_INFO	m_info;
};



extern RageSound*		SOUND;	// global and accessable from anywhere in our program
