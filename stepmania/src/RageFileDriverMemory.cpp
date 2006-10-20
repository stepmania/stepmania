#include "global.h"
#include "RageFileDriverMemory.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include <errno.h>

struct RageFileObjMemFile
{
	RageFileObjMemFile():
		m_iRefs(0),
		m_Mutex("RageFileObjMemFile") { }
	RString m_sBuf;
	int m_iRefs;
	RageMutex m_Mutex;

	static void AddReference( RageFileObjMemFile *pFile )
	{
		pFile->m_Mutex.Lock();
		++pFile->m_iRefs;
		pFile->m_Mutex.Unlock();
	}

	static void ReleaseReference( RageFileObjMemFile *pFile )
	{
		pFile->m_Mutex.Lock();
		const int iRefs = --pFile->m_iRefs;
		const bool bShouldDelete = (pFile->m_iRefs == 0);
		pFile->m_Mutex.Unlock();
		ASSERT( iRefs >= 0 );

		if( bShouldDelete )
			delete pFile;
	}
};

RageFileObjMem::RageFileObjMem( RageFileObjMemFile *pFile )
{
	if( pFile == NULL )
		pFile = new RageFileObjMemFile;

	m_pFile = pFile;
	m_iFilePos = 0;
	RageFileObjMemFile::AddReference( m_pFile );
}

RageFileObjMem::~RageFileObjMem()
{
	RageFileObjMemFile::ReleaseReference( m_pFile );
}

int RageFileObjMem::ReadInternal( void *buffer, size_t bytes )
{
	m_pFile->m_Mutex.Lock();

	m_iFilePos = min( m_iFilePos, GetFileSize() );
	bytes = min( bytes, (size_t) GetFileSize() - m_iFilePos );
	memcpy( buffer, &m_pFile->m_sBuf[m_iFilePos], bytes );
	m_iFilePos += bytes;
	m_pFile->m_Mutex.Unlock();

	return bytes;
}

int RageFileObjMem::WriteInternal( const void *buffer, size_t bytes )
{
	m_pFile->m_Mutex.Lock();
	m_pFile->m_sBuf.replace( m_iFilePos, bytes, (const char *) buffer, bytes );
	m_pFile->m_Mutex.Unlock();

	m_iFilePos += bytes;
	return bytes;
}

int RageFileObjMem::SeekInternal( int offset )
{
	m_iFilePos = clamp( offset, 0, GetFileSize() );
	return m_iFilePos;
}

int RageFileObjMem::GetFileSize() const
{
	LockMut(m_pFile->m_Mutex);
	return m_pFile->m_sBuf.size();
}

RageFileObjMem::RageFileObjMem( const RageFileObjMem &cpy ):
	RageFileObj( cpy )
{
	m_pFile = cpy.m_pFile;
	m_iFilePos = cpy.m_iFilePos;
	RageFileObjMemFile::AddReference( m_pFile );
}

RageFileObjMem *RageFileObjMem::Copy() const
{
	RageFileObjMem *pRet = new RageFileObjMem( *this );
	return pRet;
}

const RString &RageFileObjMem::GetString() const
{
	return m_pFile->m_sBuf;
}

void RageFileObjMem::PutString( const RString &sBuf )
{
	m_pFile->m_Mutex.Lock();
	m_pFile->m_sBuf = sBuf;
	m_pFile->m_Mutex.Unlock();
}

RageFileDriverMem::RageFileDriverMem():
	RageFileDriver( new NullFilenameDB ),
	m_Mutex("RageFileDriverMem")
{
}

RageFileDriverMem::~RageFileDriverMem()
{
	for( unsigned i = 0; i < m_Files.size(); ++i )
	{
		RageFileObjMemFile *pFile = m_Files[i];
		RageFileObjMemFile::ReleaseReference( pFile );
	}
}

RageFileBasic *RageFileDriverMem::Open( const RString &sPath, int mode, int &err )
{
	LockMut(m_Mutex);

	if( mode == RageFile::WRITE )
	{
		/* If the file exists, delete it. */
		Remove( sPath );

		RageFileObjMemFile *pFile = new RageFileObjMemFile;

		/* Add one reference, representing the file in the filesystem. */
		RageFileObjMemFile::AddReference( pFile );

		m_Files.push_back( pFile );
		FDB->AddFile( sPath, 0, 0, pFile );

		return new RageFileObjMem( pFile );
	}

	RageFileObjMemFile *pFile = (RageFileObjMemFile *) FDB->GetFilePriv( sPath );
	if( pFile == NULL )
	{
		err = ENOENT;
		return NULL;
	}

	return new RageFileObjMem( pFile );
}

bool RageFileDriverMem::Remove( const RString &sPath )
{
	LockMut(m_Mutex);

	RageFileObjMemFile *pFile = (RageFileObjMemFile *) FDB->GetFilePriv( sPath );
	if( pFile == NULL )
		return false;

	/* Unregister the file. */
	FDB->DelFile( sPath );
	vector<RageFileObjMemFile *>::iterator it = find( m_Files.begin(), m_Files.end(), pFile );
	ASSERT( it != m_Files.end() );
	m_Files.erase( it );

	RageFileObjMemFile::ReleaseReference( pFile );

	return true;
}

static struct FileDriverEntry_MEM: public FileDriverEntry
{
	FileDriverEntry_MEM(): FileDriverEntry( "MEM" ) { }
	RageFileDriver *Create( const RString &sRoot ) const { return new RageFileDriverMem(); }
} const g_RegisterDriver;

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
