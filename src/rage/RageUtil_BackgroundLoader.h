/* BackgroundLoader - Loads files in a thread. */

#ifndef RAGE_UTIL_BACKGROUND_LOADER_H
#define RAGE_UTIL_BACKGROUND_LOADER_H

#include "RageThreads.h"
#include <map>

class RageFileDriverCached;

class BackgroundLoader
{
public:
	BackgroundLoader();

	/* Note that destruction of this object will wait for any existing requests
	 * to finish aborting before returning. */
	~BackgroundLoader();

	/* Read the file in a background thread.  Files will be read in the order requested. */
	void CacheFile( const RString &file );

	/* Return true if the requested CacheFile request has finished.  If true is returned,
	 * the cached file can be read using the path returned in sActualPath. */
	bool IsCacheFileFinished( const RString &sFile, RString &sActualPath );

	/* Call this when finished with a cached file, to release any resources. */
	void FinishedWithCachedFile( RString sFile );

	/* Abort all loads. */
	void Abort();

private:
	RageThread m_LoadThread;
	bool m_bShutdownThread;
	void LoadThread();
	static int LoadThread_Start( void *p ) { ((BackgroundLoader *) p)->LoadThread(); return 0; }

	RString GetRequest();

	RString GetCachePath( RString sPath ) const;
	RString m_sCachePathPrefix;

	RageSemaphore m_StartSem;

	/* Lock before accessing any of the rest of the object.  Don't keep this locked
	 * while doing expensive operations, like reading files. */
	RageMutex m_Mutex;

	vector<RString> m_CacheRequests;

	/* Filename to number of completed requests */
	map<RString,int> m_FinishedRequests;

	bool m_sThreadIsActive;
	bool m_sThreadShouldAbort;

	RageFileDriverCached *m_pDriver;
};

#endif

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
