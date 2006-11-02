#include "global.h"
#include "CrashHandlerNetworking.h"
#include "RageThreads.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "Foreach.h"

#if defined(WINDOWS)
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

/*
 * This has an arch-like abstraction layout, since it's intended to become one.
 * It'll be moved out of here as things get polished further.  The Winsock and
 * BSD implementations will be kept separate, since while they look similar,
 * they're different in the details, especially when you want to cleanly support
 * cancellation.
 *
 * Design notes:
 *
 *  - Allow cancellation at any time.  The only thing more annoying than a laggy
 *    network, is an application that won't immediately respond to cancelling.
 *    (This is even more important here, since we're using this after a crash.)
 *  - Assume some operations and/or some architectures are going to have problems
 *    doing everything nonblockingly, and do everything in a thread.  This also allows
 *    more complex network interactions, since it doesn't need to maintain an equally
 *    complex state machine.
 *  - When an operation is cancelled or an error occurs, all operations until the
 *    next call to Close() become no-ops.  This allows lenient error checking; it's
 *    guaranteed that further calls to Read or Write will not reset the error state.
 *  - The only function that may be called from another thread without serialization is
 *    Cancel().
 *
 * All calls are blocking, except for:
 *
 *  - Cancel(), which is always nonblocking.  The operation in progress, if any,
 *    will be aborted immediately.  (The only exception here is incomplete
 *    implementations, which may block);
 *  - Close(), if Shutdown() was called first, since the necessary blocking occurs
 *    during Shutdown;
 *  - Close(), if an error or cancellation occurred.
 *
 * Accepting incoming TCP connections is beyond the immediate scope of this interface
 * (that would involve a second listener class, which would be a factory for NetworkStream).
 * UDP is probably within the scope of this class, but there are some outstanding design
 * issues and it's beyond the work I'm doing right now.
 */

class NetworkStream
{
public:
	enum ConnectionType
	{
		CONN_TCP,
		CONN_UDP,
	};
	NetworkStream() { }
	virtual ~NetworkStream() { }

	/*
	 * Open a connection.  Must be in STATE_IDLE.
	 */
	virtual void Open( const RString &sHost, int iPort, ConnectionType ct = CONN_TCP ) = 0;

	/*
	 * Close down a connection.  Returns to STATE_IDLE.
	 */
	virtual void Close() = 0;

	/*
	 * Wait for all sent data to be flushed, and shut down the connection.
	 */
	virtual void Shutdown() = 0;

	/*
	 * Read data.  Block until any data is received, then return all data
	 * available.  Return the number of bytes read.  The return value will
	 * always be >= 0, unless an error or cancellation occured.
	 */
	virtual int Read( void *pBuffer, size_t iSize ) = 0;

	/*
	 * Write data to the socket.  (Design note: we always write all of the
	 * data unless an error or cancellation occurs, and those states are checked
	 * with GetState().  If that happens, the number of bytes written is
	 * meaningless, since it may have simply been buffered and never sent.  So,
	 * this function returns no value.)
	 */
	virtual void Write( const void *pBuffer, size_t iSize ) = 0;

	/*
	 * Cancel the connection.  This operation can clear an error state, aborts
	 * any blocking calls, never fails, and will always result in the socket
	 * being in STATE_CANCELLED.
	 *
	 * This is true even if the state was STATE_IDLE.  For example, the user
	 * thread may be closing one connection on this object and opening another,
	 * and the UI thread may call Cancel() in between these operations.
	 *
	 * This operation is threadsafe: a socket can be cancelled from any thread
	 * while another thread is reading or writing.  This is the only function
	 * that can be called without serialization.
	 */
	virtual void Cancel() = 0;

	enum State
	{
		/* The stream is closed, and is ready to be opened. */
		STATE_IDLE,

		/* The stream is connected and able to send and receive data. */
		STATE_CONNECTED,

		/* The stream has been shut down on either end (either an EOF from
		 * the other end, or shutdown() being called). */
		STATE_SHUTDOWN,

