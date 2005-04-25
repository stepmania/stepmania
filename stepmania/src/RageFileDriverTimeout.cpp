/*
 * This is a filesystem wrapper driver.  To use it, mount it on top of another filesystem at a
 * different mountpoint.  For example, mount the local path "d:/" as a normal directory to
 * /cdrom-native:
 *
 * FILEMAN->Mount( "dir", "d:/", "/cdrom-native" );
 *
 * and then mount a timeout filesystem on top of that:
 *
 * FILEMAN->Mount( "timeout", "/cdrom-native", "/cdrom-timeout" );
 *
 * and do all access to the device at /cdrom-timeout.
 *
 * A common problem with accessing devices, such as CDROMs or flaky pen drives, is that they
 * can take a very long time to fail under error conditions.  A single request may have a
 * timeout of 500ms, but a single operation may do many requests.  The system will usually
 * retry operations, and we want to allow it to do so, but we don't want the high-level
 * operation to take too long as a whole.
 *
 * There's no portable way to abort a running filesystem operation.  In Unix, you may be
 * able to use SIGALRM, which should cause a running operation to fail with EINTR, but
 * I don't trust that, and it's not portable.
 *
 * This driver abstracts all access to another driver, and enforces high-level timeouts.
 * Operations are run in a separate thread.  If an operation takes too long to complete,
 * the main thread will see it fail.  The operation in the thread will actually continue;
 * it will time out for real eventually.  To prevent accumulation of failed threads, if
 * an operation times out, we'll refuse all further access until all operations have
 * finished and exited.  (Load a separate driver for each device, so if one device fails,
 * others continue to function.)
 * 
 * All operations must run in the thread, including retrieving directory lists, Open()
 * and deleting file objects.  Read/write operations are copied through an intermediate
 * buffer, so we don't clobber stuff if the operation times out, the call returns and the
 * operation then completes.
 *
 * Unmounting the filesystem will wait for all timed-out operations to complete.
 *
 * This class is not threadsafe; do not access files on this filesystem from multiple
 * threads simultaneously.
 */

#include "global.h"
#include "RageFileDriverTimeout.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include "RageUtil_WorkerThread.h"
#include "RageLog.h"
#include <errno.h>

enum ThreadRequest
{
	REQ_OPEN,
	REQ_CLOSE,
	REQ_GET_FILE_SIZE,
	REQ_READ,
	REQ_WRITE,
	REQ_SEEK,
	REQ_FLUSH,
	REQ_COPY,
	REQ_POPULATE_FILE_SET,
	REQ_FLUSH_DIR_CACHE,
};

/* This is the class that does most of the work. */
class ThreadedFileWorker: public WorkerThread
{
public:
	ThreadedFileWorker( CString sPath );
	~ThreadedFileWorker();

	/* Threaded operations.  If a file operation times out, the caller loses all access
	 * to the file and should fail all future operations; this is because the thread
	 * is still trying to finish the operation.  The thread will clean up afterwards. */
	RageFileBasic *Open( const CString &sPath, int iMode, int &iErr );
	void Close( RageFileBasic *pFile );
	int GetFileSize( RageFileBasic *&pFile );
	int Seek( RageFileBasic *&pFile, int iPos, CString &sError );
	int Read( RageFileBasic *&pFile, void *pBuf, int iSize, CString &sError );
	int Write( RageFileBasic *&pFile, const void *pBuf, int iSize, CString &sError );
	int Flush( RageFileBasic *&pFile, CString &sError );
	RageFileBasic *Copy( RageFileBasic *&pFile, CString &sError );

	bool FlushDirCache( const CString &sPath );
	bool PopulateFileSet( FileSet &fs, const CString &sPath );

protected:
	void HandleRequest( int iRequest );
	void RequestTimedOut();

private:

	/* All requests: */
	RageFileDriver *m_pChildDriver;

	/* List of files to delete: */
	vector<RageFileBasic *> m_apDeletedFiles;
	RageMutex m_DeletedFilesLock;

	/* REQ_OPEN, REQ_POPULATE_FILE_SET, REQ_FLUSH_DIR_CACHE: */
	CString m_sRequestPath; /* in */

	/* REQ_OPEN, REQ_COPY: */
	RageFileBasic *m_pResultFile; /* out */

