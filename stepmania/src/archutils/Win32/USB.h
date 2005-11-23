/* WindowsFileIO - Windows device I/O */

#ifndef WIN32_USB_H
#define WIN32_USB_H

#include <vector>
#include <windows.h>

class WindowsFileIO
{
public:
	WindowsFileIO();
	~WindowsFileIO();
	bool Open( CString sPath, int iBlockSize );
	bool IsOpen() const;

	/* Nonblocking read.  size must always be the same.  Returns the number of bytes
	 * read, or 0. */
	int read( void *p );
	static int read_several( const vector<WindowsFileIO *> &sources, void *p, int &actual, float timeout );

private:
	void queue_read();
	int finish_read( void *p );

	HANDLE m_Handle;
	OVERLAPPED m_Overlapped;
	char *m_pBuffer;
	int m_iBlockSize;
};

/* WindowsFileIO - Windows USB I/O */
class USBDevice
{
public:
	int GetPadEvent();
	bool Open( int VID, int PID, int blocksize, int num );
	bool IsOpen() const;

	WindowsFileIO m_IO;
};

#endif

/*
 * (c) 2002-2005 Glenn Maynard
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