		/* When in CANCELLED, all blocking calls fail immediately; this state must be
		 * cleared explicitly by calling Close(). */
		STATE_CANCELLED,
		STATE_ERROR
	};

	State GetState() const { return m_State; }
	RString GetError() const { return m_sError; }

protected:
	State m_State;
	RString m_sError;
};

class NetworkStream_Win32: public NetworkStream
{
public:
	NetworkStream_Win32();
	~NetworkStream_Win32();

	void Open( const RString &sHost, int iPort, ConnectionType ct = CONN_TCP );
	void Shutdown();
	void Close();
	int Read( void *pBuffer, size_t iSize );
	void Write( const void *pBuffer, size_t iSize );

	void Cancel();

private:
	int WaitForCompletionOrCancellation( int iEvent );
	void SetError( const RString &sError );
	static RString WinSockErrorToString( int iError );

	SOCKET m_Socket;
	HANDLE m_hResolve;
	HWND m_hResolveHwnd;

	/* This event is signalled on cancellation, to wake us up if we're blocking. */
	HANDLE m_hCompletionEvent;

	RString m_sHost;
	int m_iPort;

	RageMutex m_Mutex;
};

NetworkStream *CreateNetworkStream()
{
	static bool bInitted = false;
	if( !bInitted )
	{
		bInitted = true;
		WSADATA WSAData;
		WORD iVersionRequested = MAKEWORD(2,0);
		if( WSAStartup(iVersionRequested, &WSAData) != 0 )
			return NULL;
	}

	return new NetworkStream_Win32;
}

/* WinSock implementation of NetworkStream. */
NetworkStream_Win32::NetworkStream_Win32():
	m_Mutex( "NetworkTCPSocket" )
{
	m_iPort = -1;
	m_State = STATE_IDLE;
	m_Socket = NULL;
#if defined(WINDOWS)
	m_hResolve = NULL;
	m_hResolveHwnd = NULL;
	m_hCompletionEvent = CreateEvent( NULL, true, false, NULL );
#endif
}

NetworkStream_Win32::~NetworkStream_Win32()
{
	Close();
	CloseHandle( m_hCompletionEvent );
}

/* Wait for the specified network event to occur, or cancellation, whichever
 * happens first.  On cancellation, return -1; on error, return the error
 * code; on success, return 0. */
int NetworkStream_Win32::WaitForCompletionOrCancellation( int iEvent )
{
	while(1)
	{
		int iRet = WaitForSingleObject( m_hCompletionEvent, INFINITE );
		if( iRet != WAIT_OBJECT_0 )
			continue;

		m_Mutex.Lock();

		/* This will reset the event.  Do this while we hold the lock. */
		WSANETWORKEVENTS events;
		WSAEnumNetworkEvents( m_Socket, m_hCompletionEvent, &events );

		/* Was the event signalled due to cancellation? */
		if( m_State == STATE_CANCELLED )
		{
			m_Mutex.Unlock();
			return -1;
		}

		m_Mutex.Unlock();

		/* If the event didn't actually occur, keep waiting. */
		if( (events.lNetworkEvents & (1<<iEvent)) )
			return events.iErrorCode[iEvent];

		/* If the socket was closed while we were waiting, stop.  Note that when the
		 * connection closes immediately after sending data, we'll receive both this
		 * message and FD_READ at the same time.  Only do this if the event we really
		 * want hasn't happened yet. */
		if( (events.lNetworkEvents & (1<<FD_CLOSE_BIT)) )
			return WSAECONNRESET;
	}
}

