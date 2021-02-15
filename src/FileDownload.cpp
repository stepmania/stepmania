#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "FileDownload.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "SpecialFiles.h"
#include "RageLog.h"
#include "Preference.h"


FileTransfer::FileTransfer()
{
	m_iPackagesPos = 0;
	m_iLinksPos = 0;
	m_bIsDownloading = false;
	m_sStatus = "";
	m_fLastUpdate = 0;
	m_iTotalBytes = 0;
	m_iDownloaded = 0;
	m_bFinished = false;

	UpdateProgress();

	// Workaround: For some reason, the first download sometimes corrupts;
	// by opening and closing the RageFile, this problem does not occur. 
	// Go figure?
	// XXX: This is a really dirty work around! Why does RageFile do this?
	// It's always some strange number of bytes at the end of the file when it corrupts.
	m_fOutputFile.Open("Packages/dummy.txt", RageFile::WRITE);
	m_fOutputFile.Close();
}

FileTransfer::~FileTransfer()
{
	m_fOutputFile.Close();
}

RString FileTransfer::Update( float fDeltaTime )
{
	HTTPUpdate();

	m_fLastUpdate += fDeltaTime;
	if (m_fLastUpdate >= 1.0)
	{
		if (m_bIsDownloading && m_bGotHeader)
			m_sStatus = ssprintf("DL @ %d KB/s", int((m_iDownloaded-m_bytesLastUpdate)/1024));

		m_bytesLastUpdate = m_iDownloaded;
		UpdateProgress();
		m_fLastUpdate = 0;
	}

	return m_sStatus;
}

void FileTransfer::UpdateProgress()
{
	// Code here did nothing...
}

void FileTransfer::Cancel()
{
	m_wSocket.close();
	m_bIsDownloading = false;
	m_iDownloaded = 0;
	m_bGotHeader = false;
	m_fOutputFile.Close();
	m_sStatus = "Failed.";
	m_sBUFFER = "";
	FILEMAN->Remove("Packages/" + m_sEndName);
}

void FileTransfer::StartDownload( const RString &sURL, const RString &sDestFile )
{
	StartTransfer( download, sURL, "", sDestFile );
}

void FileTransfer::StartUpload( const RString &sURL, const RString &sSrcFile, const RString &sDestFile )
{
	StartTransfer( upload, sURL, sSrcFile, sDestFile );
}

extern Preference<RString> g_sCookie;

