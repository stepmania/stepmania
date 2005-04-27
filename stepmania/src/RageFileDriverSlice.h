/* RageFileDriverSlice - Treat a portion of a file as a file. */

#ifndef RAGE_FILE_DRIVER_SLICE_H
#define RAGE_FILE_DRIVER_SLICE_H

#include "RageFileBasic.h"

class RageFileDriverSlice: public RageFileObj
{
public:
	/* pFile will be freed if DeleteFileWhenFinished is called. */
	RageFileDriverSlice( RageFileBasic *pFile, int iOffset, int iFileSize );
	RageFileDriverSlice( const RageFileDriverSlice &cpy );
	~RageFileDriverSlice();
	RageFileBasic *Copy() const;

	void DeleteFileWhenFinished() { m_bFileOwned = true; }

	int ReadInternal( void *pBuffer, size_t iBytes );
	int WriteInternal( const void *pBuffer, size_t iBytes ) { SetError( "Not implemented" ); return -1; }
	int SeekInternal( int iOffset );
	int GetFileSize() const { return m_iFileSize; }

private:
	RageFileBasic *m_pFile;
	int m_iFilePos;
	int m_iOffset, m_iFileSize;
	bool m_bFileOwned;
};

#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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