RString NetworkStream_Win32::WinSockErrorToString( int iError )
{
	/* If iError is -1, we were cancelled and WaitForCompletionOrCancellation returned it.  We won't
	 * use the error string. */
	if( iError == -1 )
		return RString();

	switch( iError )
	{
	case WSAEINTR: return "Interrupted function call.";
	case WSAEACCES: return "Permission denied.";
	case WSAEFAULT: return "Bad address.";
	case WSAEINVAL: return "Invalid argument.";
	case WSAEMFILE: return "Too many open files.";
	case WSAEWOULDBLOCK: return "Resource temporarily unavailable.";
	case WSAEINPROGRESS: return "Operation now in progress.";
	case WSAEALREADY: return "Operation already in progress.";
	case WSAENOTSOCK: return "Socket operation on nonsocket.";
	case WSAEDESTADDRREQ: return "Destination address required.";
	case WSAEMSGSIZE: return "Message too long.";
	case WSAEPROTOTYPE: return "Protocol wrong type for socket.";
	case WSAENOPROTOOPT: return "Bad protocol option.";
	case WSAEPROTONOSUPPORT: return "Protocol not supported.";
	case WSAESOCKTNOSUPPORT: return "Socket type not supported.";
	case WSAEOPNOTSUPP: return "Operation not supported.";
	case WSAEPFNOSUPPORT: return "Protocol family not supported.";
	case WSAEAFNOSUPPORT: return "Address family not supported by protocol family.";
	case WSAEADDRINUSE: return "Address already in use.";
	case WSAEADDRNOTAVAIL: return "Cannot assign requested address.";
	case WSAENETDOWN: return "Network is down.";
	case WSAENETUNREACH: return "Network is unreachable.";
	case WSAENETRESET: return "Network dropped connection on reset.";
	case WSAECONNABORTED: return "Software caused connection abort.";
	case WSAECONNRESET: return "Connection reset by peer.";
	case WSAENOBUFS: return "No buffer space available.";
	case WSAEISCONN: return "Socket is already connected.";
	case WSAENOTCONN: return "Socket is not connected.";
	case WSAESHUTDOWN: return "Cannot send after socket shutdown.";
	case WSAETIMEDOUT: return "Connection timed out.";
	case WSAECONNREFUSED: return "Connection refused.";
	case WSAEHOSTDOWN: return "Host is down.";
	case WSAEHOSTUNREACH: return "No route to host.";
	case WSAEPROCLIM: return "Too many processes.";
	case WSASYSNOTREADY: return "Network subsystem is unavailable.";
	case WSAVERNOTSUPPORTED: return "Winsock.dll version out of range.";
	case WSANOTINITIALISED: return "Successful WSAStartup not yet performed.";
	case WSAEDISCON: return "Graceful shutdown in progress. ";
	case WSATYPE_NOT_FOUND: return "Class type not found.";
	case WSAHOST_NOT_FOUND: return "Host not found.";
	case WSATRY_AGAIN: return "Nonauthoritative host not found.";
	case WSANO_RECOVERY: return "This is a nonrecoverable error.";
	case WSANO_DATA: return "Valid name, no data record of requested type.";
	case WSA_INVALID_HANDLE: return "Specified event object handle is invalid.";
	case WSA_INVALID_PARAMETER: return "One or more parameters are invalid.";
	case WSA_IO_INCOMPLETE: return "Overlapped I/O event object not in signaled state.";
	case WSA_IO_PENDING: return "Overlapped operations will complete later.";
	case WSA_NOT_ENOUGH_MEMORY: return "Insufficient memory available.";
	case WSA_OPERATION_ABORTED: return "Overlapped operation aborted.";
	case WSASYSCALLFAILURE: return "System call failure.";
	default: return ssprintf( "unknown Winsock error %i", iError );
	}
}

void NetworkStream_Win32::SetError( const RString &sError )
{
	m_Mutex.Lock();
	if( m_State != STATE_CANCELLED )
	{
		m_sError = sError;
		m_State = STATE_ERROR;
	}

	if( m_Socket != INVALID_SOCKET )
	{
		closesocket( m_Socket );
		m_Socket = INVALID_SOCKET;
	}

	m_Mutex.Unlock();
}

/* WSAAsyncGetHostByName returns events through a window. */
#include "MessageWindow.h"
class ResolveMessageWindow: public MessageWindow
{
public:
	ResolveMessageWindow(): MessageWindow("DNS notification window")
	{
		m_iResult = 0;
	}

