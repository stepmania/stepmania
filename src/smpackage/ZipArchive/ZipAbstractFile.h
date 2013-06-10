///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipAbstractFile.h $
// $Archive: /ZipArchive/ZipAbstractFile.h $
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

#if !defined(AFX_ZIPABSTRACTFILE_H__46F247DE_21A6_4D12_AF64_B5A6B3CF4D57__INCLUDED_)
#define AFX_ZIPABSTRACTFILE_H__46F247DE_21A6_4D12_AF64_B5A6B3CF4D57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZipExport.h"
#include "ZipString.h"

class ZIP_API CZipAbstractFile
{
public:

	enum {	begin	= SEEK_SET, // 0
			current = SEEK_CUR, // 1
			end		= SEEK_END  // 2
	};
	CZipAbstractFile(){}
	virtual bool Open(LPCTSTR , UINT , bool ){return true;}
	virtual void Close() = 0;
	virtual void Flush() = 0;
	virtual ZIP_ULONGLONG GetPosition() const = 0;	
	virtual ZIP_ULONGLONG Seek(ZIP_LONGLONG lOff, int nFrom) = 0;
	virtual ZIP_ULONGLONG GetLength() const = 0;
	virtual void SetLength(ZIP_ULONGLONG nNewLen) = 0;	
	virtual ZIP_ULONGLONG SeekToBegin(){return Seek(0, begin);}
	virtual ZIP_ULONGLONG SeekToEnd(){return Seek(0, end);}
	virtual CZipString GetFilePath() const = 0;	
	virtual UINT Read(void *lpBuf, UINT nCount) = 0;
	virtual void Write(const void* lpBuf, UINT nCount) = 0;	
	virtual bool IsClosed() const = 0;	
	virtual ~CZipAbstractFile(){};

};



#endif // !defined(AFX_ZIPABSTRACTFILE_H__46F247DE_21A6_4D12_AF64_B5A6B3CF4D57__INCLUDED_)