void FileTransfer::StartTransfer( TransferType type, const RString &sURL, const RString &sSrcFile, const RString &sDestFile )
{
	RString Proto;
	RString Server;
	int Port=80;
	RString sAddress;

	if( !ParseHTTPAddress( sURL, Proto, Server, Port, sAddress ) )
	{
		m_sStatus = "Invalid URL.";
		m_bFinished = true;
		UpdateProgress();
		return;
	}

	m_bSavingFile = sDestFile != "";

	m_sBaseAddress = "http://" + Server;
	if( Port != 80 )
		m_sBaseAddress += ssprintf( ":%d", Port );
	m_sBaseAddress += "/";

	if( sAddress.Right(1) != "/" )
	{
		m_sEndName = Basename( sAddress );
		m_sBaseAddress += Dirname( sAddress );
	}
	else
	{
		m_sEndName = "";
	}

	// Open the file...

	// First find out if a file by this name already exists if so, then we gotta
	// ditch out.
	// XXX: This should be fixed by a prompt or something?

	// if we are not talking about a file, let's not worry
	if( m_sEndName != "" && m_bSavingFile )
	{
		if( !m_fOutputFile.Open( sDestFile, RageFile::WRITE | RageFile::STREAMED ) )
		{
			m_sStatus = m_fOutputFile.GetError();
			UpdateProgress();
			return;
		}
	}
	// Continue...

	sAddress = URLEncode( sAddress );

	if ( sAddress != "/" )
		sAddress = "/" + sAddress;

	m_wSocket.close();
	m_wSocket.create();

	m_wSocket.blocking = true;

	if( !m_wSocket.connect( Server, (short) Port ) )
	{
		m_sStatus = "Failed to connect.";
		UpdateProgress();
		return;
	}

	// Produce HTTP header
	RString sAction;
	switch( type )
	{
	case upload: sAction = "POST"; break;
	case download: sAction = "GET"; break;
	}

	vector<RString> vsHeaders;
	vsHeaders.push_back( sAction+" "+sAddress+" HTTP/1.0" );
	vsHeaders.push_back( "Host: " + Server );
	vsHeaders.push_back( "Cookie: " + g_sCookie.Get() );
	vsHeaders.push_back( "Connection: closed" );
	string sBoundary = "--ZzAaB03x";
	vsHeaders.push_back( "Content-Type: multipart/form-data; boundary=" + sBoundary );
	RString sRequestPayload;
	if( type == upload )
	{
		RageFile f;
		if( !f.Open( sSrcFile ) )
			FAIL_M( f.GetError() );
		sRequestPayload.reserve( f.GetFileSize() );
		int iBytesRead = f.Read( sRequestPayload );
		if( iBytesRead == -1 )
			FAIL_M( f.GetError() );

		sRequestPayload = "--" + sBoundary + "\r\n" + 
			"Content-Disposition: form-data; name=\"name\"\r\n" +
			"\r\n" +
			"Chris\r\n" +
			"--" + sBoundary + "\r\n" + 
			"Content-Disposition: form-data; name=\"userfile\"; filename=\"" + Basename(sSrcFile) + "\"\r\n" +
			"Content-Type: application/zip\r\n" + 
			"\r\n" +
			sRequestPayload + "\r\n" +
			"--" + sBoundary + "--";
	}
	/*
	if( sRequestPayload.size() > 0 )
	{
		sHeader += "Content-Type: application/octet-stream\r\n";
		sHeader += "Content-Length: multipart/form-data; boundary=" + sBoundary + "\r\n";
		//sHeader += "Content-Length: " + ssprintf("%d",sRequestPayload.size()) + "\r\n";
	}
	*/

	vsHeaders.push_back( "Content-Length: " + ssprintf("%zd",sRequestPayload.size()) );

	RString sHeader;
	for (RString const &h : vsHeaders)
		sHeader += h + "\r\n";
	sHeader += "\r\n";

	m_wSocket.SendData( sHeader.c_str(), sHeader.length() );
	m_wSocket.SendData( "\r\n" );

	m_wSocket.SendData( sRequestPayload.c_str(), sRequestPayload.size() );

	m_sStatus = "Header Sent.";
	m_wSocket.blocking = false;
	m_bIsDownloading = true;
	m_sBUFFER = "";
	m_bGotHeader = false;
	UpdateProgress();
}

static size_t FindEndOfHeaders( const RString &buf )
{
	size_t iPos1 = buf.find( "\n\n" );
	size_t iPos2 = buf.find( "\r\n\r\n" );
	LOG->Trace("end: %u, %u", unsigned(iPos1), unsigned(iPos2));
	if( iPos1 != string::npos && (iPos2 == string::npos || iPos2 > iPos1) )
		return iPos1 + 2;
	else if( iPos2 != string::npos && (iPos1 == string::npos || iPos1 > iPos2) )
		return iPos2 + 4;
	else
		return string::npos;
}

