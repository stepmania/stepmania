////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipPlatform.cpp $
// $Archive: /ZipArchive/ZipPlatform.cpp $
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


#include "stdafx.h"
#include "ZipPlatform.h"
#include "ZipFileHeader.h"
#include "ZipException.h"
#include "ZipAutoBuffer.h"
#include <sys/stat.h>

#if defined _MSC_VER && !defined __BORLANDC__ /*_MSC_VER may be defined in Borland after converting the VC project */
        #include <sys/utime.h>
#else
        #include <utime.h>
#endif

#include <cstddef>
#include <direct.h>
#include <io.h>
#include <time.h>
#include "ZipPathComponent.h"
#include "ZipCompatibility.h"

const TCHAR CZipPathComponent::m_cSeparator = _T('\\');


#ifndef _UTIMBUF_DEFINED
#define _utimbuf utimbuf
#endif

DWORD ZipPlatform::GetDeviceFreeSpace(LPCTSTR lpszPath)
{
	DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
	CZipPathComponent zpc (lpszPath);
	CZipString szDrive = zpc.GetFileDrive();
	if (!GetDiskFreeSpace(
		szDrive,
		&SectorsPerCluster,
		&BytesPerSector,
		&NumberOfFreeClusters,
		&TotalNumberOfClusters))
	{
		CZipPathComponent::AppendSeparator(szDrive); // in spite of what is written in MSDN it is sometimes needed (on fixed disks)
		if (!GetDiskFreeSpace(
			szDrive,
			&SectorsPerCluster,
			&BytesPerSector,
			&NumberOfFreeClusters,
			&TotalNumberOfClusters))

				return 0;
	}
	__int64 total = SectorsPerCluster * BytesPerSector * NumberOfFreeClusters;
	return (DWORD)total;
}

bool ZipPlatform::GetFileSize(LPCTSTR lpszFileName, DWORD& dSize)
{
	HANDLE f = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (!f)
		return false;
	DWORD dwSize;
	dwSize = ::GetFileSize(f, NULL);
	CloseHandle(f);
	if (dwSize == 0xFFFFFFFF)
		return false;
	dSize = dwSize;
	return true;
}

CZipString ZipPlatform::GetTmpFileName(LPCTSTR lpszPath, DWORD iSizeNeeded)
{
		TCHAR empty[] = _T("");
		CZipString tempPath;
		bool bCheckTemp = true;
		if (lpszPath)
		{
			tempPath = lpszPath;
			bCheckTemp = GetDeviceFreeSpace(tempPath) < iSizeNeeded;

		}
		if (bCheckTemp)
		{
			DWORD size = GetTempPath(0, NULL);
			if (size == 0)
				return empty;

			GetTempPath(size, tempPath.GetBuffer(size));
			tempPath.ReleaseBuffer();
			if (GetDeviceFreeSpace(tempPath) < iSizeNeeded)
			{
				if (!GetCurrentDirectory(tempPath) || GetDeviceFreeSpace(tempPath) < iSizeNeeded)
					return empty;
			}
		}
		CZipString tempName;
		if (!GetTempFileName(tempPath, _T("ZAR"), 0, tempName.GetBuffer(_MAX_PATH)))
			return empty;
		tempName.ReleaseBuffer();
		return tempName;
}


bool ZipPlatform::GetCurrentDirectory(CZipString& sz)
{
	DWORD i = ::GetCurrentDirectory(0, NULL);
	if (!i)
		return false;
	TCHAR* pBuf = new TCHAR[i];
	bool b = true;
	if (!::GetCurrentDirectory(i, pBuf))
		b = false;
	else
		sz = pBuf;
	delete[] pBuf;
	return b;
}

bool ZipPlatform::SetFileAttr(LPCTSTR lpFileName, DWORD uAttr)
{
	return ::SetFileAttributes(lpFileName, uAttr) != 0;
}

bool ZipPlatform::GetFileAttr(LPCTSTR lpFileName, DWORD& uAttr)
{
	// not using MFC due to MFC bug (attr is one byte there)
	DWORD temp = ::GetFileAttributes(lpFileName);
	if (temp == -1)
		return false;
	uAttr = temp;
	return true;

}

bool ZipPlatform::GetFileModTime(LPCTSTR lpFileName, time_t & ttime)
{
#if defined _MSC_VER && !defined __BORLANDC__ /*_MSC_VER may be defined in Borland after converting the VC project */
    struct _stat st;
    if (_tstat(lpFileName, &st) != 0)
#else
    struct stat st;
    if (stat(lpFileName, &st) != 0)
#endif
	return false;

	ttime = st.st_mtime;
	return ttime != -1;
}

bool ZipPlatform::SetFileModTime(LPCTSTR lpFileName, time_t ttime)
{
	struct _utimbuf ub;
	ub.actime = time(NULL);
	ub.modtime = ttime == -1 ? time(NULL) : ttime; // if wrong file time, set it to the current
	return _tutime(lpFileName, &ub) == 0;
}


bool ZipPlatform::ChangeDirectory(LPCTSTR lpDirectory)
{
	return _tchdir(lpDirectory) == 0; // returns 0 if ok
}
int ZipPlatform::FileExists(LPCTSTR lpszName)
{
	if (_taccess(lpszName, 0) == 0)
	{
		if (DirectoryExists(lpszName))
			return -1;
		return 1;
	}
	else
		return 0;

}

