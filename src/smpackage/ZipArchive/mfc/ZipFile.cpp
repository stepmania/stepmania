////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFile.cpp $
// $Archive: /ZipArchive/ZipFile.cpp $
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

#include "stdafx.h"
#include "ZipFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// IMPLEMENT_DYNAMIC(CZipAbstractFile, CFile)
IMPLEMENT_DYNAMIC(CZipFile, CFile)

CZipFile::CZipFile()
{
}

CZipFile::~CZipFile()
{

}

CZipFile::operator HANDLE()
{
	return (HANDLE)m_hFile;
}

// __int64 CZipFile::Seek(__int64 dOff, UINT nFrom)
// {
// 	ASSERT_VALID(this);
// 	ASSERT(m_hFile != (UINT)hFileNull);
// 	ASSERT(nFrom == begin || nFrom == end || nFrom == current);
// 	ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);
// 	LARGE_INTEGER li;
// 	li.QuadPart = dOff;
// 
// 	li.LowPart  = ::SetFilePointer((HANDLE)m_hFile, li.LowPart, &li.HighPart, (DWORD)nFrom);
// 	DWORD dw = GetLastError();
// 	if ((li.LowPart == (DWORD)-1) && (dw != NO_ERROR))
// 	{
// 		CFileException::ThrowOsError((LONG)dw);
// 	}
// 
// 	return li.QuadPart;
// }
