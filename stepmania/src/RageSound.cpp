#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageSound

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageSound.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"


RageSound*		SOUND	= NULL;

#if 0
RageSound::RageSound( HWND hWnd ) { }
RageSound::~RageSound() { }
void RageSound::PlayOnceStreamed( CString sPath ) { }
void RageSound::PlayOnceStreamedFromDir( CString sDir ) { }
#else

#pragma comment(lib, "bass/bass.lib") 

#include "bass/bass.h"



RageSound::RageSound( HWND hWnd )
{
	LOG->Trace( "RageSound::RageSound()" );

	// save the HWND
	if( !hWnd )
		throw RageException( "RageSound called with NULL hWnd." );
	m_hWndApp = hWnd;

	if( BASS_GetVersion() != MAKELONG(1,6) )
		throw RageException( "BASS version 1.6 DLL could not be loaded.  Verify that Bass.dll exists in the program directory.");

	if( !BASS_Init( -1, 44100, BASS_DEVICE_LEAVEVOL|BASS_DEVICE_LATENCY, m_hWndApp ) )
	{
		throw RageException( 
			"There was an error while initializing your sound card.\n\n"
			"The most likely cause of this problem is that you do not have a sound card\n"
			"installed, or that you have not yet installed a driver for your sound card.\n"
			"Before running this program again, please verify that your sound card is\n"
			"is working in other applications."
		);
	}

	BASS_Start();

	ZeroMemory( &m_info, sizeof(m_info) );
	m_info.size = sizeof(m_info);
	BASS_GetInfo( &m_info );


	LOG->Trace( 
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
	LOG->Trace( "RageSound::~RageSound()" );

	BASS_Stop();
	BASS_Free();
}

void RageSound::PlayOnceStreamed( CString sPath )
{
	HSTREAM hStream = BASS_StreamCreateFile( FALSE, const_cast<char*>((const char *)sPath), 0, 0, BASS_STREAM_AUTOFREE );
	if( hStream == NULL )
		throw RageException( "RageSound: Error creating stream." );

	if( FALSE == BASS_StreamPlay( hStream, FALSE, 0 ) )
		throw RageException( "RageSound: Error playing a sound stream." );

	// this stream will free itself when stopped 	
}

void RageSound::PlayOnceStreamedFromDir( CString sDir )
{
	if( sDir == "" )
		return;

	// make sure there's a backslash at the end of this path
	if( sDir[sDir.GetLength()-1] != '\\' )
		sDir += "\\";

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );

	if( arraySoundFiles.GetSize() != 0 )
	{
		int index = rand() % arraySoundFiles.GetSize();
		PlayOnceStreamed( sDir + arraySoundFiles[index] );
	}
}
#endif
