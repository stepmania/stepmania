////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFile.h $
// $Archive: /ZipArchive/ZipFile.h $
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
#include "ZipExport.h"

class ZIP_API CZipFile :public CZipAbstractFile, public CFile
{
public:
	DECLARE_DYNAMIC(CZipFile)
	void Flush(){CFile::Flush();}
	ZIP_ULONGLONG GetPosition() const {return CFile::GetPosition() ;}
	CZipString GetFilePath() const {return CFile::GetFilePath();}
	void SetLength(ZIP_ULONGLONG nNewLen) {CFile::SetLength(nNewLen);}
	UINT Read(void *lpBuf, UINT nCount){return CFile::Read(lpBuf, nCount);}
	void Write(const void* lpBuf, UINT nCount){CFile::Write(lpBuf, nCount);}
	ZIP_ULONGLONG Seek(ZIP_LONGLONG lOff , int nFrom){return CFile::Seek(lOff, nFrom);}
	ZIP_ULONGLONG GetLength() const {return CFile::GetLength();}
	bool Open( LPCTSTR lpszFileName, UINT nOpenFlags, bool bThrowExc)
	{
		CFileException* e = new CFileException;
		bool bRet = CFile::Open(lpszFileName, nOpenFlags, e) != 0;
		if (!bRet && bThrowExc)
			throw e;
		e->Delete();
		return bRet;

	}
	CZipFile();
	bool IsClosed() const 
	{
		return m_hFile == CFile::hFileNull;
	}


	CZipFile( LPCTSTR lpszFileName, UINT nOpenFlags ):CFile(lpszFileName, nOpenFlags)
	{
	}
	void Close( )
	{
 		if (!IsClosed())
			CFile::Close();
	}
	operator HANDLE();
	virtual ~CZipFile();

};

#endif // !defined(AFX_ZIPFILE_H__80609DE0_2C6D_4C94_A90C_0BE34A50C769__INCLUDED_)
