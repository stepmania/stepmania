#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageSound.cpp

 Desc: Sound effects library (currently a wrapper around Bass Sound Library).

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "RageSound.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ErrorCatcher/ErrorCatcher.h"

#include "bass/bass.h"
#pragma comment(lib, "bass/bass.lib") 
 

RageSound*		SOUND	= NULL;


RageSound::RageSound( HWND hWnd )
{
	LOG->WriteLine( "RageSound::RageSound()" );
	// save the HWND
	if( !hWnd )
		FatalError( "RageSound called with NULL hWnd." );
	m_hWndApp = hWnd;

	if( BASS_GetVersion() != MAKELONG(1,5) )
		FatalError( "BASS version 1.5 DLL could not be loaded.  Verify that Bass.dll exists in the program directory.");

	if( !BASS_Init( -1, 44100, BASS_DEVICE_LEAVEVOL|BASS_DEVICE_LATENCY, m_hWndApp ) )
	{
		FatalError( 
			"There was an error while initializing your sound card.\n\n"
			"The most likely cause of this problem is that you do not have a sound card\n"
			"installed, or that you have not yet installed a driver for your sound card.\n"
			"Before running this program again, please verify that your sound card is\n"
			"is working in other Screens applications."
		);
	}

	BASS_Start();

	ZeroMemory( &m_info, sizeof(m_info) );
	m_info.size = sizeof(m_info);
	BASS_GetInfo( &m_info );


	LOG->WriteLine( 
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
	BASS_Stop();
	BASS_Free();
}

void RageSound::PlayOnceStreamed( CString sPath )
{
	HSTREAM hStream = BASS_StreamCreateFile( FALSE, (void*)((LPCTSTR)sPath), 0, 0, BASS_STREAM_AUTOFREE );
	if( hStream == NULL )
		FatalError( "RageSound: Error creating stream." );

	if( FALSE == BASS_StreamPlay( hStream, FALSE, 0 ) )
		FatalError( "RageSound: Error playing a sound stream." );

	// this stream will free itself when stopped 
}

void RageSound::PlayOnceStreamedFromDir( CString sDir )
{
	// make sure there's a backslash at the end of this path
	if( sDir[sDir.GetLength()-1] != '\\' )
		sDir += "\\";

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );

	int index = rand() % arraySoundFiles.GetSize();
	PlayOnceStreamed( sDir + arraySoundFiles[index] );
}
