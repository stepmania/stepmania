#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: RageSound.cpp
//
// Desc: Sound effects library (currently a wrapper around Bass Sound Library).
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------



#include "RageSound.h"
#include "RageUtil.h"

#include "bass/bass.h"
#pragma comment(lib, "bass/bass.lib") 


LPRageSound				SOUND	= NULL;


RageSound::RageSound( HWND hWnd )
{
	// save the HWND
	if( !hWnd )
		RageError( "RageSound called with NULL hWnd." );
	m_hWndApp = hWnd;

	if( BASS_GetVersion() != MAKELONG(1,1) )
		RageError( "BASS version 1.1 DLL could not be loaded.  Verify that Bass.dll exists in the program directory.");

	if( !BASS_Init( -1, 44100, BASS_DEVICE_NOSYNC, m_hWndApp ) )
		RageError( "BASS can't initialize sound device." );

	BASS_Start();
}

RageSound::~RageSound()
{
	BASS_Free();
}


HSAMPLE RageSound::LoadSound( const CString sFileName )
{
	RageLog( "RageSound::LoadSound( '%s' )", sFileName );

	HSAMPLE hSample = BASS_SampleLoad( FALSE, (void*)((LPCTSTR)sFileName), 0, 0, 0, 0 );
	if( hSample == NULL )
		RageError( ssprintf("RageSound::LoadSound: error loading %s (error code %d)", 
								               sFileName, BASS_ErrorGetCode()) );
	return hSample;
}

VOID RageSound::UnloadSound( HSAMPLE hSample )
{
	BASS_SampleFree( hSample );
}

VOID RageSound::Play( HSAMPLE hSample )
{
	if( NULL == BASS_SamplePlay( hSample ) )
		RageError( "There was an error playing a sound sample.  Are you sure this is a valid HSAMPLE?" );
}

VOID RageSound::Stop( HSAMPLE hSample )
{
	if( FALSE == BASS_SampleStop( hSample ) )
		RageError( "There was an error stopping a sound sample.  Are you sure this is a valid HSAMPLE?" );
}