////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipException.cpp $
// $Archive: /ZipArchive/ZipException.cpp $
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
#include "ZipException.h"
#include <errno.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifdef _MFC_VER
	IMPLEMENT_DYNAMIC( CZipException, CException)
#endif

CZipException::CZipException(int iCause, LPCTSTR lpszZipName)
#ifdef _MFC_VER
	:CException(TRUE)
#endif
{
	m_iCause = iCause;

	if (lpszZipName)
		m_szFileName = lpszZipName;	
}

CZipException::~CZipException()
{

}

// inline void CZipException::Throw(int iZipError, LPCTSTR lpszZipName)
// {
// #ifdef _MFC_VER
// 	throw new CZipException(iZipError, lpszZipName);
// #else
// 	CZipException e(iZipError, lpszZipName);
// 	throw e;
// #endif
// MSVC++: ignore "Unreachable code" warning here, it's due to 
// optimizations
// }

int CZipException::ZlibErrToZip(int iZlibError)
{
	switch (iZlibError)
	{
	case 2://Z_NEED_DICT:
		return CZipException::needDict;
	case 1://Z_STREAM_END:
		return CZipException::streamEnd;
	case -1://Z_ERRNO:
		return CZipException::errNo;
	case -2://Z_STREAM_ERROR:
		return CZipException::streamError;
	case -3://Z_DATA_ERROR:
		return CZipException::dataError;
	case -4://Z_MEM_ERROR:
		return CZipException::memError;
	case -5://Z_BUF_ERROR:
		return CZipException::bufError;
	case -6://Z_VERSION_ERROR:
		return CZipException::versionError;
	default:
		return CZipException::generic;
	}
	
}

#ifdef ZIP_ENABLE_ERROR_DESCRIPTION

BOOL CZipException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
	UINT* )

{
	if (!lpszError || !nMaxError)
		return FALSE;
	CZipString sz = GetErrorDescription();
	if (sz.IsEmpty())
		return FALSE;
	UINT iLen = sz.GetLength();
	if (nMaxError - 1 < iLen)
		iLen = nMaxError - 1;
	LPTSTR lpsz = sz.GetBuffer(iLen);
#ifdef _UNICODE	
	wcsncpy(lpszError, lpsz, iLen);
#else
	strncpy(lpszError, lpsz, iLen);
#endif
	lpszError[iLen] = _T('\0');
	return TRUE;
}


CZipString CZipException::GetErrorDescription()
{
	return GetInternalErrorDescription(m_iCause);
}


CZipString CZipException::GetSystemErrorDescription()
{
#ifdef WIN32
	DWORD x = GetLastError();
	if (x)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,    
			          NULL, x, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				      (LPTSTR) &lpMsgBuf, 0, NULL);
		CZipString sz = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
		return sz;
	}
#endif
	return GetInternalErrorDescription(errno == 0 ? generic : errno, true);
}

CZipString CZipException::GetInternalErrorDescription(int iCause, bool bNoLoop)
{
	CZipString sz;
	switch (iCause)
	{
		case EROFS:
			sz = _T("Read-only file system");
			break;
		case ESPIPE:
			sz = _T("Illegal seek");
			break;
		case ENOSPC:
			sz = _T("No space left on device");
			break;
		case EFBIG:
			sz = _T("File too large");
			break;
		case EMFILE:
			sz = _T("Too many open files");
			break;
		case ENFILE:
			sz = _T("File table overflow");
			break;
		case EINVAL:
			sz = _T("Invalid argument");
			break;
		case EISDIR:
			sz = _T("Is a directory");
			break;
		case ENOTDIR:
			sz = _T("Not a directory");
			break;
		case ENODEV:
			sz = _T("No such device");
			break;
		case EXDEV:
			sz = _T("Cross-device link");
			break;
		case EEXIST:
			sz = _T("File exists");
			break;
		case EFAULT:
			sz = _T("Bad address");
			break;
		case EACCES:
			sz = _T("Permission denied");
			break;
		case ENOMEM:
			sz = _T("Not enough space");
			break;
		case EBADF:
			sz = _T("Bad file number");
			break;
		case ENXIO:
			sz = _T("No such device or address");
			break;
		case EIO:
			sz = _T("I/O error");
			break;
		case EINTR:
			sz = _T("Interrupted system call");
			break;
		case ENOENT:
			sz = _T("No such file or directory");
			break;
		case EPERM:
			sz = _T("Not super-user");
			break;
		case badZipFile:
			sz = _T("Damaged or not a zip file");
			break;
		case badCrc:
			sz = _T("Crc mismatched");
			break;
		case noCallback:
			sz = _T("No disk-spanning callback functor set");
			break;
		case aborted:
			sz = _T("Disk change aborted");
			break;
		case abortedAction:
			sz = _T("Action aborted");
			break;
		case abortedSafely:
			sz = _T("Action aborted safely");
			break;
		case nonRemovable:
			sz = _T("The device selected for the disk spanning archive is non removable");
			break;
		case tooManyVolumes:
			sz = _T("Limit of the maximum volumes reached (999)");
			break;
		case tooLongFileName:
			sz = _T("The filename of the file being added to the archive is too long");
			break;
		case badPassword:
			sz = _T("Incorrect password set for the file being decrypted");
			break;
		case dirWithSize:
			sz = _T("During testing found the directory with the size greater than 0");
			break;
		case internal:
			sz = _T("Internal error");
			break;
		case notRemoved:
			sz.Format(_T("%s (%s)"), _T("Error while removing a file"), (LPCTSTR)GetSystemErrorDescription());
			break;
		case notRenamed:
			sz.Format(_T("%s (%s)"), _T("Error while renaming a file"), (LPCTSTR)GetSystemErrorDescription());
			break;
		case platfNotSupp:
			sz = _T("Cannot create the file for the specified platform");
			break;
		case cdirNotFound:
			sz = _T("The central directory was not found in the archive (or you were trying to open not the last disk of a multi-disk archive)");
			break;
		case streamEnd:
			sz = _T("Zlib Library error (end of stream)");
			break;
		case errNo:
			sz = GetInternalErrorDescription(errno != errNo ? errno : generic);
			break;
		case streamError:
			sz = _T("Zlib library error (stream error)");
			break;
		case dataError:
			sz = _T("Zlib library error (data error)");
			break;
		case memError:
			sz = _T("Not enough memory");
			break;
		case bufError:
			sz = _T("Zlib library error (buffer error)");
			break;
		case versionError:
			sz = _T("Zlib library error (version error)");
			break;
		default:
			sz = bNoLoop ? _T("Unknown error") :(LPCTSTR) GetSystemErrorDescription();
	}
	return sz;
}

#endif //ZIP_ENABLE_ERROR_DESCRIPTION
