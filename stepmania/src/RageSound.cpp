#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageSound.cpp

 Desc: Sound effects library (currently a wrapper around Bass Sound Library).

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


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

	if( BASS_GetVersion() != MAKELONG(1,3) )
		RageError( "BASS version 1.3 DLL could not be loaded.  Verify that Bass.dll exists in the program directory.");

	if( !BASS_Init( -1, 44100, BASS_DEVICE_LEAVEVOL, m_hWndApp ) )
	{
		MessageBox( NULL, 
					"There was an error while initializing your sound card.\n\n"
					"The most likely cause of this problem is that you do not have a sound card\n"
					"installed, or that you have not yet installed a driver for your sound card.\n"
					"Before running this program again, please verify that your sound card is\n"
					"is working in other Windows applications.", 
					"Sound error", 
					MB_ICONSTOP );
		RageError( "BASS can't initialize sound device." );
	}

	BASS_Start();
}

RageSound::~RageSound()
{
	BASS_Free();
}