	/* REQ_POPULATE_FILE_SET: */
	FileSet m_ResultFileSet; /* out */

	/* REQ_OPEN: */
	int m_iRequestMode; /* in */

	/* REQ_CLOSE, REQ_GET_FILE_SIZE, REQ_COPY: */
	RageFileBasic *m_pRequestFile; /* in */

	/* REQ_OPEN, REQ_GET_FILE_SIZE, REQ_READ, REQ_SEEK */
	int m_iResultRequest; /* out */

	/* REQ_READ, REQ_WRITE */
	int m_iRequestSize; /* in */
	CString m_sResultError; /* out */

	/* REQ_SEEK */
	int m_iRequestPos; /* in */

	/* REQ_READ */
	char *m_pResultBuffer; /* out */

	/* REQ_WRITE */
	char *m_pRequestBuffer; /* in */
};

static vector<ThreadedFileWorker *> g_apWorkers;
static RageMutex g_apWorkersMutex("WorkersMutex");

/* Set the timeout length, and reset the timer. */
void RageFileDriverTimeout::SetTimeout( float fSeconds )
{
	g_apWorkersMutex.Lock();
	for( unsigned i = 0; i < g_apWorkers.size(); ++i )
		g_apWorkers[i]->SetTimeout( fSeconds );
	g_apWorkersMutex.Unlock();
}


ThreadedFileWorker::ThreadedFileWorker( CString sPath ):
	WorkerThread( sPath ),
	m_DeletedFilesLock( sPath + "DeletedFilesLock" )
{
	/* Grab a reference to the child driver.  We'll operate on it directly. */
	m_pChildDriver = FILEMAN->GetFileDriver( sPath );
	if( m_pChildDriver == NULL )
		LOG->Warn( "ThreadedFileWorker: Mountpoint \"%s\" not found", sPath.c_str() );

	m_pResultFile = NULL;
	m_pRequestFile = NULL;
	m_pResultBuffer = NULL;

	g_apWorkersMutex.Lock();
	g_apWorkers.push_back( this );
	g_apWorkersMutex.Unlock();

	StartThread();
}

ThreadedFileWorker::~ThreadedFileWorker()
{
	StopThread();

	if( m_pChildDriver != NULL )
		FILEMAN->ReleaseFileDriver( m_pChildDriver );

	/* Unregister ourself. */
	g_apWorkersMutex.Lock();
	for( unsigned i = 0; i < g_apWorkers.size(); ++i )
	{
		if( g_apWorkers[i] == this )
		{
			g_apWorkers.erase( g_apWorkers.begin()+i );
			break;
		}
	}
	g_apWorkersMutex.Unlock();
}

void ThreadedFileWorker::HandleRequest( int iRequest )
{
	{
		m_DeletedFilesLock.Lock();
		vector<RageFileBasic *> apDeletedFiles = m_apDeletedFiles;
		m_apDeletedFiles.clear();
		m_DeletedFilesLock.Unlock();

		for( unsigned i = 0; i < apDeletedFiles.size(); ++i )
			delete apDeletedFiles[i];
	}

	/* We have a request. */
	switch( iRequest )
	{
	case REQ_OPEN:
		ASSERT( m_pResultFile == NULL );
		ASSERT( !m_sRequestPath.empty() );
		m_iResultRequest = 0;
		m_pResultFile = m_pChildDriver->Open( m_sRequestPath, m_iRequestMode, m_iResultRequest );
		break;

	case REQ_CLOSE:
		ASSERT( m_pRequestFile != NULL );
		delete m_pRequestFile;
		m_pRequestFile = NULL;

		break;

	case REQ_GET_FILE_SIZE:
		ASSERT( m_pRequestFile != NULL );
		m_iResultRequest = m_pRequestFile->GetFileSize();
		break;

	case REQ_SEEK:
		ASSERT( m_pRequestFile != NULL );
		m_iResultRequest = m_pRequestFile->Seek( m_iRequestPos );
		m_sResultError = m_pRequestFile->GetError();
		break;
	case REQ_READ:
		ASSERT( m_pRequestFile != NULL );
		ASSERT( m_pResultBuffer != NULL );
		m_iResultRequest = m_pRequestFile->Read( m_pResultBuffer, m_iRequestSize );
		m_sResultError = m_pRequestFile->GetError();
		break;

	case REQ_WRITE:
		ASSERT( m_pRequestFile != NULL );
		ASSERT( m_pRequestBuffer != NULL );
		m_iResultRequest = m_pRequestFile->Write( m_pRequestBuffer, m_iRequestSize );
		m_sResultError = m_pRequestFile->GetError();
		break;

	case REQ_FLUSH:
		ASSERT( m_pRequestFile != NULL );
		m_iResultRequest = m_pRequestFile->Flush();
		m_sResultError = m_pRequestFile->GetError();
		break;

	case REQ_COPY:
		ASSERT( m_pRequestFile != NULL );
		m_pResultFile = m_pRequestFile->Copy();
		break;

	case REQ_POPULATE_FILE_SET:
		ASSERT( !m_sRequestPath.empty() );

		m_ResultFileSet = FileSet();
		m_pChildDriver->FDB->GetFileSetCopy( m_sRequestPath, m_ResultFileSet );
		break;

	case REQ_FLUSH_DIR_CACHE:
		m_pChildDriver->FlushDirCache( m_sRequestPath );
		break;

	default:
		FAIL_M( ssprintf("%i", iRequest) );
	}
}