	int GetResult() const { return m_iResult; }

protected:
	bool HandleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
	{
		if( msg == WM_USER )
		{
			m_iResult = WSAGETASYNCERROR( lParam );
			StopRunning();
			return true;
		}

		if( msg == WM_USER+1 )
		{
			m_iResult = 0;
			StopRunning();
			return true;
		}

		return false;
	}

	int m_iResult;
};

void NetworkStream_Win32::Open( const RString &sHost, int iPort, ConnectionType ct )
{
	m_Mutex.Lock();
	if( m_State == STATE_CANCELLED )
	{
		m_Mutex.Unlock();
		return;
	}

	/* Always shut down a stream completely before reusing it. */
	ASSERT_M( m_State == STATE_IDLE, ssprintf("%s:%i: %i", sHost.c_str(), iPort, m_State) );

	m_sHost = sHost;
	m_iPort = iPort;
	
	/* Look up the hostname. */
	hostent *pHost = NULL;
	char pBuf[MAXGETHOSTSTRUCT];
	{
		pHost = (hostent *) pBuf;

		ResolveMessageWindow mw;
		m_hResolve = WSAAsyncGetHostByName(
			mw.GetHwnd(),
			WM_USER,
			m_sHost,
			(char *) pHost,
			MAXGETHOSTSTRUCT
		);
		m_hResolveHwnd = mw.GetHwnd();
		m_Mutex.Unlock();

		mw.Run();

		m_Mutex.Lock();
		m_hResolve = NULL;
		m_hResolveHwnd = NULL;
		if( m_State == STATE_CANCELLED )
		{
			m_Mutex.Unlock();
			return;
		}
		int iError = mw.GetResult();
		if( iError )
		{
			SetError( ssprintf("DNS error: %s", WinSockErrorToString(iError).c_str() ) );
			m_Mutex.Unlock();
			return;
		}
		m_Mutex.Unlock();
	}

	{
		sockaddr_in addr;
		addr.sin_addr.s_addr = *(DWORD *)pHost->h_addr_list[0];
		addr.sin_family = PF_INET;
		addr.sin_port = htons( (uint16_t) iPort );

		m_Mutex.Lock();
		m_Socket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( m_Socket == INVALID_SOCKET )
		{
			int iError = WSAGetLastError();
			SetError( ssprintf("Error creating socket: %s", WinSockErrorToString(iError).c_str() ) );
			return;
		}

		/* Set up the completion event to be signalled when these events occur.  This
		 * also sets the socket to nonblocking. */
		WSAEventSelect( m_Socket, m_hCompletionEvent, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );

//		fcntl( m_Socket, O_NONBLOCK, 1 );

		/* Start opening the connection. */
		int iResult = connect(m_Socket, (SOCKADDR *) &addr, sizeof(addr));

		m_Mutex.Unlock();

		/* We expect EINPROGRESS/WSAEWOULDBLOCK. */
		if( iResult == SOCKET_ERROR )
		{
			/* Block until the connection attempt completes. */
			int iError = WSAGetLastError();
			if( iError == WSAEWOULDBLOCK )
				iError = WaitForCompletionOrCancellation( FD_CONNECT_BIT );
			if( iError )
			{
				SetError( ssprintf("Couldn't connect: %s", WinSockErrorToString(iError).c_str() ) );
				return;
			}
		}
	}

	m_State = STATE_CONNECTED;
}

void NetworkStream_Win32::Close()
{
	if( m_State == STATE_IDLE )
		return;
	
	/* If we have an active, stable connection, make sure we flush any data
	 * completely before closing.  If you don't want to do this, call Cancel()
	 * first. */
	Shutdown();

	m_Mutex.Lock();
	if( m_State == STATE_CANCELLED )
		return;

	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );
	m_Socket = INVALID_SOCKET;
	m_State = STATE_IDLE;
	m_Mutex.Unlock();
}

