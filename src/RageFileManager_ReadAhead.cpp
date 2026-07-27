#include "global.h"
#include "RageFileManager_ReadAhead.h"
#include "RageThreads.h"
#include "RageLog.h"

#include <cerrno>
#include <cstddef>
#include <vector>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif
#if defined(WIN32)
#include <io.h>
#endif

#if defined(HAVE_POSIX_FADVISE)

void RageFileManagerReadAhead::Init() { }
void RageFileManagerReadAhead::Shutdown() { }
void RageFileManagerReadAhead::ReadAhead( RageFileBasic *pFile, int iBytes )
{
	int iFD = pFile->GetFD();
	if( iFD == -1 )
		return;

	int iStart = lseek( iFD, 0, SEEK_CUR );
	posix_fadvise( iFD, iStart, iBytes, POSIX_FADV_WILLNEED );
}

void RageFileManagerReadAhead::DiscardCache( RageFileBasic *pFile, int iRelativePosition, int iBytes )
{
	int iFD = pFile->GetFD();
	if( iFD == -1 )
		return;

	int iStart = lseek( iFD, 0, SEEK_CUR );
	iStart += iRelativePosition;
	if( iStart < 0 )
	{
		iBytes -= iStart;
		iStart = 0;
	}

	if( iBytes == 0 )
		return;

	posix_fadvise( iFD, iStart, iBytes, POSIX_FADV_DONTNEED );
}

#else
#if 0
/* This doesn't currently work, because dup() locks the file position of the new FD with the old
 * (which makes no sense at all), so it's not easy to read from a copy of the FD without
 * interfering with the original. */
class RageFileReadAheadThread
{
public:
	RageFileReadAheadThread( int iFD, int iFrom, int iBytes )
	{
		m_iFD = iFD;
		m_iFrom = iFrom;
		m_iBytes = iBytes;
		m_bFinished = false;

		m_WorkerThread.SetName( "Read-ahead thread" );
		m_WorkerThread.Create( StartWorkerMain, this );
	}

	~RageFileReadAheadThread()
	{
		close( m_iFD );
		m_WorkerThread.Wait();
	}
	bool IsFinished() const { return m_bFinished; }

private:
	static int StartWorkerMain( void *pThis )
	{
		((RageFileReadAheadThread *) (pThis))->WorkerMain();
		return 0;
	}

	void WorkerMain()
	{
		int iFDCopy = dup( m_iFD );
		if( iFDCopy != -1 )
		{
			char [] buf = new char[m_iBytes];
			lseek( iFDCopy, m_iFrom, SEEK_SET );
			read( iFDCopy, buf, m_iBytes );
			delete [] buf;

			close( iFDCopy );
			LOG->Trace("read");
		}

		m_bFinished = true;
	}

	RageThread m_WorkerThread;

	bool m_bFinished;
	int m_iFD;
	int m_iFrom;
	int m_iBytes;
};


static std::vector<RageFileReadAheadThread *> g_apReadAheads;

void RageFileManagerReadAhead::Init() { }
void RageFileManagerReadAhead::Shutdown()
{
	for( std::size_t i = 0; i < g_apReadAheads.size(); ++i )
		delete g_apReadAheads[i];
	g_apReadAheads.clear();
}

/* This emulates posix_fadvise(POSIX_FADV_WILLNEED), to force data into cache and hint the
 * kernel to start read-ahead from that point. */
void RageFileManagerReadAhead::ReadAhead( RageFileBasic *pFile, int iBytes )
{
	int iFD = pFile->GetFD();
	if( iFD == -1 )
		return;

	iFD = dup( iFD );
	if( iFD == -1 )
	{
		LOG->Trace( "error: dup(): %s", strerror(errno) );
		return;
	}

	int iStart = lseek( iFD, 0, SEEK_CUR );

	RageFileReadAheadThread *pReadAhead = new RageFileReadAheadThread( iFD, iStart, iBytes );
	g_apReadAheads.push_back( pReadAhead );

	for( std::size_t i = 0; i < g_apReadAheads.size(); ++i )
	{
		if( g_apReadAheads[i]->IsFinished() )
		{
			LOG->Trace("cleaned");
			delete g_apReadAheads[i];
			g_apReadAheads.erase( g_apReadAheads.begin()+i, g_apReadAheads.begin()+i+1 );
			--i;
		}
	}
}
void RageFileManagerReadAhead::DiscardCache( RageFileBasic *pFile, int iRelativePosition, int iBytes ) { }
#else
void RageFileManagerReadAhead::Init() { }
void RageFileManagerReadAhead::Shutdown() { }
void RageFileManagerReadAhead::ReadAhead( RageFileBasic *pFile, int iBytes ) { }
void RageFileManagerReadAhead::DiscardCache( RageFileBasic *pFile, int iRelativePosition, int iBytes ) { }
#endif
#endif

void RageFileManagerReadAhead::CacheHintStreaming( RageFileBasic *pFile )
{
#if defined(HAVE_POSIX_FADVISE)
	/* This guesses at the actual size of the file on disk, which may be smaller if this file is compressed.
	 * Since this is usually used on music and video files, it generally shouldn't be. */
	int iFD = pFile->GetFD();
	if( iFD == -1 )
		return;
	int iPos = pFile->Tell();
	int iFrom = lseek( iFD, 0, SEEK_CUR );
	int iBytes = pFile->GetFileSize() - iPos;
	posix_fadvise( iFD, iFrom, iBytes, POSIX_FADV_SEQUENTIAL );
#endif
}


/*
 * Copyright (c) 2010 Glenn Maynard
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
