#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RandomSample.h

 Desc: Holds multiple sounds samples and can play a random sound easily.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/



#include "RandomSample.h"
#include "RageUtil.h"
#include "RageLog.h"



RandomSample::RandomSample()
{
	m_iIndexLastPlayed = -1;
}

RandomSample::~RandomSample()
{
	for( unsigned i=0; i<m_pSamples.size(); i++ )
		SAFE_DELETE( m_pSamples[i] );
}


bool RandomSample::LoadSoundDir( CString sDir )
{
	if( sDir == "" )
		return true;

	// make sure there's a backslash at the end of this path
	if( sDir[sDir.GetLength()-1] != '\\' )
		sDir += "\\";

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );

	for( unsigned i=0; i<arraySoundFiles.size(); i++ )
		LoadSound( sDir + arraySoundFiles[i] );

	return true;
}
	
bool RandomSample::LoadSound( CString sSoundFilePath )
{
	LOG->Trace( "RandomSample::LoadSound( %s )", sSoundFilePath.GetString() );

	RageSoundSample* pSS = new RageSoundSample;
	pSS->Load( sSoundFilePath );


	m_pSamples.Add( pSS );
	
	return true;
}

void RandomSample::PlayRandom()
{
	// play one of the samples
	if( m_pSamples.empty() )
	{
		LOG->Trace( "WARNING:  Tried to play a RandomSample that has 0 sounds loaded." );
		return;
	}

	int iIndexToPlay = 0;
	for( int i=0; i<5; i++ )
	{
		iIndexToPlay = rand() % m_pSamples.size();
		if( iIndexToPlay != m_iIndexLastPlayed )
			break;
	}

	m_pSamples[iIndexToPlay]->Play();
	m_iIndexLastPlayed = iIndexToPlay;
}

void RandomSample::Pause()
{
	m_pSamples[m_iIndexLastPlayed]->Pause();
}

void RandomSample::Stop()
{
	if( m_iIndexLastPlayed == -1 )	// nothing is currently playing
		return;

	m_pSamples[m_iIndexLastPlayed]->Stop();
}