void ThreadedFileWorker::RequestTimedOut()
{
	/* The event timed out.  Clean up any residue from the last action. */
	SAFE_DELETE( m_pRequestFile );
	SAFE_DELETE( m_pResultFile );
	SAFE_DELETE( m_pRequestBuffer );
	SAFE_DELETE( m_pResultBuffer );
}

RageFileBasic *ThreadedFileWorker::Open( const CString &sPath, int iMode, int &iErr )
{
	if( m_pChildDriver == NULL )
	{
		iErr = ENODEV;
		return NULL;
	}

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		iErr = EFAULT; /* Win32 has no ETIMEDOUT */
		return NULL;
	}

	m_sRequestPath = sPath;
	m_iRequestMode = iMode;

	if( !DoRequest(REQ_OPEN) )
	{
		LOG->Trace( "Open(%s) timed out", sPath.c_str() );
		iErr = EFAULT; /* Win32 has no ETIMEDOUT */
		return NULL;
	}

	iErr = m_iResultRequest;
	RageFileBasic *pRet = m_pResultFile;
	m_pResultFile = NULL;

	return pRet;
}

void ThreadedFileWorker::Close( RageFileBasic *pFile )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	if( !IsTimedOut() )
	{
		/* If we're not in a timed-out state, try to wait for the deletion to complete
		 * before continuing. */
		m_pRequestFile = pFile;
		DoRequest( REQ_CLOSE );
		m_pRequestFile = NULL;
	}
	else
	{
		/* Delete the file when the timeout completes. */
		m_DeletedFilesLock.Lock();
		m_apDeletedFiles.push_back( pFile );
		m_DeletedFilesLock.Unlock();
	}
}

int ThreadedFileWorker::GetFileSize( RageFileBasic *&pFile )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */
	
	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
		return -1;

	m_pRequestFile = pFile;

	if( !DoRequest(REQ_GET_FILE_SIZE) )
	{
		/* If we time out, we can no longer access pFile. */
		pFile = NULL;
		return -1;
	}

	m_pRequestFile = NULL;

	return m_iResultRequest;
}

int ThreadedFileWorker::Seek( RageFileBasic *&pFile, int iPos, CString &sError )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
	{
		sError = "Operation timed out";
		return -1;
	}

	m_pRequestFile = pFile;
	m_iRequestPos = iPos; /* in */

	if( !DoRequest(REQ_SEEK) )
	{
		/* If we time out, we can no longer access pFile. */
		sError = "Operation timed out";
		pFile = NULL;
		return -1;
	}

	m_pRequestFile = NULL;

	return m_iResultRequest;
}