ZIPINLINE  bool ZipPlatform::IsDriveRemovable(LPCTSTR lpszFilePath)
{
	CZipPathComponent zpc(lpszFilePath);
	return ::GetDriveType(zpc.GetFileDrive()) == DRIVE_REMOVABLE;
}

ZIPINLINE  bool ZipPlatform::SetVolLabel(LPCTSTR lpszPath, LPCTSTR lpszLabel)
{
	CZipPathComponent zpc(lpszPath);
	CZipString szDrive = zpc.GetFileDrive();
	CZipPathComponent::AppendSeparator(szDrive);
	return ::SetVolumeLabel(szDrive, lpszLabel) != 0;
}

ZIPINLINE void ZipPlatform::AnsiOem(CZipAutoBuffer& buffer, bool bAnsiToOem)
{
	if (bAnsiToOem)
		CharToOemBuffA(buffer, buffer, buffer.GetSize());
	else
		OemToCharBuffA(buffer, buffer, buffer.GetSize());
}

ZIPINLINE  bool ZipPlatform::RemoveFile(LPCTSTR lpszFileName, bool bThrow)
{
	if (!::DeleteFile((LPTSTR)lpszFileName))
		if (bThrow)
			CZipException::Throw(CZipException::notRemoved, lpszFileName);
		else
			return false;
	return true;

}
ZIPINLINE  bool ZipPlatform::RenameFile( LPCTSTR lpszOldName, LPCTSTR lpszNewName, bool bThrow)
{
	if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))
		if (bThrow)
			CZipException::Throw(CZipException::notRenamed, lpszOldName);
		else
			return false;
	return true;

}
ZIPINLINE  bool ZipPlatform::IsDirectory(DWORD uAttr)
{
	return (uAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
ZIPINLINE  bool ZipPlatform::CreateDirectory(LPCTSTR lpDirectory)
{
	return ::CreateDirectory(lpDirectory, NULL) != 0;
}

ZIPINLINE  DWORD ZipPlatform::GetDefaultAttributes()
{
	return 0x81a40020; // make it readable under Unix
}

ZIPINLINE  DWORD ZipPlatform::GetDefaultDirAttributes()
{
	return 0x41ff0010; // make it readable under Unix
}

ZIPINLINE  int ZipPlatform::GetSystemID()
{
	return ZipCompatibility::zcDosFat;
}

ZIPINLINE bool ZipPlatform::GetSystemCaseSensitivity()
{
	return false;
}

#ifdef _UNICODE
int ZipPlatform::WideToSingle(LPCTSTR lpWide, CZipAutoBuffer &szSingle)
{
	std::size_t wideLen = wcslen(lpWide);
	if (wideLen == 0)
	{
		szSingle.Release();
		return 0;
	}

	// iLen does not include terminating character
	int iLen = WideCharToMultiByte(CP_ACP,0, lpWide, wideLen, szSingle,
		0, NULL, NULL);
	if (iLen > 0)
	{
		szSingle.Allocate(iLen, true);
		iLen = WideCharToMultiByte(CP_ACP,0, lpWide , wideLen, szSingle,
			iLen, NULL, NULL);
		ASSERT(iLen != 0);
	}
	else // here it means error
	{
		szSingle.Release();
		iLen --;
	}
	return iLen;

}
int ZipPlatform::SingleToWide(const CZipAutoBuffer &szSingle, CZipString& szWide)
{
	int singleLen = szSingle.GetSize();
		// iLen doesn't include terminating character
	int iLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSingle.GetBuffer(), singleLen, NULL, 0);
	if (iLen > 0)
	{
		iLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSingle.GetBuffer(), singleLen,
			szWide.GetBuffer(iLen) , iLen);
		szWide.ReleaseBuffer(iLen);
		ASSERT(iLen != 0);
	}
	else
	{
		szWide.Empty();
		iLen --; // return -1
	}
	return iLen;

}
#endif

#ifndef _MFC_VER
#include <io.h>
#include <share.h>
bool ZipPlatform::TruncateFile(int iDes, DWORD iSize)
{
	int ret = chsize(iDes, iSize);
	return ret != -1;

}

int ZipPlatform::OpenFile(LPCTSTR lpszFileName, UINT iMode, int iShareMode)
{
	switch (iShareMode)
	{
	case (CZipFile::shareDenyWrite & CZipFile::shareDenyRead):
		iShareMode = SH_DENYRW;
		break;
	case (CZipFile::shareDenyRead):
		iShareMode = SH_DENYRD;
		break;
	case (CZipFile::shareDenyWrite):
		iShareMode = SH_DENYWR;
		break;
	default:
		iShareMode = SH_DENYNO;
	}
	return  _tsopen(lpszFileName, iMode, iShareMode, S_IREAD | S_IWRITE /*required only when O_CREAT mode*/);
}

bool ZipPlatform::FlushFile(int iDes)
{
	return _commit(iDes) == 0;
}

int ZipPlatform::GetFileSystemHandle(int iDes)
{
	return _get_osfhandle(iDes);
}


#endif //_MFC_VER
