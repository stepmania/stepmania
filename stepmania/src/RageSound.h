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
	
	HSAMPLE LoadSample( const CString sFileName );
	void	UnloadSample( HSAMPLE hSample );
	void	PlaySample( HSAMPLE hSample );
	void	StopSample( HSAMPLE hSample );
	float GetSampleLength( HSAMPLE hSample );
	float GetSamplePosition( HSAMPLE hSample );

	HSTREAM LoadStream( const CString sFileName );
	void	UnloadStream( HSTREAM hStream);
	void	PlayStream( HSTREAM hStream);
	void	PauseStream( HSTREAM hStream);
	void	StopStream( HSTREAM hStream);
	float GetStreamLength( HSTREAM hStream);
	float GetStreamPosition( HSTREAM hStream);
	BOOL	IsPlaying(DWORD handle);



private:
	HWND		m_hWndApp;	// this is set on GRAPHICS_Create()
//	HSAMPLE		m_hSample[NUM_STREAMS];
};


typedef RageSound* LPRageSound;


extern LPRageSound			SOUND;	// global and accessable from anywhere in our program


#endif