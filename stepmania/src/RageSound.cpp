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

	if( !BASS_Init( -1, 44100, BASS_DEVICE_NOSYNC, m_hWndApp ) )
	{
		MessageBox( NULL, "There was an error while initializing your sound card.\n\n\
The most likely cause of this problem is that you do not have a sound card\n\
installed, or that you have not yet installed a driver for your sound card.\n\
Before running this program again, please verify that your sound card is\n\
is working in other Windows applications.", "Sound error", MB_ICONSTOP );
		RageError( "BASS can't initialize sound device." );
	}

	BASS_Start();
}

RageSound::~RageSound()
{
	BASS_Free();
}


// Sample stuff

HSAMPLE RageSound::LoadSample( const CString sFileName )
{
	RageLog( "RageSound::LoadSound( '%s' )", sFileName );

	HSAMPLE hSample = BASS_SampleLoad( FALSE, (void*)((LPCTSTR)sFileName), 0, 0, 0, 0 );
	if( hSample == NULL )
		RageError( ssprintf("RageSound::LoadSound: error loading %s (error code %d)", 
								               sFileName, BASS_ErrorGetCode()) );
	
	return hSample;
}

void RageSound::UnloadSample( HSAMPLE hSample )
{
	BASS_SampleFree( hSample );
}

void RageSound::PlaySample( HSAMPLE hSample )
{
	HCHANNEL hChannel = BASS_SamplePlay( hSample );
	if( hChannel == NULL )
		RageError( "There was an error playing a sound sample.  Are you sure this is a valid HSAMPLE?" );
	
	DWORD dwPosition = BASS_ChannelGetPosition( hChannel );
	RageLog( "First BASS_ChannelGetPosition: %d", dwPosition );
}

void RageSound::StopSample( HSAMPLE hSample )
{
	if( FALSE == BASS_SampleStop( hSample ) )
		RageError( "There was an error stopping a sound sample.  Are you sure this is a valid HSAMPLE?" );
}

float RageSound::GetSampleLength( HSAMPLE hSample )
{
	return 90;
}

float RageSound::GetSamplePosition( HSAMPLE hSample )
{
	DWORD dwPosition = BASS_ChannelGetPosition( hSample );
	RageLog( "BASS_ChannelGetPosition: %d", dwPosition );
	float fSeconds = BASS_ChannelBytes2Seconds( hSample, dwPosition );
//	fSeconds += 0.05f;		// fudge number.  Should use a BASS_SYNC to sync the music.
	return fSeconds;
}


// Stream stuff

HSTREAM RageSound::LoadStream( const CString sFileName )
{
	RageLog( "RageSound::LoadSound( '%s' )", sFileName );

	HSAMPLE hStream = BASS_StreamCreateFile( FALSE, (void*)((LPCTSTR)sFileName), 0, 0, 0 );
	if( hStream == NULL )
		RageError( ssprintf("RageSound::LoadSound: error loading %s (error code %d)", 
								               sFileName, BASS_ErrorGetCode()) );
	return hStream;
}

void RageSound::UnloadStream( HSTREAM hStream )
{
	BASS_StreamFree( hStream );
}

void RageSound::PlayStream( HSTREAM hStream )
{
	if( FALSE == BASS_StreamPlay( hStream, FALSE, 0 ) )
		RageError( "There was an error playing a sound stream.  Are you sure this is a valid HSTREAM?" );
}

void RageSound::PauseStream( HSTREAM hStream )
{
	if( FALSE == BASS_ChannelPause( hStream ) )
		RageError( "There was an error pausing a sound stream.  Are you sure this is a valid HSTREAM?" );
}

void RageSound::StopStream( HSTREAM hStream )
{
	if( FALSE == BASS_ChannelStop( hStream ) )
		RageError( "There was an error stopping a sound stream.  Are you sure this is a valid HSTREAM?" );
}

float RageSound::GetStreamLength( HSTREAM hStream )
{
	DWORD dwLength = BASS_StreamGetLength(hStream); 
	float fSeconds = BASS_ChannelBytes2Seconds( hStream, dwLength );
	return fSeconds;
}

float RageSound::GetStreamPosition( HSTREAM hStream )
{
	DWORD dwPosition = BASS_ChannelGetPosition( hStream );
	float fSeconds = BASS_ChannelBytes2Seconds( hStream, dwPosition );
	//fSeconds += 0.05f;		// fudge number.  Should use a BASS_SYNC to sync the music.
	return fSeconds;
}

BOOL RageSound::IsPlaying( DWORD handle )
{
	BOOL bIsPlaying = (0 == BASS_ChannelIsActive(handle) );
	return bIsPlaying;
}