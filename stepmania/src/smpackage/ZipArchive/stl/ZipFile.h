////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFile.h $
// $Archive: /ZipArchive_STL/ZipFile.h $
// $Date$ $Author$
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

#if !defined(AFX_ZIPFILE_H__80609DE0_2C6D_4C94_A90C_0BE34A50C769__INCLUDED_)
#define AFX_ZIPFILE_H__80609DE0_2C6D_4C94_A90C_0BE34A50C769__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZipAbstractFile.h"
#include "ZipString.h"
#include "ZipExport.h"

#ifndef __GNUC__
	#include <io.h>
#else
	#include <unistd.h>
	#include <errno.h>
#endif

class ZIP_API CZipFile :public CZipAbstractFile
{
	void ThrowError() const;
public:
	int m_hFile;
	operator HANDLE();
	enum OpenModes
	{
		modeRead =          0x0001,
		modeWrite =         0x0002,
		modeReadWrite =     modeRead | modeWrite,
		shareDenyWrite =    0x0004,
		shareDenyRead =     0x0008,
		shareDenyNone =     0x0010,
		modeCreate =        0x0020,
		modeNoTruncate =    0x0040,
	};
	
	CZipFile(LPCTSTR lpszFileName, UINT openFlags)
	{
		m_hFile = -1;
		Open(lpszFileName, openFlags, true);
	}
	void Flush();
	ZIP_ULONGLONG GetLength() const;
	CZipString GetFilePath() const {return m_szFileName;}
	bool IsClosed()const { return m_hFile == -1;}
	bool Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow);
	void Close() 
	{
		if (IsClosed())
			return;

		if (close(m_hFile) != 0)
			ThrowError();
		else
		{
			m_szFileName.empty();
			m_hFile = -1;
		}
	}
	void Write(const void* lpBuf, UINT nCount)
	{
		if (write(m_hFile, lpBuf, nCount) != (int) nCount)
			ThrowError();
	}
	ZIP_ULONGLONG GetPosition() const
	{
#ifndef __GNUC__
		long ret = tell(m_hFile);
#else
		long ret = lseek(m_hFile, 0, SEEK_CUR);
#endif
		if (ret == -1L)
			ThrowError();
		return ret;
	}
	void SetLength(ZIP_ULONGLONG nNewLen);
	UINT Read(void *lpBuf, UINT nCount)
	{
		errno = 0;
		int ret = read(m_hFile, lpBuf, nCount);
		if (ret < (int) nCount && errno != 0)
			ThrowError();
		return ret;

	}
	ZIP_ULONGLONG Seek(ZIP_LONGLONG dOff, int nFrom)
	{
		long ret = lseek(m_hFile, (long)dOff, nFrom);
		if (ret == -1)
			ThrowError();
		return ret;
	}
	CZipFile ();
	virtual ~CZipFile (){Close();};
protected:
	CZipString m_szFileName;

};

#endif // !defined(AFX_ZIPFILE_H__80609DE0_2C6D_4C94_A90C_0BE34A50C769__INCLUDED_)
