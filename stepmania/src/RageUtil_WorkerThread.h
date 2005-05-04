/* WorkerThread - a worker thread for operations that are allowed to time out. */

#ifndef RAGE_UTIL_WORKER_THREAD_H
#define RAGE_UTIL_WORKER_THREAD_H

#include "RageThreads.h"
#include "RageTimer.h"

class WorkerThread
{
public:
	WorkerThread( const CString &sName );
	virtual ~WorkerThread();

	/* Call SetTimeout(10) to start a timeout period of 10 seconds.  This is not a
	 * per-request timeout; you have 10 seconds to do your work, at which point all
	 * requests time out until SetTimeout is called again. */
	void SetTimeout( float fSeconds );
	bool TimeoutEnabled() const { return !m_Timeout.IsZero(); }

	/* Return true if the last operation has timed out and has not yet recovered. */
	bool IsTimedOut() const { return m_bTimedOut; }

	/* Pause until the next heartbeat completes.  Returns false if timed out.  This
	 * triggers no actions, so no cleanup is run and IsTimedOut() is not affected. */
	bool WaitForOneHeartbeat();

protected:
	/* Call this in the derived class to start and stop the thread. */
	void StartThread();
	void StopThread();

	/* Run the given request.  Return true if the operation completed, false on timeout.
	 * Always call IsTimedOut() first; if true is returned, the thread is currently
	 * timed out and DoRequest() must not be called. */
	bool DoRequest( int iRequest );

	/* Overload this in the derived class to handle requests. */
	virtual void HandleRequest( int iRequest ) = 0;

	/* If DoRequest times out, this will be called in the thread after completion.
	 * Clean up.  No new requests will be allowed until this completes. */
	virtual void RequestTimedOut() { }

	/* Enable a heartbeat.  DoHeartbeat will be called every fSeconds while idle.
	 * DoHeartbeat may safely time out; if DoRequest tries to start a request in
	 * the main thread, it'll simply time out. */
	void SetHeartbeat( float fSeconds ) { m_fHeartbeat = fSeconds; m_NextHeartbeat.Touch(); }
	virtual void DoHeartbeat() { }

private:
	static int StartWorkerMain( void *pThis ) { ((WorkerThread *) (pThis))->WorkerMain(); return 0; }
	void WorkerMain();

	enum { REQ_SHUTDOWN = -1, REQ_NONE = -2 };
	RageThread m_WorkerThread;
	RageEvent m_WorkerEvent;
	CString m_sName;
	int m_iRequest;
	bool m_bRequestFinished;
	bool m_bTimedOut;
	RageTimer m_Timeout;

	float m_fHeartbeat;
	RageTimer m_NextHeartbeat;
	RageEvent m_HeartbeatEvent;
};

#endif

/*
 * (c) 2005 Glenn Maynard
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
