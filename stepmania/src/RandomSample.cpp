#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RandomSample.h

 Desc: Holds multiple sounds samples and can play a random sound easily.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/



#include "RandomSample.h"
#include "RageSound.h"
#include "RageUtil.h"
#include "RageLog.h"



RandomSample::RandomSample()
{
	m_iIndexLastPlayed = -1;
}


RandomSample::~RandomSample()
{
	UnloadAll();
}

bool RandomSample::Load( CString sFilePath, int iMaxToLoad )
{
	CString sDir, sFName, sExt;
	splitrelpath( sFilePath, sDir, sFName, sExt );

	sExt.MakeLower();

	if( sExt == "" )        return LoadSoundDir( sFilePath, iMaxToLoad );
	else                            return LoadSound( sFilePath );
}

void RandomSample::UnloadAll()
{
	for( unsigned i=0; i<m_pSamples.size(); i++ )
		delete m_pSamples[i];
	m_pSamples.clear();
}

bool RandomSample::LoadSoundDir( CString sDir, int iMaxToLoad )
{
	if( sDir == "" )
		return true;

#if 0
	/* (don't want to do this just yet) */
	/* If this is actually a directory, add a backslash to the filename,
	 * so we'll look for eg. themes\Default\sounds\sDir\*.mp3.  Otherwise,
	 * don't, so we'll look for all of the files starting with sDir,
	 * eg. themes\Default\sounds\sDir*.mp3. */
	if(IsADirectory(sDir) && sDir[sDir.GetLength()-1] != SLASH )
		sDir += SLASH;
#else
	// make sure there's a slash at the end of this path
	if( sDir.Right(1) != SLASH )
		sDir += SLASH;
#endif

	CStringArray arraySoundFiles;
	GetDirListing( sDir + "*.mp3", arraySoundFiles );
	GetDirListing( sDir + "*.ogg", arraySoundFiles );
	GetDirListing( sDir + "*.wav", arraySoundFiles );

	random_shuffle( arraySoundFiles.begin(), arraySoundFiles.end() );
	arraySoundFiles.resize( min( arraySoundFiles.size(), (unsigned)iMaxToLoad ) );

	for( unsigned i=0; i<arraySoundFiles.size(); i++ )
		LoadSound( sDir + arraySoundFiles[i] );

	return true;
}
	
bool RandomSample::LoadSound( CString sSoundFilePath )
{
	LOG->Trace( "RandomSample::LoadSound( %s )", sSoundFilePath.c_str() );

	RageSound *pSS = new RageSound;
	pSS->Load( sSoundFilePath );


	m_pSamples.push_back( pSS );
	
	return true;
}

void RandomSample::PlayRandom()
{
	// play one of the samples
	if( m_pSamples.empty() )
	{
//		LOG->Trace( "WARNING:  Tried to play a RandomSample that has 0 sounds loaded." );
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

void RandomSample::Stop()
{
	if( m_iIndexLastPlayed == -1 )	// nothing is currently playing
		return;

	m_pSamples[m_iIndexLastPlayed]->Stop();
}
