#ifndef FileTransfer_H
#define FileTransfer_H

#if !defined(WITHOUT_NETWORKING)

#include "ezsockets.h"
#include "RageFile.h"

class FileTransfer
{
public:
	FileTransfer();
	~FileTransfer();

	void StartDownload(const std::string &sURL, const std::string &sDestFile);
	void StartUpload(const std::string &sURL, const std::string &sSrcFile, const std::string &sDestFile);

	void Cancel();
	std::string Update(float fDeltaTime);
	bool IsFinished() const { return m_bFinished; }
	void Finish();
	int GetResponseCode() const { return m_iResponseCode; }
	std::string GetResponse() const { return m_sBUFFER; }
	std::string GetStatus() const { return m_sStatus; }
private:
	enum TransferType { download, upload };
	void StartTransfer(TransferType type, const std::string &sURL, const std::string &sSrcFile, const std::string &sDestFile);
	int m_iPackagesPos;
	int m_iLinksPos;

	// HTTP portion
	void HTTPUpdate();

	// True if proper string, false if improper
	bool ParseHTTPAddress(const std::string & URL, std::string & Proto, std::string & Server, int & Port, std::string & Addy);

	void	UpdateProgress();

	bool	m_bIsDownloading;
	float	m_fLastUpdate;
	long	m_bytesLastUpdate;

	std::string	m_sStatus;

	EzSockets m_wSocket;

	bool	m_bGotHeader;

	RageFile	m_fOutputFile;
	std::string	m_sEndName;
	bool	m_bSavingFile;

	std::string m_sBaseAddress;
	// HTTP Header information responce
	long	m_iTotalBytes;
	long	m_iDownloaded;

	long	m_iResponseCode;
	std::string	m_sResponseName;

	// Raw HTTP Buffer
	std::string m_sBUFFER;

	bool m_bFinished;
};

#endif

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
