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
#include "RageHelper.h"

#include "bass/bass.h"
#pragma comment(lib, "bass/bass.lib") 


RageSound*		SOUND	= NULL;


RageSound::RageSound( HWND hWnd )
{
	HELPER.Log( "RageSound::RageSound()" );
	// save the HWND
	if( !hWnd )
		HELPER.FatalError( "RageSound called with NULL hWnd." );
	m_hWndApp = hWnd;

	if( BASS_GetVersion() != MAKELONG(1,3) )
		HELPER.FatalError( "BASS version 1.3 DLL could not be loaded.  Verify that Bass.dll exists in the program directory.");

	if( !BASS_Init( -1, 44100, BASS_DEVICE_LEAVEVOL|BASS_DEVICE_LATENCY, m_hWndApp ) )
	{
		HELPER.FatalError( 
			"There was an error while initializing your sound card.\n\n"
			"The most likely cause of this problem is that you do not have a sound card\n"
			"installed, or that you have not yet installed a driver for your sound card.\n"
			"Before running this program again, please verify that your sound card is\n"
			"is working in other Windows applications."
		);
	}

	BASS_Start();

	ZeroMemory( &m_info, sizeof(m_info) );
	m_info.size = sizeof(m_info);
	BASS_GetInfo( &m_info );


	HELPER.Log( 
		"Sound card info:\n"
		" - play latency is %u ms\n"
		" - total device hardware memory is %u bytes\n"
		" - free device hardware memory is %u bytes\n"
		" - number of free sample slots in the hardware is %u\n"
		" - number of free 3D sample slots in the hardware is %u\n"
		" - min sample rate supported by the hardware is %u\n"
		" - max sample rate supported by the hardware is %u",
		m_info.latency,
		m_info.hwsize,
		m_info.hwfree,
		m_info.freesam,
		m_info.free3d,
		m_info.minrate,
		m_info.maxrate
		);
}

RageSound::~RageSound()
{
	BASS_Free();
}