void NetworkStream_Win32::Shutdown()
{
	/* If we're not in a normal, stable state (eg. an error occurred, or we were
	 * cancelled), don't wait to flush. */
	if( m_State != STATE_CONNECTED )
		return;

	shutdown( m_Socket, SD_BOTH );
	m_State = STATE_SHUTDOWN;
}

void NetworkStream_Win32::Cancel()
{
	m_Mutex.Lock();

	/* Mark cancellation. */
	m_State = STATE_CANCELLED;

	/* If resolving, abort the resolve. */
	if( m_hResolve != NULL )
	{
		/* When we cancel the request, no message at all will be sent to the window,
		 * so we need to do it ourself to inform it that it was cancelled.  Be sure
		 * to only do this on successful cancel. */
		if( WSACancelAsyncRequest(m_hResolve) == 0 )
			PostMessage( m_hResolveHwnd, WM_USER+1, 0, 0 );
	}

	/* Break out if we're waiting in WaitForCompletionOrCancellation(). */
	SetEvent( m_hCompletionEvent );

	m_Mutex.Unlock();
}

int NetworkStream_Win32::Read( void *pBuffer, size_t iSize )
{
	if( m_State != STATE_CONNECTED )
		return 0;

	char *p = (char *) pBuffer;
	int iRead = 0;
	while( iSize > 0 )
	{
		int iRet = recv( m_Socket, p, iSize, 0 );
		if( iRet > 0 )
		{
			p += iRet;
			iSize -= iRet;
			iRead += iRet;
			continue;
		}

		if( iRet == 0 )
		{
			/* We hit EOF. */
			break;
		}

		int iError = WSAGetLastError();
		if( iError == WSAEWOULDBLOCK )
		{
			/* There's no date to read.  If we've already received some data, return it. */
			if( iRead > 0 )
				break;

			iError = WaitForCompletionOrCancellation( FD_READ_BIT );
			if( iError == 0 )
				continue;
		}

		/* If the other side closed the connection, just return EOF. */
		if( iError == WSAECONNRESET )
			break;

		/* If we're cancelled or hit an error while reading, do return the data we managed
		 * to get.  Be sure not to overwrite CANCELLED with ERROR. */
		SetError( ssprintf("Error reading: %s", WinSockErrorToString(iError).c_str() ) );
		break;
	}

	return iRead;
}

void NetworkStream_Win32::Write( const void *pBuffer, size_t iSize )
{
	if( m_State != STATE_CONNECTED )
		return;

	const char *p = (const char *) pBuffer;
	while( iSize > 0 )
	{
		int iRet = send( m_Socket, p, iSize, 0 );
		ASSERT( iRet != 0 );
		if( iRet > 0 )
		{
			p += iRet;
			iSize -= iRet;
			continue;
		}

		int iError = WSAGetLastError();
		if( iError == WSAEWOULDBLOCK )
		{
			iError = WaitForCompletionOrCancellation( FD_WRITE_BIT );
			if( iError == 0 )
				continue;
		}

		SetError( ssprintf("Error writing: %s", WinSockErrorToString(iError).c_str() ) );
		return;
	}
}

/*
 * Send a set of data over HTTP, as a POST form.
 */

NetworkPostData::NetworkPostData():
	m_Mutex( "NetworkPostData" )
{
	m_pStream = CreateNetworkStream();
}

NetworkPostData::~NetworkPostData()
{
	delete m_pStream;
}

/* Create a MIME multipart data block from the given set of fields. */
void NetworkPostData::CreateMimeData( const map<RString,RString> &mapNameToData, RString &sOut, RString &sMimeBoundaryOut )
{
	/* Find a non-conflicting mime boundary. */
	while(1)
	{
		sMimeBoundaryOut = ssprintf( "--%08i", rand() );
		FOREACHM_CONST( RString, RString, mapNameToData, d )
			if( d->second.find(sMimeBoundaryOut) != RString::npos )
				continue;
		break;
	}

	FOREACHM_CONST( RString, RString, mapNameToData, d )
	{
		sOut += "--" + sMimeBoundaryOut + "\r\n";
		sOut += ssprintf( "Content-Disposition: form-data; name=\"%s\"\r\n", d->first.c_str() );
		sOut += "\r\n";
		sOut += d->second;
		sOut += "\r\n";
	}
	if( sOut.size() )
		sOut += "--" + sMimeBoundaryOut + "--\r\n";

}

