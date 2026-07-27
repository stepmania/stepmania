////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipMemFile.h $
// $Archive: /ZipArchive/ZipMemFile.h $
// $Date: 2003-07-21 21:10:30 -0500 (Mon, 21 Jul 2003) $ $Author: gmaynard $
////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyright 2000-2003 by Tadeusz Dracz (http://www.artpol-software.com/)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// For the licensing details see the file License.txt
////////////////////////////////////////////////////////////////////////////////

/**
* \file ZipMemFile.h
* Interface for the CZipMemFile class.
*
*/
#if !defined(AFX_ZIPMEMFILE_H__EA73AB25_6B51_4C5E_8D78_BAC95812598F__INCLUDED_)
#define AFX_ZIPMEMFILE_H__EA73AB25_6B51_4C5E_8D78_BAC95812598F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZipAbstractFile.h"
#include "ZipString.h"
#include "ZipExport.h"

#include <cstddef>

/**
	A memory buffer which behaves like a physical file.
	Automatically grows when necessary
*/
class ZIP_API CZipMemFile : public CZipAbstractFile
{
protected:
	std::size_t m_nGrowBy, m_nPos;
	std::size_t m_nBufSize, m_nDataSize;
	BYTE* m_lpBuf;
	bool m_bAutoDelete;
	void Free()
	{
		if (m_lpBuf)
		{
			free(m_lpBuf);
			m_lpBuf = NULL;
		}
	}
	void Init()
	{
		m_nGrowBy = m_nPos = 0;
		m_nBufSize = m_nDataSize = 0;
		m_lpBuf = NULL;

	}
	void Grow(std::size_t nBytes);
public:
	bool IsClosed() const { return m_lpBuf == NULL;}
	void Flush(){}

	ZIP_ULONGLONG Seek(ZIP_LONGLONG lOff, int nFrom);
	ZIP_ULONGLONG GetLength() const {return m_nDataSize;}
	void Write(const void* lpBuf, UINT nCount);
	UINT Read(void* lpBuf, UINT nCount);
	void SetLength(ZIP_ULONGLONG nNewLen);
	CZipString GetFilePath() const  {return _T("");}
	CZipMemFile(long nGrowBy = 1024)
	{
		Init();
		m_nGrowBy = nGrowBy;
		m_bAutoDelete = true;
	}

	CZipMemFile(BYTE* lpBuf, UINT nBufSize, long nGrowBy = 0)
	{
		Init();
		Attach(lpBuf, nBufSize, nGrowBy);
	}
	ZIP_ULONGLONG GetPosition() const {	return m_nPos;}
	void Attach(BYTE* lpBuf, UINT nBufSize, long nGrowBy = 0)
	{
		Close();
		m_lpBuf = lpBuf;
		m_nGrowBy = nGrowBy;
		m_nBufSize = nBufSize;
		m_nDataSize = nGrowBy == 0 ? nBufSize : 0;
		m_bAutoDelete = false;
	}
	BYTE* Detach()
	{
		BYTE* b = m_lpBuf;
		Init();
		return b;
	}
	void Close()
	{
		if (m_bAutoDelete)
			Free();
		Init();
	}
	virtual ~CZipMemFile(){Close();}

};

#endif // !defined(AFX_ZIPMEMFILE_H__EA73AB25_6B51_4C5E_8D78_BAC95812598F__INCLUDED_)
