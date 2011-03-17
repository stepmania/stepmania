#ifndef CRASH_HANDLER_NETWORKING_H
#define CRASH_HANDLER_NETWORKING_H

#include "RageThreads.h"
#include <map>

class NetworkStream;

// Send a set of data over HTTP, as a POST form.
class NetworkPostData
{
public:
	NetworkPostData();
	~NetworkPostData();

	void SetData( const RString &sKey, const RString &sData );

	// For simplicity, we don't parse URLs here.
	void Start( const RString &sHost, int iPort, const RString &sPath );

	// Cancel the running operation, and close the thread.
	void Cancel();

	// If the operation is unfinished, return false. Otherwise, close the thread and return true.
	bool IsFinished();

	RString GetStatus() const;
	float GetProgress() const;
	RString GetError() const;
	RString GetResult() const { return m_sResult; }

private:
	static void CreateMimeData( const map<RString,RString> &mapNameToData, RString &sOut, RString &sMimeBoundaryOut );
	void SetProgress( float fProgress );

	RageThread m_Thread;
	void HttpThread();
	static int HttpThread_Start( void *p ) { ((NetworkPostData *) p)->HttpThread(); return 0; }

	mutable RageMutex m_Mutex;
	RString m_sStatus;
	float m_fProgress;

	// When the thread exists, it owns the rest of the data, regardless of m_Mutex.
	map<RString, RString> m_Data;

	bool m_bFinished;
	RString m_sHost;
	int m_iPort;
	RString m_sPath;
	RString m_sResult;

	NetworkStream *m_pStream;
};

#endif

/*
 * (c) 2006 Glenn Maynard
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