int ThreadedFileWorker::Read( RageFileBasic *&pFile, void *pBuf, int iSize, CString &sError )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
	{
		sError = "Operation timed out";
		return -1;
	}

	m_pRequestFile = pFile;
	m_iRequestSize = iSize;
	m_pResultBuffer = new char[iSize];

	if( !DoRequest(REQ_READ) )
	{
		/* If we time out, we can no longer access pFile. */
		sError = "Operation timed out";
		pFile = NULL;
		return -1;
	}

	int iGot = m_iResultRequest;
	memcpy( pBuf, m_pResultBuffer, iGot );

	m_pRequestFile = NULL;
	delete [] m_pResultBuffer;
	m_pResultBuffer = NULL;

	return iGot;
}

int ThreadedFileWorker::Write( RageFileBasic *&pFile, const void *pBuf, int iSize, CString &sError )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
	{
		sError = "Operation timed out";
		return -1;
	}

	m_pRequestFile = pFile;
	m_iRequestSize = iSize;
	m_pRequestBuffer = new char[iSize];
	memcpy( m_pRequestBuffer, pBuf, iSize );

	if( !DoRequest(REQ_WRITE) )
	{
		/* If we time out, we can no longer access pFile. */
		sError = "Operation timed out";
		pFile = NULL;
		return -1;
	}

	int iGot = m_iResultRequest;

	m_pRequestFile = NULL;
	delete [] m_pRequestBuffer;
	m_pRequestBuffer = NULL;

	return iGot;
}

int ThreadedFileWorker::Flush( RageFileBasic *&pFile, CString &sError )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
	{
		sError = "Operation timed out";
		return -1;
	}

	m_pRequestFile = pFile;

	if( !DoRequest(REQ_FLUSH) )
	{
		/* If we time out, we can no longer access pFile. */
		sError = "Operation timed out";
		pFile = NULL;
		return -1;
	}

	m_pRequestFile = NULL;

	return m_iResultRequest;
}

RageFileBasic *ThreadedFileWorker::Copy( RageFileBasic *&pFile, CString &sError )
{
	ASSERT( m_pChildDriver != NULL ); /* how did you get a file to begin with? */

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
	{
		this->Close( pFile );
		pFile = NULL;
	}

	if( pFile == NULL )
	{
		sError = "Operation timed out";
		return NULL;
	}

	m_pRequestFile = pFile;
	if( !DoRequest(REQ_COPY) )
	{
		/* If we time out, we can no longer access pFile. */
		sError = "Operation timed out";
		pFile = NULL;
		return NULL;
	}

	RageFileBasic *pRet = m_pResultFile;
	m_pRequestFile = NULL;
	m_pResultFile = NULL;

	return pRet;
}


bool ThreadedFileWorker::PopulateFileSet( FileSet &fs, const CString &sPath )
{
	if( m_pChildDriver == NULL )
		return false;

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
		return false;

	/* Fill in a different FileSet; copy the results in on success. */
	m_sRequestPath = sPath;

	/* Kick off the worker thread, and wait for it to finish. */
	if( !DoRequest(REQ_POPULATE_FILE_SET) )
	{
		LOG->Trace( "PopulateFileSet(%s) timed out", sPath.c_str() );
		return false;
	}

	fs = m_ResultFileSet;
	return true;
}

bool ThreadedFileWorker::FlushDirCache( const CString &sPath )
{
	if( m_pChildDriver == NULL )
		return false;

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
		return false;

	m_sRequestPath = sPath;

	/* Kick off the worker thread, and wait for it to finish. */
	if( !DoRequest(REQ_FLUSH_DIR_CACHE) )
	{
		LOG->Trace( "FlushDirCache(%s) timed out", sPath.c_str() );
		return false;
	}

	return true;
}


class RageFileObjTimeout: public RageFileObj
{
public:
	/* pFile will be freed by passing it to pWorker. */
	RageFileObjTimeout( ThreadedFileWorker *pWorker, RageFileBasic *pFile, int iSize )
	{
		m_pWorker = pWorker;
		m_pFile = pFile;
		m_iFileSize = iSize;
	}

	~RageFileObjTimeout()
	{
		if( m_pFile != NULL )
			m_pWorker->Close( m_pFile );
	}

	int GetFileSize() const
	{
		return m_iFileSize;
	}

