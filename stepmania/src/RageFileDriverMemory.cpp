#include "global.h"
#include "RageFileDriverMemory.h"
#include "RageUtil.h"
#include "RageUtil_FileDB.h"
#include <errno.h>

struct RageFileObjMemFile
{
	RageFileObjMemFile():
		m_iRefs(0),
		m_bDeleted(false),
		m_Mutex("RageFileObjMemFile") { }
	CString m_sBuf;
	int m_iRefs;
	bool m_bDeleted;
	RageMutex m_Mutex;
};

class RageFileObjMem: public RageFileObj
{
private:
	RageFileObjMemFile *m_pFile;
	int m_iFilePos;

public:
	RageFileObjMem( RageFileObjMemFile *pFile, RageFile &p ):
		RageFileObj(p)
	{
		m_pFile = pFile;
		m_iFilePos = 0;
		++m_pFile->m_iRefs;
	}

	~RageFileObjMem()
	{
		m_pFile->m_Mutex.Lock();
		const int iRefs = --m_pFile->m_iRefs;
		const bool bShouldDelete = (iRefs == 0 && m_pFile->m_bDeleted);
		m_pFile->m_Mutex.Unlock();
		ASSERT( iRefs >= 0 );

		/* If the file was removed while we still had it open, and is now unreferenced,
		 * delete it. */
		if( bShouldDelete )
			delete m_pFile;
	}

	int Read( void *buffer, size_t bytes )
	{
		m_pFile->m_Mutex.Lock();

		m_iFilePos = min( m_iFilePos, GetFileSize() );
		bytes = min( bytes, (size_t) GetFileSize() - m_iFilePos );
		memcpy( buffer, &m_pFile->m_sBuf[m_iFilePos], bytes );
		m_iFilePos += bytes;
		m_pFile->m_Mutex.Unlock();

		return bytes;
	}

	int Write( const void *buffer, size_t bytes )
	{
		m_pFile->m_Mutex.Lock();
		m_pFile->m_sBuf.append( (const char *) buffer, bytes );
		m_pFile->m_Mutex.Unlock();

		m_iFilePos += bytes;
		return bytes;
	}

	void Rewind()
	{
		m_iFilePos = 0;
	}

	int Seek( int offset )
	{
		m_iFilePos = clamp( offset, 0, GetFileSize() );
		return m_iFilePos;
	}

	int GetFileSize()
	{
		LockMut(m_pFile->m_Mutex);
		return m_pFile->m_sBuf.size();
	}

	RageFileObj *Copy( RageFile &p ) const
	{
		m_pFile->m_Mutex.Unlock();
		++m_pFile->m_iRefs;
		m_pFile->m_Mutex.Lock();

		RageFileObjMem *pRet = new RageFileObjMem( m_pFile, p );
		pRet->m_iFilePos = m_iFilePos;

		return pRet;
	}
};


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
		ASSERT( !pFile->m_iRefs );

		delete pFile;
	}
}

RageFileObj *RageFileDriverMem::Open( const CString &sPath, int mode, RageFile &p, int &err )
{
	LockMut(m_Mutex);

	if( mode == RageFile::WRITE )
	{
		/* If the file exists, delete it. */
		Remove( sPath );

		RageFileObjMemFile *pFile = new RageFileObjMemFile;

		m_Files.push_back( pFile );
		FDB->AddFile( sPath, 0, 0, pFile );

		return new RageFileObjMem( pFile, p );
	}

	RageFileObjMemFile *pFile = (RageFileObjMemFile *) FDB->GetFilePriv( sPath );
	if( pFile == NULL )
	{
		err = ENOENT;
		return NULL;
	}

	return new RageFileObjMem( pFile, p );
}

bool RageFileDriverMem::Remove( const CString &sPath )
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

	pFile->m_Mutex.Lock();
	if( pFile->m_iRefs )
	{
		/* The file is in use.  The last RageFileObjMem dtor will delete it. */
		pFile->m_bDeleted = true;
		pFile->m_Mutex.Unlock();
	}
	else
	{
		pFile->m_Mutex.Unlock();
		delete pFile;
	}

	return true;
}

static struct FileDriverEntry_MEM: public FileDriverEntry
{
	FileDriverEntry_MEM(): FileDriverEntry( "MEM" ) { }
	RageFileDriver *Create( CString Root ) const { return new RageFileDriverMem(); }
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