void FileTransfer::HTTPUpdate()
{
	if( !m_bIsDownloading )
		return;

	int BytesGot=0;
	// Keep this as a code block, as there may be need to "if" it out some time.
	/* If you need a conditional for a large block of code, stick it in
	 * a function and return. */
	for(;;)
	{
		char Buffer[1024];
		int iSize = m_wSocket.ReadData( Buffer, 1024 );
		if( iSize <= 0 )
			break;

		m_sBUFFER.append( Buffer, iSize );
		BytesGot += iSize;
	}

	if( !m_bGotHeader )
	{
		m_sStatus = "Waiting for header.";
		// We don't know if we are using unix-style or dos-style
		size_t iHeaderEnd = FindEndOfHeaders( m_sBUFFER );
		if( iHeaderEnd == m_sBUFFER.npos )
			return;

		// "HTTP/1.1 200 OK"
		size_t i = m_sBUFFER.find(" ");
		size_t j = m_sBUFFER.find(" ",i+1);
		size_t k = m_sBUFFER.find("\n",j+1);
		if ( i == string::npos || j == string::npos || k == string::npos )
		{
			m_iResponseCode = -100;
			m_sResponseName = "Malformed response.";
			return;
		}
		m_iResponseCode = StringToInt(m_sBUFFER.substr(i+1,j-i));
		m_sResponseName = m_sBUFFER.substr( j+1, k-j );

		i = m_sBUFFER.find("Content-Length:");
		j = m_sBUFFER.find("\n", i+1 );

		if( i != string::npos )
			m_iTotalBytes = StringToInt(m_sBUFFER.substr(i+16,j-i));
		else
			m_iTotalBytes = -1;	// We don't know, so go until disconnect

		m_bGotHeader = true;
		m_sBUFFER.erase( 0, iHeaderEnd );
	}

	if( m_bSavingFile )
	{
		m_iDownloaded += m_sBUFFER.length();
		m_fOutputFile.Write( m_sBUFFER );

		// HACK: We're ending up with incomplete files after close() if we don't flush here. Why?
		m_fOutputFile.Flush();

		m_sBUFFER = "";
	}
	else
	{
		m_iDownloaded = m_sBUFFER.length();
	}

	if ( ( m_iTotalBytes <= m_iDownloaded && m_iTotalBytes != -1 ) ||
					//We have the full doc. (And we knew how big it was)
		( m_iTotalBytes == -1 && 
			( m_wSocket.state == EzSockets::skERROR || m_wSocket.state == EzSockets::skDISCONNECTED ) ) )
				// We didn't know how big it was, and were disconnected
				// So that means we have it all.
	{
		m_wSocket.close();
		m_bIsDownloading = false;
		m_bGotHeader=false;
		m_sStatus = ssprintf( "Done;%dB", int(m_iDownloaded) );
		m_bFinished = true;

		if( m_iResponseCode < 200 || m_iResponseCode >= 400 )
		{
			m_sStatus = ssprintf( "%ld", m_iResponseCode ) + m_sResponseName;
		}
		else
		{
			if( m_bSavingFile && m_iResponseCode < 300 )
			{
				RString sZipFile = m_fOutputFile.GetRealPath();
				m_fOutputFile.Close();
				FILEMAN->FlushDirCache();
				m_iDownloaded = 0;
			}
		}
	}
}

bool FileTransfer::ParseHTTPAddress( const RString &URL, RString &sProto, RString &sServer, int &iPort, RString &sAddress )
{
	// [PROTO://]SERVER[:PORT][/URL]

	Regex re(
		"^([A-Z]+)://" // [0]: HTTP://
		"([^/:]+)"     // [1]: a.b.com
		"(:([0-9]+))?" // [2], [3]: :1234 (optional, default 80)
		"(/(.*))?$");    // [4], [5]: /foo.html (optional)
	vector<RString> asMatches;
	if( !re.Compare( URL, asMatches ) )
		return false;
	ASSERT( asMatches.size() == 6 );

	sProto = asMatches[0];
	sServer = asMatches[1];
	if( asMatches[3] != "" )
	{
		iPort = StringToInt(asMatches[3]);
		if( iPort == 0 )
			return false;
	}
	else
		iPort = 80;

	sAddress = asMatches[5];

	return true;
}

void FileTransfer::Finish()
{
	for(;;)
	{
		float fSleepSeconds = 0.1f;
		this->Update( fSleepSeconds );
		usleep( int( fSleepSeconds * 1000000.0 ) );
		if( this->IsFinished() )
		{
			break;
		}
	}
}

#endif
/*
 * (c) 2004 Charles Lohr, Chris Danford
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
