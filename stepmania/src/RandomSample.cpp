#include "global.h"
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
	if( GetExtension(sFilePath) == "" )
		return LoadSoundDir( sFilePath, iMaxToLoad );
	else
		return LoadSound( sFilePath );
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
	if(IsADirectory(sDir) && sDir[sDir.GetLength()-1] != "/" )
		sDir += "/";
#else
	// make sure there's a slash at the end of this path
	if( sDir.Right(1) != "/" )
		sDir += "/";
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

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
