/* RageFileDriverReadAhead - Read-ahead hinting for seamless rewinding. */

#ifndef RAGE_FILE_DRIVER_READ_AHEAD_H
#define RAGE_FILE_DRIVER_READ_AHEAD_H

#include "RageFileBasic.h"

#include <cstddef>

class RageFileDriverReadAhead: public RageFileObj
{
public:
	/* This filter can only be used on supported files; test before using. */
	static bool FileSupported( RageFileBasic *pFile );

	/* pFile will be freed if DeleteFileWhenFinished is called. */
	RageFileDriverReadAhead( RageFileBasic *pFile, int iCacheBytes, int iPostBufferReadAhead = -1 );
	RageFileDriverReadAhead( const RageFileDriverReadAhead &cpy );
	~RageFileDriverReadAhead();
	RageFileDriverReadAhead *Copy() const;

	void DeleteFileWhenFinished() { m_bFileOwned = true; }

	virtual RString GetError() const { return m_pFile->GetError(); }
	virtual void ClearError()  { return m_pFile->ClearError(); }

	int ReadInternal( void *pBuffer, std::size_t iBytes );
	int WriteInternal( const void *pBuffer, std::size_t iBytes ) { return m_pFile->Write( pBuffer, iBytes ); }
	int SeekInternal( int iOffset );
	int GetFileSize() const { return m_pFile->GetFileSize(); }
	int GetFD() { return m_pFile->GetFD(); }
	int Tell() const { return m_iFilePos; }

private:
	void FillBuffer( int iBytes );

	RageFileBasic *m_pFile;
	int m_iFilePos;
	bool m_bFileOwned;
	RString m_sBuffer;
	int m_iPostBufferReadAhead;
	bool m_bReadAheadNeeded;
};

#endif

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

