#ifndef WIN32_USB_H
#define WIN32_USB_H

#include <vector>

/* The API for Windows device I/O is obscenely horrible, so encapsulate it. */
class WindowsFileIO
{
public:
	WindowsFileIO();
	~WindowsFileIO();
	bool Open(CString path, int blocksize);
	bool IsOpen() const;

	/* Returns the amount of data read. */
	/* If nonblocking is true, this will */
	/* Nonblocking read.  size must always be the same.  Returns the number of bytes
	 * read, or 0. */
	int read(void *p);
	static int read_several(const vector<WindowsFileIO *> &sources, void *p, int &actual, float timeout);

private:
	HANDLE h;
	OVERLAPPED ov;
	char *buf;
	int blocksize;
	
	void queue_read();
	int finish_read(void *p);
};

class USBDevice
{
public:
	int GetPadEvent();
	bool Open(int VID, int PID, int blocksize, int num);
	bool IsOpen() const;

	WindowsFileIO io;
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
