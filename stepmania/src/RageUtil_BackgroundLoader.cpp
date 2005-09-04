#include "global.h"
#include "RageUtil_BackgroundLoader.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageThreads.h"


/* If we're on an OS with a good caching system, writing to our own cache will only
 * waste memory.  In that case, just read the file, to force it into system cache.
 * If we're on a system with an unreliable cache, read it into our own cache.  This
 * helps in Windows, where even reading directory entries off of a CD--even if they've
 * just been read--can cause long delays if the disk is in use. */
static const bool g_bWriteToCache = true;

static const bool g_bEnableBackgroundLoading = false;

BackgroundLoader::BackgroundLoader():
	m_StartSem( "BackgroundLoaderSem" ),
	m_Mutex( "BackgroundLoaderMutex" )
{
	if( !g_bEnableBackgroundLoading )
		return;

	m_sCachePathPrefix = ssprintf( "@mem/%p", this );

	m_bShutdownThread = false;
	m_sThreadIsActive = m_sThreadShouldAbort = false;
	m_LoadThread.SetName( "BackgroundLoader" );
	m_LoadThread.Create( LoadThread_Start, this );
}

static void DeleteEmptyDirectories( CString sDir )
{
	vector<CString> asNewDirs;
	GetDirListing( sDir + "/*", asNewDirs, false, true );
	for( unsigned i = 0; i < asNewDirs.size(); ++i )
	{
		ASSERT_M( IsADirectory(asNewDirs[i]), asNewDirs[i] );
		DeleteEmptyDirectories( asNewDirs[i] );
	}

	FILEMAN->Remove( sDir );
}

BackgroundLoader::~BackgroundLoader()
{
	if( !g_bEnableBackgroundLoading )
		return;

	Abort();

	m_bShutdownThread = true;
	m_StartSem.Post();
	m_LoadThread.Wait();

	/* Delete all leftover cached files. */
	map<CString,int>::iterator it;
	for( it = m_FinishedRequests.begin(); it != m_FinishedRequests.end(); ++it )
		FILEMAN->Remove( GetCachePath( it->first ) );

	/* m_sCachePathPrefix should be filled with several empty directories.  Delete
	 * them and m_sCachePathPrefix, so we don't leak them. */
	DeleteEmptyDirectories( m_sCachePathPrefix );
}

/* Pull a request out of m_CacheRequests. */
CString BackgroundLoader::GetRequest()
{
	if( !g_bEnableBackgroundLoading )
		return CString();

	LockMut( m_Mutex );
	if( !m_CacheRequests.size() )
		return CString();

	CString ret;
	ret = m_CacheRequests.front();
	m_CacheRequests.erase( m_CacheRequests.begin(), m_CacheRequests.begin()+1 );
	return ret;
}

CString BackgroundLoader::GetCachePath( CString sPath ) const
{
	return m_sCachePathPrefix + sPath;
}

void BackgroundLoader::LoadThread()
{
	while( !m_bShutdownThread )
	{
		/* Wait for a request.  It's normal for this to wait for a long time; don't
		 * fail on timeout. */
		m_StartSem.Wait( false );

		CString sFile = GetRequest();
		if( sFile.empty() )
			continue;

		{
			/* If the file already exists, short circuit. */
			LockMut( m_Mutex );
			map<CString,int>::iterator it;
			it = m_FinishedRequests.find( sFile );
			if( it != m_FinishedRequests.end() )
			{
				++it->second;
				LOG->Trace("XXX: request %s done loading (already done), cnt now %i", sFile.c_str(), m_FinishedRequests[sFile] );
				continue;
			}
		}

		m_sThreadIsActive = true;

		LOG->Trace("XXX: reading %s", sFile.c_str());

		CString sCachePath = GetCachePath( sFile );

		/* Open the file and read it. */
		RageFile src;
		if( src.Open(sFile) )
		{
			/* If we're writing to a file cache ... */
			RageFile dst;

			bool bWriteToCache = g_bWriteToCache;
			if( bWriteToCache )
				bWriteToCache = dst.Open( sCachePath, RageFile::WRITE );
			LOG->Trace("XXX: go on '%s' to '%s'", sFile.c_str(), sCachePath.c_str());
			
			char buf[1024*4];
			while( !m_sThreadShouldAbort && !src.AtEOF() )
			{
				int got = src.Read( buf, sizeof(buf) );
				if( got > 0 && bWriteToCache )
					dst.Write( buf, got );
			}
			if( bWriteToCache )
				dst.Close();

			LOG->Trace("XXX: done");
		}
		src.Close();

		LockMut( m_Mutex );
		if( !m_sThreadShouldAbort )
		{
			++m_FinishedRequests[sFile];
		LOG->Trace("XXX: request %s done loading, cnt now %i", sFile.c_str(), m_FinishedRequests[sFile] );
		}
		else
		{
			FILEMAN->Remove( sCachePath );

			LOG->Trace("XXX: request %s aborted", sFile.c_str() );
		}

		m_sThreadShouldAbort = false;
		m_sThreadIsActive = false;
	}
}

void BackgroundLoader::CacheFile( const CString &sFile )
{
	if( !g_bEnableBackgroundLoading )
		return;

	if( sFile == "" )
		return;

	LockMut( m_Mutex );
	m_CacheRequests.push_back( sFile );
	m_StartSem.Post();
}

bool BackgroundLoader::IsCacheFileFinished( const CString &sFile, CString &sActualPath )
{
	if( !g_bEnableBackgroundLoading )
	{
		sActualPath = sFile;
		return true;
	}

	LockMut( m_Mutex );

	if( sFile == "" )
	{
		sActualPath = "";
		return true;
	}

	map<CString,int>::iterator it;
	it = m_FinishedRequests.find( sFile );
	if( it == m_FinishedRequests.end() )
		return false;

	LOG->Trace("XXX: %s finished (%i)", sFile.c_str(), it->second);
	if( g_bWriteToCache )
		sActualPath = GetCachePath( sFile );
	else
		sActualPath = sFile;

	return true;
}

void BackgroundLoader::FinishedWithCachedFile( CString sFile )
{
	if( !g_bEnableBackgroundLoading )
		return;

	if( sFile == "" )
		return;

	map<CString,int>::iterator it;
	it = m_FinishedRequests.find( sFile );
	ASSERT_M( it != m_FinishedRequests.end(), sFile );

	--it->second;
	ASSERT_M( it->second >= 0, ssprintf("%i", it->second) );
	if( !it->second )
	{
		m_FinishedRequests.erase( it );
		FILEMAN->Remove( GetCachePath( sFile ) );
	}
}

void BackgroundLoader::Abort()
{
	if( !g_bEnableBackgroundLoading )
		return;

	LockMut( m_Mutex );

	/* Clear any pending requests. */
	while( !GetRequest().empty() )
		;

	/* Clear any previously finished requests. */
	while( m_FinishedRequests.size() )
		FinishedWithCachedFile( m_FinishedRequests.begin()->first );

	/* Tell the thread to abort any request it's handling now. */
	if( m_sThreadIsActive )
		m_sThreadShouldAbort = true;
}

/*
 * (c) 2004 Glenn Maynard
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
