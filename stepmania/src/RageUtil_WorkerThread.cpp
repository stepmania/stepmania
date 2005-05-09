#include "global.h"
#include "RageUtil_WorkerThread.h"
#include "RageUtil.h"
#include "RageLog.h"

WorkerThread::WorkerThread( const CString &sName ):
	m_WorkerEvent( "\"" + sName + "\" worker event" ),
	m_HeartbeatEvent( "\"" + sName + "\" heartbeat event" )
{
	m_sName = sName;
	m_Timeout.SetZero();
	m_iRequest = REQ_NONE;
	m_bTimedOut = false;
	m_fHeartbeat = -1;
	m_bRequestFinished = false;

	m_WorkerThread.SetName( "WorkerThread (" + sName + ")" );
}

WorkerThread::~WorkerThread()
{
	/* The worker thread must be stopped by the derived class. */
	ASSERT( !m_WorkerThread.IsCreated() );
}

void WorkerThread::SetTimeout( float fSeconds )
{
	m_WorkerEvent.Lock();
	if( fSeconds < 0 )
		m_Timeout.SetZero();
	else
	{
		m_Timeout.Touch();
		m_Timeout += fSeconds;
	}
	m_WorkerEvent.Unlock();
}


void WorkerThread::StartThread()
{
	ASSERT( !m_WorkerThread.IsCreated() );

	m_WorkerThread.Create( StartWorkerMain, this );
}

void WorkerThread::StopThread()
{
	/* If we're timed out, wait. */
	m_WorkerEvent.Lock();
	if( m_bTimedOut )
	{
		LOG->Trace( "Waiting for timed-out worker thread \"%s\" to complete ...", m_sName.c_str() );
		while( m_bTimedOut )
			m_WorkerEvent.Wait();
	}
	m_WorkerEvent.Unlock();

	/* Disable the timeout.  This will ensure that we really wait for the worker
	 * thread to shut down. */
	SetTimeout( -1 );

	/* Shut down. */
	DoRequest( REQ_SHUTDOWN );
	m_WorkerThread.Wait();
}

bool WorkerThread::DoRequest( int iRequest )
{
	ASSERT( !m_bTimedOut );
	ASSERT( m_iRequest == REQ_NONE );

	if( m_Timeout.IsZero() && iRequest != REQ_SHUTDOWN )
		LOG->Warn( "Request made with timeout disabled (%s, iRequest = %i)", m_sName.c_str(), iRequest );

	/* Set the request, and wake up the worker thread. */
	m_WorkerEvent.Lock();

	m_iRequest = iRequest;
	m_WorkerEvent.Broadcast();

	/* Wait for it to complete or time out. */
	while( !m_bRequestFinished )
	{
		bool bTimedOut = !m_WorkerEvent.Wait( &m_Timeout );
		if( bTimedOut )
			break;
	}

	const bool bRequestFinished = m_bRequestFinished;
	if( m_bRequestFinished )
	{
		/* The request finished successfully.  It's the calling function's
		 * responsibility to clean up. */
		m_bRequestFinished = false;
	}
	else
	{
		/* The request hasn't finished yet.  Set m_bTimedOut true.  This tells the
		 * still-running request that it timed out, and so it's the thread's
		 * responsibility to clean up--we can't do it, since we'd collide with the
		 * request. */
		m_bTimedOut = true;
	}
	m_WorkerEvent.Unlock();

	return bRequestFinished;
}

void WorkerThread::WorkerMain()
{
	while(1)
	{
		bool bTimeToRunHeartbeat = false;
		m_WorkerEvent.Lock();
		while( m_iRequest == REQ_NONE && !bTimeToRunHeartbeat )
		{
			if( !m_WorkerEvent.Wait( m_fHeartbeat != -1? &m_NextHeartbeat:NULL ) )
				bTimeToRunHeartbeat = true;
		}
		const int iRequest = m_iRequest;
		m_iRequest = REQ_NONE;

		m_WorkerEvent.Unlock();

		/* If it's time to run a heartbeat, do so. */
		if( bTimeToRunHeartbeat )
		{
			DoHeartbeat();

			/* Wake up anyone waiting for a heartbeat. */
			m_HeartbeatEvent.Lock();
			m_HeartbeatEvent.Broadcast();
			m_HeartbeatEvent.Unlock();

			/* Schedule the next heartbeat. */
			m_NextHeartbeat.Touch();
			m_NextHeartbeat += m_fHeartbeat;
		}

		if( iRequest != REQ_NONE )
		{
			/* Handle the request. */
			if( iRequest != REQ_SHUTDOWN )
			{
				CHECKPOINT_M( ssprintf("HandleRequest(%i)", iRequest) );
				HandleRequest( iRequest );
				CHECKPOINT_M( ssprintf("HandleRequest(%i) done", iRequest) );
			}

			/* Lock the mutex, to keep DoRequest where it is (if it's still running). */
			/* The request is finished.  If it timed out, clear the timeout flag and
			 * call RequestTimedOut, to allow cleaning up. */
			m_WorkerEvent.Lock();

			if( m_bTimedOut )
			{
				LOG->Trace( "Request %i timed out", iRequest );

				/* The calling thread timed out.  It's already gone and moved on, so
				 * it's our responsibility to clean up.  No new requests will come in
				 * until we clear m_bTimedOut, so we can safely unlock and clean up. */
				m_WorkerEvent.Unlock();

				RequestTimedOut();

				/* Clear the time-out flag, indicating that we can work again. */
				m_bTimedOut = false;

				CHECKPOINT;
			}
			else
			{
				CHECKPOINT_M( ssprintf("HandleRequest(%i) OK", iRequest) );

				m_bRequestFinished = true;

				/* We're finished.  Wake up the requester. */
				m_WorkerEvent.Broadcast();
				m_WorkerEvent.Unlock();
			}
		}

		if( iRequest == REQ_SHUTDOWN )
			break;
	}
}

bool WorkerThread::WaitForOneHeartbeat()
{
	/* It doesn't make sense to wait for a heartbeat if there is no heartbeat. */
	ASSERT( m_fHeartbeat != -1 );

	m_HeartbeatEvent.Lock();
	bool bTimedOut = !m_HeartbeatEvent.Wait( &m_Timeout );
	m_HeartbeatEvent.Unlock();

	return !bTimedOut;
}

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