	RageFileBasic *Copy() const
	{
		CString sError;
		RageFileBasic *pRet = m_pWorker->Copy( m_pFile, sError );

		if( m_pFile == NULL )
		{
//			SetError( "Operation timed out" );
			return NULL;
		}

		if( pRet == NULL )
		{
//			SetError( sError );
			return NULL;
		}

		return pRet;
	}

protected:
	int SeekInternal( int iPos )
	{
		CString sError;
		int iRet = m_pWorker->Seek( m_pFile, iPos, sError );

		if( m_pFile == NULL )
		{
			SetError( "Operation timed out" );
			return -1;
		}

		if( iRet == -1 )
			SetError( sError );

		return iRet;
	}


	int ReadInternal( void *pBuffer, size_t iBytes )
	{
		CString sError;
		int iRet = m_pWorker->Read( m_pFile, pBuffer, iBytes, sError );

		if( m_pFile == NULL )
		{
			SetError( "Operation timed out" );
			return -1;
		}

		if( iRet == -1 )
			SetError( sError );

		return iRet;
	}

	int WriteInternal( const void *pBuffer, size_t iBytes )
	{
		CString sError;
		int iRet = m_pWorker->Write( m_pFile, pBuffer, iBytes, sError );

		if( m_pFile == NULL )
		{
			SetError( "Operation timed out" );
			return -1;
		}

		if( iRet == -1 )
			SetError( sError );

		return iRet;
	}

	int FlushInternal()
	{
		CString sError;
		int iRet = m_pWorker->Flush( m_pFile, sError );

		if( m_pFile == NULL )
		{
			SetError( "Operation timed out" );
			return -1;
		}

		if( iRet == -1 )
			SetError( sError );

		return iRet;
	}

	/* Mutable because the const operation Copy() timing out results in the original
	 * object no longer being available. */
	mutable RageFileBasic *m_pFile;

	ThreadedFileWorker *m_pWorker;

	/* GetFileSize isn't allowed to fail, so cache the file size on load. */
	int m_iFileSize;
};

/* This FilenameDB runs PopulateFileSet in the worker thread. */
class TimedFilenameDB: public FilenameDB
{
public:
	TimedFilenameDB()
	{
		ExpireSeconds = -1;
		m_pWorker = NULL;
	}

	void SetWorker( ThreadedFileWorker *pWorker )
	{
		ASSERT( pWorker != NULL );
		m_pWorker = pWorker;
	}

	void PopulateFileSet( FileSet &fs, const CString &sPath )
	{
		ASSERT( m_pWorker != NULL );
		m_pWorker->PopulateFileSet( fs, sPath );
	}

private:
	ThreadedFileWorker *m_pWorker;
};

RageFileDriverTimeout::RageFileDriverTimeout( CString sPath ):
        RageFileDriver( new TimedFilenameDB() )
{
	m_pWorker = new ThreadedFileWorker( sPath );

	((TimedFilenameDB *) FDB)->SetWorker( m_pWorker );
}

RageFileBasic *RageFileDriverTimeout::Open( const CString &sPath, int iMode, int &iErr )
{
	RageFileBasic *pChildFile = m_pWorker->Open( sPath, iMode, iErr );
	if( pChildFile == NULL )
		return NULL;

	/* RageBasicFile::GetFileSize isn't allowed to fail, but we are; grab the file
	 * size now and store it. */
	int iSize = 0;
	if( iMode & RageFile::READ )
	{
		iSize = m_pWorker->GetFileSize( pChildFile );
		if( iSize == -1 )
		{
			/* When m_pWorker->GetFileSize fails, it takes ownership of pChildFile. */
			ASSERT( pChildFile == NULL );
			iErr = EFAULT;
			return NULL;
		}
	}

	return new RageFileObjTimeout( m_pWorker, pChildFile, iSize );
}

void RageFileDriverTimeout::FlushDirCache( const CString &sPath )
{
	RageFileDriver::FlushDirCache( sPath );
	m_pWorker->FlushDirCache( sPath );
}

RageFileDriverTimeout::~RageFileDriverTimeout()
{
	delete m_pWorker;
}

static struct FileDriverEntry_Timeout: public FileDriverEntry
{
        FileDriverEntry_Timeout(): FileDriverEntry( "TIMEOUT" ) { }
        RageFileDriver *Create( CString Root ) const { return new RageFileDriverTimeout( Root ); }
} const g_RegisterDriver;

/*
 * Copyright (c) 2005 Glenn Maynard
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
