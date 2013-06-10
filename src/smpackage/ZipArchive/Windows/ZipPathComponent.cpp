////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipPathComponent.cpp $
// $Archive: /ZipArchive/ZipPathComponent.cpp $
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
#include "ZipPathComponent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CZipPathComponent::~CZipPathComponent()
{

}

void CZipPathComponent::SetFullPath(LPCTSTR lpszFullPath)
{

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	
	
	CZipString szTempPath(lpszFullPath);
	const CZipString szPrefix = _T("\\\\?\\unc\\");
	int i = -1, iLen = szPrefix.GetLength();
	if (iLen > szTempPath.GetLength())
		iLen = szTempPath.GetLength();
	CZipString szPossiblePrefix = szTempPath.Left(iLen);
	szPossiblePrefix.MakeLower(); // must perform case insensitive comparison
	while (++i < iLen && szPossiblePrefix[i] == szPrefix[i]); 
	if (i == 2 || i == 4 || i == 8) // unc path, unicode path or unc path meeting windows file name conventions
	{
		m_szPrefix = szTempPath.Left(i);
		szTempPath = szTempPath.Mid(i);		
	}
	else
		m_szPrefix.Empty();

	_tsplitpath(szTempPath, szDrive , szDir, szFname, szExt);
	m_szDrive = szDrive;
	m_szDirectory = szDir;
	
	m_szDirectory.TrimLeft(m_cSeparator);
	m_szDirectory.TrimRight(m_cSeparator);
	SetExtension(szExt);
	m_szFileTitle = szFname;
}


CZipString CZipPathComponent::GetNoDrive() const
{
	CZipString szPath = m_szDirectory;
	CZipString szFileName = GetFileName();
	if (!szFileName.IsEmpty() && !szPath.IsEmpty())
		szPath += m_cSeparator;

	szPath += szFileName;
	return szPath;	
}