void NetworkPostData::HttpThread()
{
	RString sData, sMimeBoundary;
	CreateMimeData( m_Data, sData, sMimeBoundary );

	// Stick to HTTP/1.0, since the protocol is simpler.
	RString sBuf = ssprintf(
		"%s %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\n"
		"Host: %s\r\n"
		"Cache-Control: no-cache\r\n",
		sData.size()? "POST":"GET",
		m_sPath.c_str(),
		m_sHost.c_str() );

	if( sData.size() )
	{
		// sBuf += "Content-Type: application/x-www-form-urlencoded\r\n"
		sBuf += "Content-Type: multipart/form-data; boundary=" + sMimeBoundary + "\r\n";
		sBuf += ssprintf( "Content-Length: %i\r\n", sData.size() );
	}
	sBuf += "\r\n";

	if( sData.size() )
		sBuf += sData;

	/* The "progress" is currently faked; it shows when we've connected, and when
	 * we've received data.  We send and receive too little data to do more. */
	SetProgress( 0 );

	/*
	 * Begin connecting.
	 */
	m_pStream->Open( m_sHost, m_iPort );
	SetProgress( 0.25f );

	/* Send the form. */
	m_pStream->Write( sBuf.data(), sBuf.size() );

	/* Read the result. */
	RString sResult;
	while( m_pStream->GetState() == NetworkStream::STATE_CONNECTED )
	{
		sBuf.clear();
		void *p = sBuf.GetBuffer( 1024 );
		int iGot = m_pStream->Read( p, 1024 );
		if( iGot >= 0 )
			sBuf.ReleaseBuffer( iGot );
		if( iGot <= 0 )
			break;
		sResult += sBuf;
	}

	SetProgress( 1.0f );

	/* Parse the results. */
	int iStart = 0, iSize = -1;
	map<RString,RString> mapHeaders;
	while( 1 )
	{
		split( sResult, "\n", iStart, iSize, false );
		if( iStart == (int) sResult.size() )
			break;

		RString sLine = sResult.substr( iStart, iSize );
		StripCrnl( sLine );
		if( sLine.empty() )
		{
			m_sResult = sResult.substr( iStart+iSize+1 );
			break;
		}
	}

	m_bFinished = true;
}

void NetworkPostData::Start( const RString &sHost, int iPort, const RString &sPath )
{
	m_bFinished = false;
	m_sHost = sHost;
	m_iPort = iPort;
	m_sPath = sPath;
	m_sStatus.clear();
	m_fProgress = 0;

	m_Thread.SetName( "HTTP thread" );
	m_Thread.Create( HttpThread_Start, this );
}

void NetworkPostData::Cancel()
{
	m_pStream->Cancel();
	m_Thread.Wait();
}

bool NetworkPostData::IsFinished()
{
	if( !m_bFinished )
		return false;

	m_Thread.Wait();
	m_bFinished = false;
	return true;
}

RString NetworkPostData::GetStatus() const
{
	LockMut( m_Mutex );
	return m_sStatus;
}

float NetworkPostData::GetProgress() const
{
	LockMut( m_Mutex );
	return m_fProgress;
}

RString NetworkPostData::GetError() const
{
	return m_pStream->GetError();
}

void NetworkPostData::SetProgress( float fProgress )
{
	m_Mutex.Lock();
	m_fProgress = fProgress;
	m_Mutex.Unlock();
}

void NetworkPostData::SetData( const RString &sKey, const RString &sData )
{
	m_Data[sKey] = sData;
}

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
