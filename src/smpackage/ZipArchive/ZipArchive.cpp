///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipArchive.cpp $
// $Archive: /ZipArchive/ZipArchive.cpp $
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
#include "ZipArchive.h"
// #include "ZipPathComponent.h"
#include "ZipPlatform.h"
#include "ZipCompatibility.h"

#include <cmath>
#include <cstddef>
#include <time.h>

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define ZIP_COMPR_REPL_MASK 0xffffff00
#define ZIP_COMPR_REPL_SIGN 0x0100 // first 8 bits should be 00 (reserved for compression level), next 8 should be different from ff (to distinguish from -1)

const TCHAR CZipArchive::m_gszCopyright[] = {_T("ZipArchive library Copyright 2000 - 2003 Tadeusz Dracz")};


void CZipAddNewFileInfo::Defaults()
{
	m_iSmartLevel = CZipArchive::zipsmSafeSmart;
	m_iReplaceIndex = -1;
	m_nBufSize = 65536;
	m_iComprLevel = -1; // default

}

CZipArchive::CZipArchive()
{

	m_bRemoveDriveLetter = m_bDetectZlibMemoryLeaks = true;
	m_bIgnoreCRC = m_bAutoFlush = false;
	m_centralDir.m_pStorage= &m_storage;
	m_info.m_stream.zalloc = (alloc_func)_zliballoc;
	m_info.m_stream.zfree = (free_func)_zlibfree;
	m_iFileOpened = nothing;
	SetCaseSensitivity(ZipPlatform::GetSystemCaseSensitivity());
}


CZipArchive::~CZipArchive()
{
	// 	Close(); // cannot be here: if an exception is thrown strange things can happen
	EmptyPtrList();

}



void CZipArchive::Open(LPCTSTR szPathName, int iMode, int iVolumeSize)
{
	if (!IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive already opened.\n"),__FILE__,__LINE__);
		return;
	}
	m_storage.Open(szPathName, iMode, iVolumeSize);
	OpenInternal(iMode);
}

void CZipArchive::Open(CZipMemFile& mf,int iMode)
{
	if (!IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive already opened.\n"),__FILE__,__LINE__);
		return;
	}
	if (iMode != zipOpen && iMode != zipOpenReadOnly && iMode != zipCreate)
	{
		TRACE(_T("%s(%i) : Mode not supported.\n"),__FILE__,__LINE__);
		return;
	}
	m_storage.Open(mf, iMode);
	OpenInternal(iMode);
}


void CZipArchive::OpenInternal(int iMode)
{
	m_pszPassword.Release();
	m_iFileOpened = nothing;
	m_centralDir.Init();
	m_iArchiveSystCompatib = ZipPlatform::GetSystemID();
	m_szRootPath.Empty();
	if ((iMode == zipOpen) ||(iMode == zipOpenReadOnly))
	{
		m_centralDir.Read();
		// if there is at least one file, get system comp. from the first one
		if (m_centralDir.IsValidIndex(0))
		{
			int iSystemComp = m_centralDir[0]->GetSystemCompatibility();
			if (ZipCompatibility::IsPlatformSupported(iSystemComp))
				m_iArchiveSystCompatib = iSystemComp;
		}
	}

}


bool CZipArchive::IsClosed(bool bArchive) const
{
	return  bArchive ?(m_storage.GetCurrentDisk() == -1):(!m_storage.m_pFile || m_storage.m_pFile->IsClosed());
}


void CZipArchive::ThrowError(int err, bool bZlib)
{
	if (bZlib)
		err = CZipException::ZlibErrToZip(err);
	CZipException::Throw(err, IsClosed() ? _T("") : (LPCTSTR)m_storage.m_pFile->GetFilePath());
}



bool CZipArchive::GetFileInfo(CZipFileHeader & fhInfo, WORD uIndex) const
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}

	if (!m_centralDir.IsValidIndex(uIndex))
		return false;

	fhInfo = *(m_centralDir[uIndex]);
	m_centralDir.ConvertFileName(true, false, &fhInfo);
	return true;
}

int CZipArchive::FindFile(LPCTSTR lpszFileName, int iCaseSensitive, bool bFileNameOnly)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return (int)-1;
	}
	bool bCS;
	bool bSporadically;
	switch (iCaseSensitive)
	{
	case ffCaseSens:
		bCS = true;
		bSporadically = true;
		break;
	case ffNoCaseSens:
		bCS = false;
		bSporadically = true;
		break;
	default:
		bCS = m_bCaseSensitive;
		bSporadically = false;
	}
	return m_centralDir.FindFile(lpszFileName, bCS, bSporadically, bFileNameOnly);
}

bool CZipArchive::OpenFile(WORD uIndex)
{
	if (!m_centralDir.IsValidIndex(uIndex))
	{
		ASSERT(FALSE);
		return false;
	}
	if (m_storage.IsSpanMode() == 1)
	{
		TRACE(_T("%s(%i) : You cannot extract from the span in creation.\n"),__FILE__,__LINE__);
		return false;
	}


	if (m_iFileOpened)
	{
		TRACE(_T("%s(%i) : A file already opened.\n"),__FILE__,__LINE__);
		return false;
	}

	m_info.Init();
	m_centralDir.OpenFile(uIndex);
	if (CurrentFile()->IsEncrypted())
	{

		if (m_pszPassword.GetSize() == 0)
		{
			TRACE(_T("%s(%i) : Password not set for the encrypted file.\n"),__FILE__,__LINE__);
				ThrowError(CZipException::badPassword);
		}
		CryptInitKeys();
		if (!CryptCheck())
			ThrowError(CZipException::badPassword);

	}
	else if (m_pszPassword.GetSize() != 0)
	{
		TRACE(_T("%s(%i) : Password set for a not encrypted file. Ignoring password.\n"),__FILE__,__LINE__);
	}

	WORD uMethod = CurrentFile()->m_uMethod;

	if ((uMethod != 0) &&(uMethod != Z_DEFLATED))
		ThrowError(CZipException::badZipFile);

	if (uMethod == Z_DEFLATED)
	{
		m_info.m_stream.opaque =  m_bDetectZlibMemoryLeaks ? &m_list : 0;
		int err = inflateInit2(&m_info.m_stream, -MAX_WBITS);
		//			* windowBits is passed < 0 to tell that there is no zlib header.
		//          * Note that in this case inflate *requires* an extra "dummy" byte
		//          * after the compressed stream in order to complete decompression and
		//          * return Z_STREAM_END.
		CheckForError(err);
	}
	m_info.m_uComprLeft = CurrentFile()->m_uComprSize;
	if (CurrentFile()->IsEncrypted())
		m_info.m_uComprLeft -= ZIPARCHIVE_ENCR_HEADER_LEN;
	m_info.m_uUncomprLeft = CurrentFile()->m_uUncomprSize;
	m_info.m_uCrc32 = 0;
	m_info.m_stream.total_out = 0;
	m_info.m_stream.avail_in = 0;

	m_iFileOpened = extract;
	return true;
}


int CZipArchive::GetLocalExtraField(char *pBuf, int iSize)const
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return -1;
	}

	if (m_iFileOpened != extract)
	{
		TRACE(_T("%s(%i) : A file must be opened to get the local extra field.\n"),__FILE__,__LINE__);
		return -1;
	}

	int size = m_centralDir.m_pLocalExtraField.GetSize();
	if (!pBuf|| !size)
		return size;

	if (iSize < size)
		size = iSize;

	memcpy(pBuf, m_centralDir.m_pLocalExtraField, size);
	return size;
}

void* CZipArchive::_zliballoc(void* opaque, UINT items, UINT size)
{
	void* p = new char[size * items];
	if (opaque)
	{
		CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
		list->AddTail(p);
	}
	return p;
}

void CZipArchive::_zlibfree(void* opaque, void* address)
{
	if (opaque)
	{
		CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
		CZipPtrListIter iter = list->Find(address);
		if (list->IteratorValid(iter))
			list->RemoveAt(iter);
	}
	delete[] (char*) address;
}

void CZipArchive::CheckForError(int iErr)
{
	if ((iErr == Z_OK) ||(iErr == Z_NEED_DICT))
		return;

	ThrowError(iErr, true);
}

CZipFileHeader* CZipArchive::CurrentFile()
{
	ASSERT(m_centralDir.m_pOpenedFile);
	return m_centralDir.m_pOpenedFile;
}

DWORD CZipArchive::ReadFile(void *pBuf,
                            DWORD iSize)
{
	if (m_iFileOpened != extract)
	{
		TRACE(_T("%s(%i) : Current file must be opened.\n"),__FILE__,__LINE__);
		return 0;
	}

	if (!pBuf || !iSize)
		return 0;

	m_info.m_stream.next_out = (Bytef*)pBuf;
	m_info.m_stream.avail_out = iSize > m_info.m_uUncomprLeft
		? m_info.m_uUncomprLeft : iSize;


	DWORD iRead = 0;

	// may happen when the file is 0 sized
	bool bForce = m_info.m_stream.avail_out == 0 && m_info.m_uComprLeft > 0;
	while (m_info.m_stream.avail_out > 0 || (bForce && m_info.m_uComprLeft > 0))
	{
		if ((m_info.m_stream.avail_in == 0) &&
			(m_info.m_uComprLeft >= 0)) // Also when there are zero bytes left!
		{
			DWORD uToRead = m_info.m_pBuffer.GetSize();
			if (m_info.m_uComprLeft < uToRead)
				uToRead = m_info.m_uComprLeft;

			if (uToRead == 0)
			{
				uToRead = 1; // Add dummy byte at end of compressed data.
			}
			else
			{
				m_storage.Read(m_info.m_pBuffer, uToRead, false);
				CryptDecodeBuffer(uToRead);
			}

			m_info.m_uComprLeft -= uToRead;

			m_info.m_stream.next_in = (Bytef*)(char*)m_info.m_pBuffer;
			m_info.m_stream.avail_in = uToRead;
		}

		if (CurrentFile()->m_uMethod == 0)
		{
			DWORD uToCopy = m_info.m_stream.avail_out < m_info.m_stream.avail_in
				? m_info.m_stream.avail_out : m_info.m_stream.avail_in;

			memcpy(m_info.m_stream.next_out, m_info.m_stream.next_in, uToCopy);

			m_info.m_uCrc32 = crc32(m_info.m_uCrc32, m_info.m_stream.next_out, uToCopy);

			m_info.m_uUncomprLeft -= uToCopy;
			m_info.m_stream.avail_in -= uToCopy;
			m_info.m_stream.avail_out -= uToCopy;
			m_info.m_stream.next_out += uToCopy;
			m_info.m_stream.next_in += uToCopy;
            m_info.m_stream.total_out += uToCopy;
			iRead += uToCopy;
		}
		else
		{
			DWORD uTotal = m_info.m_stream.total_out;
			Bytef* pOldBuf =  m_info.m_stream.next_out;
			int err = inflate(&m_info.m_stream, Z_SYNC_FLUSH);
			DWORD uToCopy = m_info.m_stream.total_out - uTotal;

			m_info.m_uCrc32 = crc32(m_info.m_uCrc32, pOldBuf, uToCopy);

			m_info.m_uUncomprLeft -= uToCopy;
			iRead += uToCopy;

			if (err == Z_STREAM_END)
				return iRead;

			CheckForError(err);
		}
	}

	return iRead;
}

void CZipArchive::Close(int iAfterException, bool bUpdateTimeStamp)
{
	// if after an exception - the archive may be closed, but the file may be opened
	if (IsClosed() && (!iAfterException || IsClosed(false)))
	{
		TRACE(_T("%s(%i) : ZipArchive is already closed.\n"),__FILE__,__LINE__);
		return;
	}

	if (m_iFileOpened == extract)
		CloseFile(NULL, iAfterException != afNoException);

	if (m_iFileOpened == compress)
		CloseNewFile(iAfterException != afNoException);

	if (iAfterException != afAfterException && !IsClosed(false)) // in disk spanning when user aborts
		WriteCentralDirectory(false);  // we will flush in CZipStorage::Close

	time_t tNewestTime = 0;

	if (bUpdateTimeStamp)
	{
		int iSize = m_centralDir.m_headers.GetSize();
		for (int i = 0; i< iSize; i++)
		{
			time_t tFileInZipTime = m_centralDir[i]->GetTime();
			if (tFileInZipTime > tNewestTime)
				tNewestTime = tFileInZipTime;
		}
	}
	m_centralDir.Clear();
	CZipString szFileName = m_storage.Close(iAfterException == afAfterException);
	if (bUpdateTimeStamp && !szFileName.IsEmpty())
		ZipPlatform::SetFileModTime(szFileName, tNewestTime);
}

void CZipArchive::WriteCentralDirectory(bool bFlush)
{
	m_centralDir.Write(GetCallback(cbSave));
	if (bFlush)
		m_storage.Flush();
}

void CZipArchive::SetCallback(CZipActionCallback* pCallback, int iWhich)
{
	CallbackType cbs[] = {cbAdd, cbAddTmp, cbAddStore,cbExtract,cbDeleteCnt,cbDelete,cbTest,cbSave, cbGetFromArchive, cbRename, cbReplace};
	int iCount = sizeof(cbs)/sizeof(CallbackType);
	for (int i = 0; i < iCount; i++)
	{
		CallbackType iCallback = cbs[i];
		if (iWhich & iCallback)
			m_callbacks.Set(pCallback, iCallback);
	}
}

void CZipArchive::SetAdvanced(int iWriteBuffer, int iGeneralBuffer, int iSearchBuffer)
{
	if (!IsClosed())
	{
		TRACE(_T("%s(%i) : Set this options before opening the archive.\n"),__FILE__,__LINE__);
		return;
	}

	m_storage.m_iWriteBufferSize = iWriteBuffer < 1024 ? 1024 : iWriteBuffer;
	m_info.m_iBufferSize = iGeneralBuffer < 1024 ? 1024 : iGeneralBuffer;
	m_centralDir.m_iBufferSize = iSearchBuffer < 1024 ? 1024 : iSearchBuffer;
}

int CZipArchive::CloseFile(CZipFile &file)
{
	CZipString temp = file.GetFilePath();
	file.Close();
	return CloseFile(temp);
}

int CZipArchive::CloseFile(LPCTSTR lpszFilePath, bool bAfterException)
{
	if (m_iFileOpened != extract)
	{
		TRACE(_T("%s(%i) : No opened file.\n"),__FILE__,__LINE__);
		return false;
	}

	int iRet = 1;
	if (!bAfterException)
	{
		if (m_info.m_uUncomprLeft == 0)
		{
			if (!m_bIgnoreCRC && m_info.m_uCrc32 != CurrentFile()->m_uCrc32)
				ThrowError(CZipException::badCrc);
		}
		else
			iRet = -1;


		if (CurrentFile()->m_uMethod == Z_DEFLATED)
			inflateEnd(&m_info.m_stream);


		if (lpszFilePath)
		{

			if (!ZipPlatform::SetFileModTime(lpszFilePath, CurrentFile()->GetTime())
				||!ZipPlatform::SetFileAttr(lpszFilePath, CurrentFile()->GetSystemAttr()))
					iRet = -2;
		}

	}

	m_centralDir.CloseFile(bAfterException);

	m_iFileOpened = nothing;
	m_info.ReleaseBuf();
	EmptyPtrList();
	return iRet;
}

bool CZipArchive::OpenNewFile(CZipFileHeader & header,
                              int iLevel,
                              LPCTSTR lpszFilePath, DWORD uInternal)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_iFileOpened)
	{
		TRACE(_T("%s(%i) : A file already opened.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_storage.IsSpanMode() == -1)
	{
		TRACE(_T("%s(%i) : You cannot add files to the existing disk spannig archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (GetCount() ==(WORD)USHRT_MAX)
	{
		TRACE(_T("%s(%i) : Maximum file count inside archive reached.\n"),__FILE__,__LINE__);
		return false;
	}

	DWORD uAttr = 0; // ..compiler
	time_t ttime;
	if (lpszFilePath)
	{

		if (!ZipPlatform::GetFileAttr(lpszFilePath, uAttr))
			// do not continue - if the file was a directory then not recognizing it will cause
			// serious errors (need uAttr to recognize it)
			return false;
		if (!ZipPlatform::GetFileModTime(lpszFilePath, ttime))
			ttime = time(NULL);
	}

	m_info.Init();


	if (lpszFilePath)
	{
		header.SetTime(ttime);
		SetFileHeaderAttr(header, uAttr); // set system compatibility as well
	}
	else
		header.SetSystemCompatibility(m_iArchiveSystCompatib);

	CZipString szFileName = header.GetFileName();


	bool bIsDirectory = header.IsDirectory();
	if (bIsDirectory)
	{
		int iNameLen = szFileName.GetLength();
		if (!iNameLen || !CZipPathComponent::IsSeparator(szFileName[iNameLen-1]))
		{
			szFileName += CZipPathComponent::m_cSeparator;
			header.SetFileName(szFileName);
		}
	}

	if (szFileName.IsEmpty())
	{
		szFileName.Format(_T("file%i"), GetCount());
		header.SetFileName(szFileName);
	}

	// make sure that all slashes are correct (as the current system default)
	// because AddNewFile calls InsertFindFastElement if necessary and
	// the find array keeps all the files already converted to the current system standards
	// we do not perform Oem->Ansi here, because who would pass oem values here?
	//
	ZipCompatibility::SlashBackslashChg(header.m_pszFileName, true);

	bool bEncrypted = m_pszPassword.GetSize() != 0;

#ifdef _DEBUG
	if (bIsDirectory && bEncrypted)
	TRACE(_T("%s(%i) : Encrypting a directory. It's pointless.\n\
	Clear the password before adding a directory.\n"),__FILE__,__LINE__);
#endif



	int iReplaceIndex = -1;
	bool bReplace = (iLevel & 0xffff) == ZIP_COMPR_REPL_SIGN;
	if (bReplace)
	{
		int iMask = ZIP_COMPR_REPL_MASK;
		iReplaceIndex = (iLevel & iMask) >> 16;
		iLevel = (char)(iLevel & ~iMask);
		ASSERT(iLevel == 0);
	}
	else
		uInternal = 0;

	if (iLevel < -1 || iLevel > 9)
		iLevel = -1;

	if (!header.PrepareData(iLevel, m_storage.IsSpanMode() == 1, bEncrypted))
		ThrowError(CZipException::tooLongFileName);

	if (bReplace)
	{
		uInternal += header.GetSize(true);
		if (header.IsEncrypted())
			uInternal += ZIPARCHIVE_ENCR_HEADER_LEN;
		if (header.IsDataDescr())
			uInternal += ZIPARCHIVE_DATADESCRIPTOR_LEN + 4; // CZipCentralDir::CloseNewFile
	}
	m_centralDir.AddNewFile(header, iReplaceIndex);
	if (bReplace)
		MakeSpaceForReplace(iReplaceIndex, uInternal, szFileName);

	// this ensures the conversion will take place anyway (must take because we are going
	// 	to write the local header in a moment
	m_centralDir.ConvertFileName(false, m_centralDir.m_bConvertAfterOpen);

	CurrentFile()->WriteLocal(m_storage);

	// we have written the local header, but if we keep filenames not converted
	// in memory , we have to restore the non-converted value
	if (m_centralDir.m_bConvertAfterOpen)
		CurrentFile()->SetFileName(szFileName);

	if (bEncrypted)
	{
		CZipAutoBuffer buf(ZIPARCHIVE_ENCR_HEADER_LEN);
		// use pseudo-crc since we don't know it yet
		CryptCryptHeader((long)header.m_uModTime << 16, buf);
		m_storage.Write(buf, ZIPARCHIVE_ENCR_HEADER_LEN, false);
	}


	m_info.m_uComprLeft = 0;
    m_info.m_stream.avail_in = (uInt)0;
    m_info.m_stream.avail_out = (uInt)m_info.m_pBuffer.GetSize();
    m_info.m_stream.next_out = (Bytef*)(char*)m_info.m_pBuffer;
    m_info.m_stream.total_in = 0;
    m_info.m_stream.total_out = 0;

	if (bIsDirectory && (CurrentFile()->m_uMethod != 0))
		CurrentFile()->m_uMethod = 0;

	if (CurrentFile()->m_uMethod == Z_DEFLATED)
    {
        m_info.m_stream.opaque = m_bDetectZlibMemoryLeaks ? &m_list : 0;

        int err = deflateInit2(&m_info.m_stream, iLevel,
			Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

		CheckForError(err);
    }
	m_iFileOpened = compress;
	return true;
}


bool CZipArchive::ExtractFile(WORD uIndex,
                              LPCTSTR lpszPath,
                              bool bFullPath,
                              LPCTSTR lpszNewName,
                              DWORD nBufSize)
{

	if (!nBufSize && !lpszPath)
		return false;

	CZipFileHeader header;
	GetFileInfo(header, uIndex); // to ensure that slash and oem conversions take place
	CZipString szFileNameInZip = (LPCTSTR)header.GetFileName();
	CZipString szFile = PredictExtractedFileName(szFileNameInZip, lpszPath, bFullPath, lpszNewName);
	CZipActionCallback* pCallback = GetCallback(cbExtract);
	if (pCallback)
		pCallback->Init(szFileNameInZip, szFile);


	if (header.IsDirectory())
	{
		if (pCallback)
			pCallback->SetTotal(0); // in case of calling LeftToDo afterwards

		ZipPlatform::ForceDirectory(szFile);
		ZipPlatform::SetFileAttr(szFile, header.GetSystemAttr());

		if (pCallback)
			pCallback->CallbackEnd();
		return true;
	}
	else
	{
		if (pCallback)
			pCallback->SetTotal(header.m_uUncomprSize);

		if (!OpenFile(uIndex))
			return false;

		CZipPathComponent zpc(szFile);
		ZipPlatform::ForceDirectory(zpc.GetFilePath());
		CZipFile f(szFile, CZipFile::modeWrite |
			CZipFile::modeCreate | CZipFile::shareDenyWrite);
		DWORD iRead;
		CZipAutoBuffer buf(nBufSize);
		int iAborted = 0;
		do
		{
			iRead = ReadFile(buf, buf.GetSize());
			if (iRead)
			{
				f.Write(buf, iRead);
				if (pCallback)
					if (!(*pCallback)(iRead))
					{
						if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
							iAborted = CZipException::abortedAction;
						else
							iAborted = CZipException::abortedSafely; // we did it!
						break;
					}

			}
		}
		while (iRead == buf.GetSize());
		bool bRet = CloseFile(f) == 1;
		if (!bRet && iAborted == CZipException::abortedSafely)
			iAborted = CZipException::abortedAction; // sorry, finished, but not successfull

		if (pCallback)
			pCallback->CallbackEnd();

		if (iAborted)
			CZipException::Throw(iAborted, szFile); // throw to distuingiush from other return codes
		return bRet;

	}
}

bool CZipArchive::ExtractFile(WORD uIndex,
                              CZipMemFile& mf,
                              DWORD nBufSize)
{
	if (!nBufSize)
		return false;

	CZipFileHeader header;
	GetFileInfo(header, uIndex); // to ensure that slash and oem conversions take place
	CZipActionCallback* pCallback = GetCallback(cbExtract);
	if (pCallback)
	{
		pCallback->Init(header.GetFileName());
		pCallback->SetTotal(header.m_uUncomprSize);
	}

	if (header.IsDirectory() || !OpenFile(uIndex))
		return false;




	CZipAutoBuffer buf(nBufSize);
	mf.SeekToEnd();
	DWORD iRead;
	int iAborted = 0;
	do
	{
		iRead = ReadFile(buf, buf.GetSize());
		if (iRead)
		{
			mf.Write(buf, iRead);
			if (pCallback)
				if (!(*pCallback)(iRead))
				{
					if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
						iAborted = CZipException::abortedAction;
					else
						iAborted = CZipException::abortedSafely; // we did it!
					break;
				}
		}
	}
	while (iRead == buf.GetSize());
	bool bRet = CloseFile() == 1;
	if (!bRet && iAborted == CZipException::abortedSafely)
		iAborted = CZipException::abortedAction; // sorry, finished, but not successfull

	if (pCallback)
		pCallback->CallbackEnd();

	if (iAborted)
		CZipException::Throw(iAborted); // throw to distuingiush from other return codes
	return bRet;
}


void CZipArchive::SetExtraField(const char *pBuf, WORD iSize)
{
	if (m_iFileOpened != compress)
	{
		TRACE(_T("%s(%i) : A new file must be opened.\n"),__FILE__,__LINE__);
		return;
	}
	if (!pBuf || !iSize)
		return;

	CurrentFile()->m_pExtraField.Allocate(iSize);
	memcpy(CurrentFile()->m_pExtraField, pBuf, iSize);
}

bool CZipArchive::WriteNewFile(const void *pBuf, DWORD iSize)
{
	if (m_iFileOpened != compress)
	{
		TRACE(_T("%s(%i) : A new file must be opened.\n"),__FILE__,__LINE__);
		return false;
	}


    m_info.m_stream.next_in = (Bytef*)pBuf;
    m_info.m_stream.avail_in = iSize;
    CurrentFile()->m_uCrc32 = crc32(CurrentFile()->m_uCrc32, (Bytef*)pBuf, iSize);


    while (m_info.m_stream.avail_in > 0)
    {
        if (m_info.m_stream.avail_out == 0)
        {
			CryptEncodeBuffer();
			m_storage.Write(m_info.m_pBuffer, m_info.m_uComprLeft, false);
			m_info.m_uComprLeft = 0;
            m_info.m_stream.avail_out = m_info.m_pBuffer.GetSize();
            m_info.m_stream.next_out = (Bytef*)(char*)m_info.m_pBuffer;
        }

        if (CurrentFile()->m_uMethod == Z_DEFLATED)
        {
            DWORD uTotal = m_info.m_stream.total_out;
            int err = deflate(&m_info.m_stream,  Z_NO_FLUSH);
			CheckForError(err);
            m_info.m_uComprLeft += m_info.m_stream.total_out - uTotal;
        }
        else
        {
            DWORD uToCopy = (m_info.m_stream.avail_in < m_info.m_stream.avail_out)
				? m_info.m_stream.avail_in : m_info.m_stream.avail_out;

			memcpy(m_info.m_stream.next_out, m_info.m_stream.next_in, uToCopy);

            m_info.m_stream.avail_in -= uToCopy;
            m_info.m_stream.avail_out -= uToCopy;
            m_info.m_stream.next_in += uToCopy;
            m_info.m_stream.next_out += uToCopy;
            m_info.m_stream.total_in += uToCopy;
            m_info.m_stream.total_out += uToCopy;
            m_info.m_uComprLeft += uToCopy;
        }
    }

	return true;
}

bool CZipArchive::CloseNewFile(bool bAfterException)
{
	if (m_iFileOpened != compress)
	{
		TRACE(_T("%s(%i) : A new file must be opened.\n"),__FILE__,__LINE__);
		return false;
	}

    m_info.m_stream.avail_in = 0;
    if (!bAfterException)
	{
		int err = Z_OK;
		if (CurrentFile()->m_uMethod == Z_DEFLATED)
			while (err == Z_OK)
			{
				if (m_info.m_stream.avail_out == 0)
				{
					CryptEncodeBuffer();
					m_storage.Write(m_info.m_pBuffer, m_info.m_uComprLeft, false);
					m_info.m_uComprLeft = 0;
					m_info.m_stream.avail_out = m_info.m_pBuffer.GetSize();
					m_info.m_stream.next_out = (Bytef*)(char*)m_info.m_pBuffer;
				}
				DWORD uTotal = m_info.m_stream.total_out;
				err = deflate(&m_info.m_stream,  Z_FINISH);
				m_info.m_uComprLeft += m_info.m_stream.total_out - uTotal;
			}

		if (err == Z_STREAM_END)
			err = Z_OK;
		CheckForError(err);

		if (m_info.m_uComprLeft > 0)
		{
			CryptEncodeBuffer();
			m_storage.Write(m_info.m_pBuffer, m_info.m_uComprLeft, false);
		}

		if (CurrentFile()->m_uMethod == Z_DEFLATED)
		{
			err = deflateEnd(&m_info.m_stream);
			CheckForError(err);
		}


		// it may be increased by the encrypted header size
		CurrentFile()->m_uComprSize += m_info.m_stream.total_out;
		CurrentFile()->m_uUncomprSize = m_info.m_stream.total_in;

		m_centralDir.CloseNewFile();
	}
	else
		m_centralDir.m_pOpenedFile = NULL;

	m_iFileOpened = nothing;
	m_info.ReleaseBuf();
	EmptyPtrList();

	if (m_bAutoFlush && !bAfterException)
		Flush();

	return true;
}

void CZipArchive::DeleteFile(WORD uIndex)
{
	CZipWordArray indexes;
	indexes.Add(uIndex);
	DeleteFiles(indexes);
}

void CZipArchive::GetIndexes(const CZipStringArray &aNames, CZipWordArray& aIndexes)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return;
	}
	int iSize = aNames.GetSize();
	for (WORD i = 0; i < iSize; i++)
	{
		int idx = FindFile(aNames[i], ffDefault, false);
		if (idx != -1)
			aIndexes.Add((WORD)idx);
	}
}

void CZipArchive::DeleteFiles(const CZipStringArray &aNames)
{
	CZipWordArray indexes;
	GetIndexes(aNames, indexes);
	DeleteFiles(indexes);
}


void CZipArchive::DeleteFiles(CZipWordArray &aIndexes)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return;
	}

	if (m_storage.IsSpanMode())
	{
		TRACE(_T("%s(%i) : You cannot delete files from the disk spannig archive.\n"),__FILE__,__LINE__);
		return;
	}

	if (m_iFileOpened)
	{
		TRACE(_T("%s(%i) : You cannot delete files if there is a file opened.\n"),__FILE__,__LINE__);
		return;
	}

	CZipActionCallback* pCallback = GetCallback(cbDeleteCnt);
	if (pCallback)
		pCallback->Init();

	int uSize = aIndexes.GetSize();
	if (!uSize)
	{
		TRACE(_T("%s(%i) : The indekses array is empty.\n"),__FILE__,__LINE__);
		return;
	}

	// remove all - that's easy so don't waste the time
	if (uSize == GetCount())
	{
		pCallback = GetCallback(cbDelete);
		if (pCallback)
		{
			// do it right and sent the notification
			pCallback->Init();
			pCallback->SetTotal(uSize);
		}

		m_centralDir.RemoveFromDisk();
		m_storage.m_pFile->SetLength(m_centralDir.GetBytesBefore());
		m_centralDir.RemoveAll();
		if (m_bAutoFlush)
			Flush();
		if (pCallback)
			pCallback->CallbackEnd();
		return;
	}

	aIndexes.Sort(true);

	CZipArray<CZipDeleteInfo> aInfo;

	int iDelIndex = 0;


	int iStep = 0; // for the compiler
	if (pCallback)
	{
		pCallback->SetTotal(GetCount());
		iStep = CZipActionCallback::m_iStep; // we don't want to wait forever
	}

	int i;
	int uMaxDelIndex = aIndexes[uSize - 1];
	for (i = aIndexes[0]; i < GetCount(); i++)
	{
		CZipFileHeader* pHeader = m_centralDir[i];
		bool bDelete;
		if (i <= uMaxDelIndex && i == aIndexes[iDelIndex])
		{
			iDelIndex++;
			bDelete = true;
		}
		else
			bDelete = false;
		aInfo.Add(CZipDeleteInfo(pHeader, bDelete));
		if (pCallback && (!(i % iStep)))
			if (!(*pCallback)(iStep))
				ThrowError(CZipException::abortedSafely);
	}
	ASSERT(iDelIndex == uSize);

	uSize = aInfo.GetSize();
	if (!uSize) // it is possible
		return;

	// now we start deleting (not safe to break)
	pCallback = GetCallback(cbDelete);
	if (pCallback)
		pCallback->Init();


	m_centralDir.RemoveFromDisk();

	DWORD uTotalToMoveBytes = 0, uLastOffset = m_storage.m_pFile->GetLength() - m_centralDir.GetBytesBefore();
	// count the number of bytes to move
	for (i = uSize - 1; i >=0 ; i--)
	{
		const CZipDeleteInfo& di = aInfo[i];
		if (!di.m_bDelete)
			uTotalToMoveBytes += uLastOffset - di.m_pHeader->m_uOffset;
		uLastOffset = di.m_pHeader->m_uOffset;
	}
	if (pCallback)
		pCallback->CallbackEnd();


	if (pCallback)
		pCallback->SetTotal(uTotalToMoveBytes);


	m_info.Init();

	DWORD uMoveBy = 0, uOffsetStart = 0;
	for (i = 0; i < uSize; i++)
	{
		const CZipDeleteInfo& di = aInfo[i];

		if (di.m_bDelete)
		{
			// next hole
			DWORD uTemp = di.m_pHeader->m_uOffset;
			m_centralDir.RemoveFile(di.m_pHeader); // first remove
			if (uOffsetStart)
			{
				// copy the files over a previous holes
				MovePackedFiles(uOffsetStart, uTemp, uMoveBy, pCallback);
				uOffsetStart = 0;  // never be at the beginning, because the first file is always to be deleted
			}
			if (i == uSize - 1)
				uTemp = (m_storage.m_pFile->GetLength() - m_centralDir.GetBytesBefore()) - uTemp;
			else
				uTemp = aInfo[i+1].m_pHeader->m_uOffset - uTemp;

			uMoveBy += uTemp;

		}
		else
		{
			if (uOffsetStart == 0) // find contiuos area to move
				uOffsetStart = di.m_pHeader->m_uOffset;
			di.m_pHeader->m_uOffset -= uMoveBy;
		}

	}
	if (uOffsetStart)
		MovePackedFiles(uOffsetStart,
			m_storage.m_pFile->GetLength() - m_centralDir.GetBytesBefore(),
			uMoveBy, pCallback);

	m_info.ReleaseBuf();
	if (uMoveBy) // just in case
		m_storage.m_pFile->SetLength(m_storage.m_pFile->GetLength() - uMoveBy);

	if (pCallback)
		pCallback->CallbackEnd();

	if (m_bAutoFlush)
		Flush();
}



bool CZipArchive::AddNewFile(LPCTSTR lpszFilePath,
                             int iComprLevel,
                             bool bFullPath,
							 int iSmartLevel,
                             unsigned long nBufSize)
{

	CZipAddNewFileInfo zanfi (lpszFilePath, bFullPath);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = zipsmSafeSmart;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);
}

bool CZipArchive::AddNewFile(LPCTSTR lpszFilePath,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel,
							 int iSmartLevel,
                             unsigned long nBufSize)
{
	CZipAddNewFileInfo zanfi(lpszFilePath, lpszFileNameInZip);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = zipsmSafeSmart;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);
}

bool CZipArchive::AddNewFile(CZipMemFile& mf,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel,
							 int iSmartLevel,
                             unsigned long nBufSize)
{
	CZipAddNewFileInfo zanfi(&mf, lpszFileNameInZip);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = zipsmSafeSmart;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);
}


bool CZipArchive::AddNewFile(CZipAddNewFileInfo& info)
{
	// no need for ASSERT and TRACE here - it will be done by OpenNewFile

	if (!m_info.m_iBufferSize)
		return false;
	CZipPathComponent::RemoveSeparators(info.m_szFilePath);
	if (!info.m_szFilePath.IsEmpty()) // it may be empty after removing sep.
	{
		if (info.m_szFileNameInZip.IsEmpty())
		{
			CZipPathComponent zpc(info.m_szFilePath);
			if (info.m_bFullPath)
			{
				if (m_bRemoveDriveLetter)
					info.m_szFileNameInZip = zpc.GetNoDrive();
			}
			else
				info.m_szFileNameInZip = TrimRootPath(zpc);
		}
	}
	else if (!info.m_pFile)
		return false;

	bool bSpan = GetSpanMode() != 0;

	// checking the iReplace index
	if (!UpdateReplaceIndex(info.m_iReplaceIndex, info.m_szFileNameInZip))
		return false;

	bool bReplace = info.m_iReplaceIndex >= 0;

	DWORD uAttr;
	time_t ttime;
	if (info.m_pFile)
	{
		uAttr = ZipPlatform::GetDefaultAttributes();
		ttime = time(NULL);
	}
	else
	{
		if (!ZipPlatform::GetFileAttr(info.m_szFilePath, uAttr))
			return false; // we don't know whether it is a file or a directory
		if (!ZipPlatform::GetFileModTime(info.m_szFilePath, ttime))
			ttime = time(NULL);
	}
	CZipFileHeader header;
	header.SetFileName(info.m_szFileNameInZip);
	if (ZipPlatform::GetSystemID() != ZipCompatibility::zcUnix)
		uAttr |= ZipCompatibility::ConvertToSystem(uAttr, ZipPlatform::GetSystemID(), ZipCompatibility::zcUnix);  // make it readable under Unix as well, since it stores its attributes in HIWORD(uAttr)
	SetFileHeaderAttr(header, uAttr);
	header.SetTime(ttime);
	bool bInternal = (info.m_iSmartLevel & zipsmInternal01) != 0;
	CZipActionCallback* pCallback = NULL;
	if (!bInternal)
	{
		pCallback = GetCallback(cbAdd);
		if (pCallback)
			pCallback->Init(info.m_szFileNameInZip, info.m_szFilePath);
	}



	if (header.IsDirectory()) // will never be when m_pFile is not NULL, so we don't check it
	{
		ASSERT(!info.m_pFile); // should never happened
		ASSERT(!bInternal);

		if (pCallback)
			pCallback->SetTotal(0); // in case of calling LeftToDo afterwards

		// clear password for a directory
		bool bRet = false;
		CZipSmClrPass smcp;
		if (info.m_iSmartLevel & zipsmCPassDir)
			smcp.ClearPasswordSmartly(this);

		bRet = OpenNewFile(header, bReplace ? (info.m_iReplaceIndex << 16) | ZIP_COMPR_REPL_SIGN : 0);

		CloseNewFile();
		if (pCallback)
			pCallback->CallbackEnd();

		return bRet;
	}

	CZipSmClrPass smcp;
	bool bIsCompression = info.m_iComprLevel != 0;
	bool bEff = (info.m_iSmartLevel & zipsmCheckForEff)&& bIsCompression;
	bool bCheckForZeroSized = (info.m_iSmartLevel & zipsmCPFile0) && !GetPassword().IsEmpty();
	bool bCheckForSmallFiles = (info.m_iSmartLevel & zipsmNotCompSmall) && bIsCompression;
	DWORD iFileSize = DWORD(-1);
	bool bNeedTempArchive = (bEff && bSpan) || (bReplace && bIsCompression);
	if (bCheckForSmallFiles || bCheckForZeroSized || bNeedTempArchive)
	{

		if (info.m_pFile)
			iFileSize = info.m_pFile->GetLength();
		else
		{
			if (!ZipPlatform::GetFileSize(info.m_szFilePath, iFileSize) && bEff)
				bEff = false; // the file size is needed only when eff. in span mode
		}
		if (iFileSize !=  DWORD(-1))
		{
			if (bCheckForZeroSized && iFileSize == 0)
				smcp.ClearPasswordSmartly(this);
			if (bCheckForSmallFiles && iFileSize < 5)
				info.m_iComprLevel = 0;
		}
	}
	bool bEffInMem = bEff && (info.m_iSmartLevel & zipsmMemoryFlag);
	CZipString szTempFileName;
	if (bNeedTempArchive && (bEffInMem ||
		!(szTempFileName = ZipPlatform::GetTmpFileName(
			m_szTempPath.IsEmpty() ? NULL : (LPCTSTR)m_szTempPath, iFileSize)
		).IsEmpty()))
	{
		CZipMemFile* pmf = NULL;
		CZipArchive zip;
		try
		{
			// compress first to a temporary file, if ok - copy the data, if not - add storing

			if (bEffInMem)
			{
				pmf = new CZipMemFile;
				zip.Open(*pmf, zipCreate);
			}
			else
				zip.Open(szTempFileName, zipCreate);
			zip.SetRootPath(m_szRootPath);
			zip.SetPassword(GetPassword());
			zip.SetSystemCompatibility(m_iArchiveSystCompatib);
			zip.SetCallback(pCallback, cbAdd);
			// create a temporary file
			int iTempReplaceIndex = info.m_iReplaceIndex;
			info.m_iSmartLevel = zipsmLazy;
			info.m_iReplaceIndex = -1;
			if (!zip.AddNewFile(info))
				throw false;
			info.m_iReplaceIndex = iTempReplaceIndex;

			// this may also happen when bReplace, but not in span mode
			if (bEff)
			{
				CZipFileHeader fh;
				zip.GetFileInfo(fh, 0);
				if (!fh.CompressionEfficient())
				{
					info.m_iComprLevel = 0;
					info.m_iSmartLevel = zipsmInternal01;
					// compression is pointless, store instead
					throw AddNewFile(info);
				}
			}

			m_info.Init();
			throw GetFromArchive(zip, 0, info.m_iReplaceIndex, true, GetCallback(cbAddTmp));
		}
		catch (bool bRet)
		{

			zip.Close(!bRet); // that doesn't really matter how it will be closed
			if (pmf)
				delete pmf;
			if (!bEffInMem)
				ZipPlatform::RemoveFile(szTempFileName, false);
			m_info.ReleaseBuf();
			return bRet;
		}
		catch (...)
		{
			zip.Close(true);
			if (pmf)
				delete pmf;
			if (!bEffInMem)
				ZipPlatform::RemoveFile(szTempFileName, false);
			m_info.ReleaseBuf();
			throw;
		}
	}

	// try to open before adding
	CZipFile f;
	CZipAbstractFile *pf;
	if (info.m_pFile)
		pf = info.m_pFile;
	else
	{
		// cannot be shareDenyWrite
		// from http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp :
		// If you specify the GENERIC_READ and GENERIC_WRITE access modes along with the FILE_SHARE_READ and FILE_SHARE_WRITE sharing modes in your first call to CreateFile. If you specify the GENERIC_READ and GENERIC_WRITE access modes and the FILE_SHARE_READ sharing mode only in your second call to CreateFile, the function will fail with a sharing violation because the read-only sharing mode specified in the second call conflicts with the read/write access that has been granted in the first call.
		if (!f.Open(info.m_szFilePath, CZipFile::modeRead | CZipFile::shareDenyNone, false))
		{
			if (pCallback)
				pCallback->CallbackEnd();
			return false;
		}
		pf = &f;
	}

	ASSERT(pf);
	// call init before opening (in case of exception we have the names)
	iFileSize = pf->GetLength();


	bool bRet;
	if (bReplace)
	{
		ASSERT(!bIsCompression);
		bRet = OpenNewFile(header, (info.m_iReplaceIndex << 16) | ZIP_COMPR_REPL_SIGN , NULL, iFileSize);
	}
	else
		bRet = OpenNewFile(header, info.m_iComprLevel);
	if (!bRet)
	{
		if (pCallback)
			pCallback->CallbackEnd();

		return false;
	}
	if (bInternal)
	{
		// we do it here, because if in OpenNewFile is replacing
		// then we get called cbReplace callback before and it would
		// overwrite callback information written in pCallback->Init
		pCallback = GetCallback(cbAddStore);
		if (pCallback)
			pCallback->Init(info.m_szFileNameInZip, info.m_szFilePath);
	}
	if (pCallback)
		pCallback->SetTotal(iFileSize);

	CZipAutoBuffer buf(info.m_nBufSize);
	DWORD iRead;
	int iAborted = 0;
	do
	{
		iRead = pf->Read(buf, info.m_nBufSize);
		if (iRead)
		{
			WriteNewFile(buf, iRead);
			if (pCallback)
				if (!(*pCallback)(iRead))
				{
					// todo: we could remove here the bytes of the file partially added if not disk-spanning
					if (iRead == buf.GetSize() && pf->Read(buf, 1) != 0) // test one byte if there is something left
					{
						if (!m_storage.IsSpanMode() && !bReplace)
						{
							RemoveLast(true);
							CloseNewFile(true);
							iAborted = CZipException::abortedSafely;
						}
						else
							iAborted = CZipException::abortedAction;
					}
					else
					{
						iAborted = CZipException::abortedSafely; // we did it!
						CloseNewFile();
					}
					break;
				}

		}

	}
	while (iRead == buf.GetSize());
	if (!iAborted)
		CloseNewFile();

	if (pCallback)
		pCallback->CallbackEnd();

	if (iAborted)
		CZipException::Throw(iAborted); // throw to distuinguish from other return codes

	if (bEff)
	{
		// remove the last file and add it without the compression if needed
		if (!info.m_pFile)
			f.Close();

		buf.Release();
		if (RemoveLast())
		{
			info.m_iComprLevel = 0;
			info.m_iSmartLevel = zipsmInternal01;
			return AddNewFile(info);
		}
	}
	return true;

}

bool CZipArchive::RemoveLast(bool bRemoveAnyway)
{
	int iIndex = GetCount() - 1;
	if (iIndex < 0)
		return false;
	CZipFileHeader* pHeader = m_centralDir[iIndex];

	if (!bRemoveAnyway && pHeader->CompressionEfficient())
		return false;

	m_centralDir.RemoveLastFile(pHeader, iIndex);
	return true;
}




CZipString CZipArchive::GetArchivePath() const
{
	if (IsClosed(false))
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return _T("");
	}
	return m_storage.m_pFile->GetFilePath();
}

CZipString CZipArchive::GetGlobalComment() const
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return _T("");
	}
	CZipString temp;
	return SingleToWide(m_centralDir.m_pszComment, temp) != -1 ? (LPCTSTR)temp : _T("");
}

bool CZipArchive::SetGlobalComment(LPCTSTR lpszComment)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}
	if (m_storage.IsSpanMode() == -1)
	{
		TRACE(_T("%s(%i) : You cannot modify the global comment of the existing disk spanning archive.\n"),__FILE__,__LINE__);
		return false;
	}

	WideToSingle(lpszComment, m_centralDir.m_pszComment);
	m_centralDir.RemoveFromDisk();
	if (m_bAutoFlush)
		Flush();

	return true;
}



int CZipArchive::GetCurrentDisk() const
{
	return m_storage.GetCurrentDisk() + 1;
}

bool CZipArchive::SetFileComment(WORD uIndex, LPCTSTR lpszComment)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}
	if (m_storage.IsSpanMode() == -1)
	{
		TRACE(_T("%s(%i) : You cannot modify the file comment in the existing disk spanning archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (!m_centralDir.IsValidIndex(uIndex))
	{
		ASSERT(FALSE);
		return false;
	}
	m_centralDir[uIndex]->SetComment(lpszComment);
	m_centralDir.RemoveFromDisk();
	if (m_bAutoFlush)
		Flush();
	return true;
}


void CZipArchive::CryptInitKeys()
{
	ASSERT(m_pszPassword.GetSize());
	m_keys[0] = 305419896L;
	m_keys[1] = 591751049L;
	m_keys[2] = 878082192L;
	for (DWORD i = 0; i < m_pszPassword.GetSize(); i++)
		CryptUpdateKeys(m_pszPassword[i]);
}

void CZipArchive::CryptUpdateKeys(char c)
{

	m_keys[0] = CryptCRC32(m_keys[0], c);
	m_keys[1] += m_keys[0] & 0xff;
	m_keys[1] = m_keys[1] * 134775813L + 1;
	c = char(m_keys[1] >> 24);
	m_keys[2] = CryptCRC32(m_keys[2], c);
}

bool CZipArchive::CryptCheck()
{
	CZipAutoBuffer buf(ZIPARCHIVE_ENCR_HEADER_LEN);
	m_storage.Read(buf, ZIPARCHIVE_ENCR_HEADER_LEN, false);
	BYTE b = 0;
	for (int i = 0; i < ZIPARCHIVE_ENCR_HEADER_LEN; i++)
	{
		b = buf[i]; // only temporary
		CryptDecode((char&)b);
	}
	// check the last byte
	return CurrentFile()->IsDataDescr() ?
		(BYTE(CurrentFile()->m_uModTime >> 8) == b) : (BYTE(CurrentFile()->m_uCrc32 >> 24) == b);
}

char CZipArchive::CryptDecryptByte()
{
	int temp = (m_keys[2] & 0xffff) | 2;
	return (char)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

void CZipArchive::CryptDecode(char &c)
{
	c ^= CryptDecryptByte();
	CryptUpdateKeys(c);
}

bool CZipArchive::SetPassword(LPCTSTR lpszPassword)
{
	if (m_iFileOpened != nothing)
	{
		TRACE(_T("%s(%i) : You cannot change the password when the file is opened.\n"),__FILE__,__LINE__);
		return false; // it's important not to change the password when the file inside archive is opened
	}
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : Setting the password for a closed archive has no effect.\n"),__FILE__,__LINE__);
	}
	if (lpszPassword)
	{
		int iLen = WideToSingle(lpszPassword, m_pszPassword);
		if (iLen == -1)
			return false;
		for (std::size_t i = 0; (int)i < iLen; i++)
			if (m_pszPassword[i] <= 0)
			{
				m_pszPassword.Release();
				TRACE(_T("%s(%i) : The password contains forbidden characters. Password cleared.\n"),__FILE__,__LINE__);
				return false;
			}
	}
	else
		m_pszPassword.Release();
	return true;
}

CZipString CZipArchive::GetPassword()const
{
	CZipString temp;
	CZipArchive::SingleToWide(m_pszPassword, temp);
	return temp;
}

DWORD CZipArchive::CryptCRC32(DWORD l, char c)
{
	const DWORD *CRC_TABLE = get_crc_table();
	return CRC_TABLE[(l ^ c) & 0xff] ^ (l >> 8);
}

void CZipArchive::CryptCryptHeader(long iCrc, CZipAutoBuffer &buf)
{
	CryptInitKeys();
	srand(UINT(time(NULL)));
	// genereate pseudo-random sequence
	char c;
	for (int i = 0; i < ZIPARCHIVE_ENCR_HEADER_LEN - 2; i++)
	{
		int t1 = rand();
		c = (char)(t1 >> 6);
		if (!c)
			c = (char)t1;
		CryptEncode(c);
		buf[i] = c;

	}
	c = (char)((iCrc >> 16) & 0xff);
	CryptEncode(c);
	buf[ZIPARCHIVE_ENCR_HEADER_LEN - 2] = c;
	c = (char)((iCrc >> 24) & 0xff);
	CryptEncode(c);
	buf[ZIPARCHIVE_ENCR_HEADER_LEN - 1] = c;
}

void CZipArchive::CryptEncode(char &c)
{
	char t = CryptDecryptByte();
	CryptUpdateKeys(c);
	c ^= t;
}

void CZipArchive::CryptEncodeBuffer()
{
	if (CurrentFile()->IsEncrypted())
		for (DWORD i = 0; i < m_info.m_uComprLeft; i++)
			CryptEncode(m_info.m_pBuffer[i]);
}

void CZipArchive::CloseFileAfterTestFailed()
{
	if (m_iFileOpened != extract)
	{
		TRACE(_T("%s(%i) : No file opened.\n"),__FILE__,__LINE__);
		return;
	}
	m_info.ReleaseBuf();
	m_centralDir.Clear(false);
	m_iFileOpened = nothing;
}

bool CZipArchive::TestFile(WORD uIndex, DWORD uBufSize)
{
	if (m_storage.IsSpanMode() == 1)
	{
		TRACE(_T("%s(%i) : You cannot test the spanning archive in creation.\n"),__FILE__,__LINE__);
		return false;
	}
	if (!uBufSize)
		return false;

	CZipFileHeader* pHeader = m_centralDir[uIndex];
	CZipActionCallback* pCallback = GetCallback(cbTest);
	if (pCallback)
	{
		pCallback->Init(m_centralDir.GetProperHeaderFileName(pHeader));
	}

	if (pHeader->IsDirectory())
	{
		if (pCallback)
			pCallback->SetTotal(0);

		// we do not test whether the password for the encrypted directory
		// is correct, since it seems to be senseless (anyway password
		// encrypted directories should be avoided - it adds 12 bytes)
		DWORD iSize = pHeader->m_uComprSize;
		if ((iSize != 0 || iSize != pHeader->m_uUncomprSize)
			// different treating compressed directories
			&& !(pHeader->IsEncrypted() && iSize == 12 && !pHeader->m_uUncomprSize))
			CZipException::Throw(CZipException::dirWithSize);

		if (pCallback)
			pCallback->CallbackEnd();

		return true;
	}
	else
	{
		try
		{
			if (pCallback)
				pCallback->SetTotal(pHeader->m_uUncomprSize);

			if (!OpenFile(uIndex))
				return false;
			CZipAutoBuffer buf(uBufSize);
			DWORD iRead;
			int iAborted = 0;
			do
			{
				iRead = ReadFile(buf, buf.GetSize());
				if (pCallback && iRead)
					if (!(*pCallback)(iRead))
					{
						if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
							iAborted = CZipException::abortedAction;
						else
							iAborted = CZipException::abortedSafely; // we did it!
						break;
					}
			}
			while (iRead == buf.GetSize());
			bool bRet = CloseFile() != -1;
			if (!bRet && iAborted == CZipException::abortedSafely)
				iAborted = CZipException::abortedAction; // sorry, finished, but not successfull

			if (pCallback)
				pCallback->CallbackEnd();

			if (iAborted)
				CZipException::Throw(iAborted); // throw to distuingiush from other return codes
			if (bRet)
				return true;
			else
				CZipException::Throw(CZipException::badZipFile);
			return false; // to satisfy the compiler and eliminate warning
		}
		catch(...)
		{
			CloseFileAfterTestFailed();
			throw;
		}
	}
}

int CZipArchive::WideToSingle(LPCTSTR lpWide, CZipAutoBuffer &szSingle)
{
#ifdef _UNICODE
	return ZipPlatform::WideToSingle(lpWide, szSingle);
#else

	std::size_t iLen = strlen(lpWide);
	// if not UNICODE just copy
	// 	iLen does not include the NULL character
	szSingle.Allocate(iLen);
	memcpy(szSingle, lpWide, iLen);
	return iLen;
#endif

}

int CZipArchive::SingleToWide(const CZipAutoBuffer &szSingle, CZipString& szWide)
{

#ifdef _UNICODE
	return ZipPlatform::SingleToWide(szSingle, szWide);
#else // if not UNICODE just copy
	int singleLen = szSingle.GetSize();
	// 	iLen does not include the NULL character
	memcpy(szWide.GetBuffer(singleLen),szSingle.GetBuffer(), singleLen);
	szWide.ReleaseBuffer(singleLen);
	return singleLen;
#endif
}

void CZipArchive::CryptDecodeBuffer(DWORD uCount)
{
	if (CurrentFile()->IsEncrypted())
		for (DWORD i = 0; i < uCount; i++)
			CryptDecode(m_info.m_pBuffer[i]);
}

void CZipArchive::EmptyPtrList()
{
	if (m_list.GetCount())
	{
		// if some memory hasn't been freed due to an error in zlib, so free it now
		CZipPtrListIter iter = m_list.GetHeadPosition();
		while (m_list.IteratorValid(iter))
			delete[] (char*) m_list.GetNext(iter);
	}
	m_list.RemoveAll();
}



void CZipArchive::SetFileHeaderAttr(CZipFileHeader& header, DWORD uAttr)
{
	header.SetSystemCompatibility(m_iArchiveSystCompatib);
	header.SetSystemAttr(uAttr);
}

void CZipArchive::EnableFindFast(bool bEnable)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : Set it after opening the archive.\n"),__FILE__,__LINE__);
		return;
	}
	m_centralDir.EnableFindFast(bEnable, m_bCaseSensitive);

}

bool CZipArchive::SetSystemCompatibility(int iSystemComp)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : Set it after opening the archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_iFileOpened == compress)
	{
		TRACE(_T("%s(%i) : Set it before opening a file inside archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (!ZipCompatibility::IsPlatformSupported(iSystemComp))
		return false;
	m_iArchiveSystCompatib = iSystemComp;
	return true;
}

void CZipArchive::SetRootPath(LPCTSTR szPath)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : Set it after opening the archive.\n"),__FILE__,__LINE__);
		return;
	}

	if (m_iFileOpened != nothing)
	{
		TRACE(_T("%s(%i) : Set it before opening a file inside archive.\n"),__FILE__,__LINE__);
		return;
	}

	if (szPath)
	{
		m_szRootPath = szPath;
		CZipPathComponent::RemoveSeparators(m_szRootPath);
	}
	else
		m_szRootPath.Empty();
}

CZipString CZipArchive::TrimRootPath(CZipPathComponent &zpc)const
{
	if (m_szRootPath.IsEmpty())
		return zpc.GetFileName();
	CZipString szPath = zpc.GetFullPath();
	return RemovePathBeginning(m_szRootPath, szPath, m_pZipCompare) ? szPath : zpc.GetFileName();
}

bool CZipArchive::RemovePathBeginning(LPCTSTR lpszBeginning, CZipString& szPath, ZIPSTRINGCOMPARE pCompareFunction)
{
	CZipString szBeginning(lpszBeginning);
	CZipPathComponent::RemoveSeparators(szBeginning);
	int iRootPathLength = szBeginning.GetLength();
	if (iRootPathLength && szPath.GetLength() >= iRootPathLength &&
		(szPath.Left(iRootPathLength).*pCompareFunction)(szBeginning) == 0)
	{
		// the beginning is the same
		if (szPath.GetLength() == iRootPathLength)
		{
			szPath.Empty();
			return true;
		}
		// is the end of m_szPathRoot only a beginning of a directory name?
		// check for a separator
		// we know the length is larger, so we can write:
		if (CZipPathComponent::IsSeparator(szPath[iRootPathLength]))
		{
			szPath = szPath.Mid(iRootPathLength);
			CZipPathComponent::RemoveSeparatorsLeft(szPath);
			return true;
		}
	}
	return false;
}

void CZipArchive::SetTempPath(LPCTSTR lpszPath, bool bForce)
{
	m_szTempPath = lpszPath;
	if (lpszPath && bForce)
		ZipPlatform::ForceDirectory(lpszPath);
	CZipPathComponent::RemoveSeparators(m_szTempPath);
}

CZipString CZipArchive::PredictFileNameInZip(LPCTSTR lpszFilePath,
						 bool bFullPath, int iWhat, bool bExactly)const
{
	CZipString sz = lpszFilePath;
	if (sz.IsEmpty())
		return _T("");
	bool bAppend;
	switch (iWhat)
	{
	case prFile:
		bAppend = false;
		break;
	case prDir:
		bAppend = true;
		break;
	default:
		bAppend = CZipPathComponent::IsSeparator(sz[sz.GetLength() - 1]);
	}

	// remove for CZipPathComponent treating last name as a file even if dir
	CZipPathComponent::RemoveSeparators(sz);
	CZipPathComponent zpc(sz);

	if (bFullPath)
	{
		if (m_bRemoveDriveLetter)
			sz = zpc.GetNoDrive();
	}
	else
		sz = TrimRootPath(zpc);

	if (bAppend && !sz.IsEmpty())
		CZipPathComponent::AppendSeparator(sz);
	CZipFileHeader fh; // create a temporary object to convert
	fh.SetFileName(sz);
	if (bExactly)
	{
		fh.SetSystemCompatibility(m_iArchiveSystCompatib);
		ZipCompatibility::FileNameUpdate(fh, false);
	}
	else
	{
		fh.SetSystemCompatibility(-1); // non existing system to prevent ansi oem conversion
		ZipCompatibility::FileNameUpdate(fh, true);// update only path separators
	}

	return fh.GetFileName();
}

CZipString CZipArchive::PredictExtractedFileName(LPCTSTR lpszFileNameInZip, LPCTSTR lpszPath, bool bFullPath, LPCTSTR lpszNewName)const
{
	CZipString szFile = lpszPath;
	CZipString sz = lpszNewName ? lpszNewName : lpszFileNameInZip;
	if (sz.IsEmpty())
		return szFile;
	if (!szFile.IsEmpty())
		CZipPathComponent::AppendSeparator(szFile);


	// remove for CZipPathComponent treating last name as a file even if dir
	CZipPathComponent::RemoveSeparators(sz);
	CZipPathComponent zpc(sz);
	szFile += bFullPath ? (m_bRemoveDriveLetter ? zpc.GetNoDrive() : sz)
						  : TrimRootPath(zpc);
	return szFile;
}


void CZipArchive::SetAutoFlush(bool bAutoFlush)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive not yet opened.\n"),__FILE__,__LINE__);
		return;
	}
	if (m_storage.IsSpanMode() != 0)
	{
		TRACE(_T("%s(%i) : Cannot set auto-flush for the disk spanning archive.\n"),__FILE__,__LINE__);
		return;
	}
	m_bAutoFlush = bAutoFlush;
}

void CZipArchive::Flush()
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive not yet opened.\n"),__FILE__,__LINE__);
		return;
	}

	if (m_storage.IsSpanMode() < 0)
	{
		TRACE(_T("%s(%i) : Cannot flush an existing disk spanning archive.\n"),__FILE__,__LINE__);
		return;
	}
	WriteCentralDirectory();
	m_storage.FlushFile();
	if (m_storage.IsSpanMode() > 0) // try to finalize disk-spanning archive without closing it
		m_storage.FinalizeSpan();
}


void CZipArchive::GetCentralDirInfo(CZipCentralDir::Info& info)const
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive not yet opened.\n"),__FILE__,__LINE__);
		return;

	}
	m_centralDir.GetInfo(info);
	if (GetSpanMode() > 0)
		info.m_uDiskEntriesNo = m_storage.GetCurrentDisk();
}

bool CZipArchive::CWildcard::IsPatternValid(LPCTSTR lpszPattern, int* iErrorType)
{
	try
	{
		/* loop through pattern to EOS */
		while (*lpszPattern)
		{
			/* determine pattern type */
			switch (*lpszPattern)
			{
				/* check literal escape, it cannot be at end of pattern */
			case _T('\\'):
				if (!*++lpszPattern)
					throw patternEsc;
				lpszPattern++;
				break;

				/* the [..] construct must be well formed */
			case _T('['):
				lpszPattern++;

				/* if the next character is ']' then bad pattern */
				if (*lpszPattern == _T(']'))
					throw patternEmpty;

				/* if end of pattern here then bad pattern */
				if (!*lpszPattern)
					throw patternClose;

				/* loop to end of [..] construct */
				while (*lpszPattern != _T(']'))
				{
					/* check for literal escape */
					if (*lpszPattern == _T('\\'))
					{
						lpszPattern++;

						/* if end of pattern here then bad pattern */
						if (!*lpszPattern++)
							throw patternEsc;
					}
					else  lpszPattern++;

					/* if end of pattern here then bad pattern */
					if (!*lpszPattern)
						throw patternClose;

					/* if this a range */
					if (*lpszPattern == _T('-'))
					{
						/* we must have an end of range */
						if (!*++lpszPattern || *lpszPattern == ']')
							throw patternRange;
						else
						{

							/* check for literal escape */
							if (*lpszPattern == _T('\\'))
								lpszPattern++;

								/* if end of pattern here
							then bad pattern           */
							if (!*lpszPattern++)
								throw patternEsc;
						}
					}
				}
				break;

				/* all other characters are valid pattern elements */
			case '*':
			case '?':
			default:
				lpszPattern++;                              /* "normal" character */
				break;
			}
		}
		throw patternValid;
	}
	catch (int i)
	{
		if (iErrorType)
			*iErrorType = i;
		return i == patternValid;
	}


}

bool CZipArchive::CWildcard::IsPattern(LPCTSTR lpszPattern)
{
	while (*lpszPattern)
	{
        switch (*lpszPattern++)
        {
        case _T('?'):
        case _T('*'):
        case _T('['):
        case _T('\\'):
			return true;
        }
	}
	return false;

}

bool CZipArchive::CWildcard::IsMatch(LPCTSTR lpszText, int *iRetCode)
{
	CZipString sz;
	if (!m_bCaseSensitive)
	{
		sz = lpszText;
		sz.MakeLower();
		lpszText = (LPCTSTR)sz;
	}
	int i = Match((LPCTSTR)m_szPattern, lpszText);
	if (iRetCode)
		*iRetCode = i;
	return i == matchValid;
}

int CZipArchive::CWildcard::MatchAfterStar(LPCTSTR p, LPCTSTR t)
{
      int iMatch = matchNone;
      TCHAR nextp;

      /* pass over existing ? and * in pattern */

      while ( *p == _T('?') || *p == _T('*') )
      {
            /* take one char for each ? and + */

            if (*p == _T('?'))
            {
                  /* if end of text then no match */
                  if (!*t++)
                        return matchAbort;
            }

            /* move to next char in pattern */

            p++;
      }

      /* if end of pattern we have matched regardless of text left */

      if (!*p)
            return matchValid;

      /* get the next character to match which must be a literal or '[' */

      nextp = *p;
      if (nextp == _T('\\'))
      {
            nextp = p[1];

            /* if end of text then we have a bad pattern */

            if (!nextp)
                  return matchPattern;
      }

      /* Continue until we run out of text or definite result seen */

      do
      {
            /* a precondition for matching is that the next character
               in the pattern match the next character in the text or that
               the next pattern char is the beginning of a range.  Increment
               text pointer as we go here */

            if (nextp == *t || nextp == _T('['))
                  iMatch = Match(p, t);

            /* if the end of text is reached then no iMatch */

            if (!*t++)
                  iMatch = matchAbort;

      } while ( iMatch != matchValid &&
                iMatch != matchAbort &&
                iMatch != matchPattern);

      /* return result */

      return iMatch;
}


int CZipArchive::CWildcard::Match(LPCTSTR lpszPattern, LPCTSTR lpszText)
{

	TCHAR range_start, range_end;  /* start and end in range */

	bool bInvert;             /* is this [..] or [!..] */
	bool bMemberMatch;       /* have I matched the [..] construct? */
	bool bLoop;               /* should I terminate? */

	for ( ; *lpszPattern; lpszPattern++, lpszText++)
	{
	/* if this is the end of the text
		then this is the end of the match */

		if (!*lpszText)
		{
			if ( *lpszPattern == _T('*') && *++lpszPattern == _T('\0') )
				return matchValid;
			else
				return matchAbort;
		}

		/* determine and react to pattern type */

		switch (*lpszPattern)
		{
		case _T('?'):                     /* single any character match */
			break;

		case _T('*'):                     /* multiple any character match */
			return MatchAfterStar (lpszPattern, lpszText);

			/* [..] construct, single member/exclusion character match */
		case _T('['):
			{
				/* move to beginning of range */

				lpszPattern++;

				/* check if this is a member match or exclusion match */

				bInvert = false;
				if (*lpszPattern == _T('!') || *lpszPattern == _T('^'))
				{
					bInvert = true;
					lpszPattern++;
				}

				/* if closing bracket here or at range start then we have a
				malformed pattern */

				if (*lpszPattern == _T(']'))
					return matchPattern;

				bMemberMatch = false;
				bLoop = true;

				while (bLoop)
				{
					/* if end of construct then bLoop is done */

					if (*lpszPattern == _T(']'))
					{
						bLoop = false;
						continue;
					}

					/* matching a '!', '^', '-', '\' or a ']' */

					if (*lpszPattern == _T('\\'))
						range_start = range_end = *++lpszPattern;
					else
						range_start = range_end = *lpszPattern;

					/* if end of pattern then bad pattern (Missing ']') */

					if (!*lpszPattern)
						return matchPattern;

					/* check for range bar */
					if (*++lpszPattern == _T('-'))
					{
						/* get the range end */

						range_end = *++lpszPattern;

						/* if end of pattern or construct
						then bad pattern */

						if (range_end == _T('\0') || range_end == _T(']'))
							return matchPattern;
						/* special character range end */
						if (range_end == _T('\\'))
						{
							range_end = *++lpszPattern;

							/* if end of text then
							we have a bad pattern */
							if (!range_end)
								return matchPattern;
						}

						/* move just beyond this range */
						lpszPattern++;
					}

					/* if the text character is in range then match found.
					make sure the range letters have the proper
					relationship to one another before comparison */

					if (range_start < range_end)
					{
						if (*lpszText >= range_start && *lpszText <= range_end)
						{
							bMemberMatch = true;
							bLoop = false;
						}
					}
					else
					{
						if (*lpszText >= range_end && *lpszText <= range_start)
						{
							bMemberMatch = true;
							bLoop = false;
						}
					}
				}

				/* if there was a match in an exclusion set then no match */
				/* if there was no match in a member set then no match */

				if ((bInvert && bMemberMatch) || !(bInvert || bMemberMatch))
					return matchRange;

				/* if this is not an exclusion then skip the rest of
				the [...] construct that already matched. */

				if (bMemberMatch)
				{
					while (*lpszPattern != _T(']'))
					{
						/* bad pattern (Missing ']') */
						if (!*lpszPattern)
							return matchPattern;

						/* skip exact match */
						if (*lpszPattern == _T('\\'))
						{
							lpszPattern++;

							/* if end of text then
							we have a bad pattern */

							if (!*lpszPattern)
								return matchPattern;
						}

						/* move to next pattern char */

						lpszPattern++;
					}
				}
				break;
			}
			case _T('\\'):  /* next character is quoted and must match exactly */

				/* move pattern pointer to quoted char and fall through */

				lpszPattern++;

				/* if end of text then we have a bad pattern */

				if (!*lpszPattern)
					return matchPattern;

				/* must match this character exactly */

				[[fallthrough]];
			default:
				if (*lpszPattern != *lpszText)
					return matchPattern;
			}
	  }
	  /* if end of text not reached then the pattern fails */

		if (*lpszText)
			return matchEnd;
		else
			return matchValid;
}

void CZipArchive::FindMatches(LPCTSTR lpszPattern, CZipWordArray &ar, bool bFullPath) const
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return;
	}

	//	ar.RemoveAll(); don't do this
	int iCount = GetCount();
	CWildcard wc(lpszPattern, m_bCaseSensitive);
	for (int i = 0; i < iCount; i++)
	{
		const CZipFileHeader* pHeader = m_centralDir[i];
		CZipString sz =  m_centralDir.GetProperHeaderFileName(pHeader);
		if (!bFullPath)
		{
			CZipPathComponent::RemoveSeparators(sz);
			CZipPathComponent zpc(sz);
			sz = zpc.GetFileName();
		}
		if (wc.IsMatch(sz))
			ar.Add(i);
	}
}

int CZipArchive::WillBeDuplicated(LPCTSTR lpszFilePath, bool bFullPath, bool bFileNameOnly , int iWhat)
{
	CZipString szFile;
	// we predict with bExactly set to false, because FindFile converts all filanames anyway
	if (bFileNameOnly)
	{
		CZipPathComponent zpc(lpszFilePath);
		szFile = PredictFileNameInZip(zpc.GetFileName(), false, iWhat);
	}
	else
		szFile = PredictFileNameInZip(lpszFilePath, bFullPath, iWhat);
	return FindFile(szFile, ffDefault, bFileNameOnly);
}


// it'll get up to the next file or to the end of file (bad if zip corrupted or not-ordered by offsett or redundant bytes added)
bool CZipArchive::GetFromArchive(CZipArchive& zip, WORD uIndex, int iReplaceIndex, bool bKeepSystComp, CZipActionCallback* pCallback)
{

	if (IsClosed() || zip.IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_iFileOpened || zip.m_iFileOpened)
	{
		TRACE(_T("%s(%i) : You cannot get files from another archive if there is a file opened.\n"),__FILE__,__LINE__);
		return false;
	}

	if (zip.m_storage.IsSpanMode())
	{
		TRACE(_T("%s(%i) : You cannot get files from the disk spannig archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_storage.IsSpanMode() == -1)
	{
		TRACE(_T("%s(%i) : You cannot add files to the existing disk spannig archive.\n"),__FILE__,__LINE__);
		return false;
	}

	ASSERT(m_info.m_pBuffer.GetSize() > 0);

	bool bIsSpan = m_storage.IsSpanMode() == 1;

	CZipFileHeader fh;
	if (!zip.GetFileInfo(fh, uIndex))
		return false;
	CZipAbstractFile* pFile = zip.m_storage.m_pFile;


	DWORD uEndOffset;
	if (uIndex < zip.GetCount() - 1)
	{
		CZipFileHeader fhTemp;
		if (!zip.GetFileInfo(fhTemp, uIndex+1))
			return false;
		uEndOffset = fhTemp.m_uOffset;
	}
	else
	{
		CZipCentralDir::Info info;
		zip.m_centralDir.GetInfo(info);
		if (info.m_bOnDisk)
			uEndOffset = info.m_uOffset;
		else
			uEndOffset = pFile->GetLength();
	}
	uEndOffset += zip.m_centralDir.GetBytesBefore();
	DWORD uStartOffset = zip.m_centralDir.GetBytesBefore() + fh.m_uOffset + fh.GetSize(true);
	DWORD uTotalToMove = uEndOffset - uStartOffset, uTotalMoved = 0;

	DWORD uPredictedSize = fh.m_uComprSize +
		(fh.IsDataDescr() ? ZIPARCHIVE_DATADESCRIPTOR_LEN : 0);
	if (uTotalToMove > uPredictedSize + 4/* may be or may be not a signature*/)
		uTotalToMove = uPredictedSize + 4;
	else if (uTotalToMove < uPredictedSize)
		ThrowError(CZipException::badZipFile);

	// conversion stuff
	CZipString szFileNameConverted, szFileName;
	bool bConvertSystem = !bKeepSystComp && fh.GetSystemCompatibility() != m_iArchiveSystCompatib;

	// GetFileInfo always converts the filename regardless of zip.m_centralDir.m_bConvertAfterOpen value
	szFileNameConverted = fh.GetFileName();
	if (bConvertSystem)
	{
		DWORD uAttr = fh.GetSystemAttr();
		fh.SetSystemCompatibility(m_iArchiveSystCompatib);
		fh.SetSystemAttr(uAttr);
	}

	ZipCompatibility::FileNameUpdate(fh, false);
	szFileName = fh.GetFileName();


	bool bNeedDataDescr = bIsSpan && !fh.IsDataDescr();
	if (bNeedDataDescr)
		fh.m_uFlag |= 8; // data descriptor present


	// needed by InsertFindFastElement
	if (m_centralDir.IsFindFastEnabled())
		fh.SetFileName(szFileNameConverted);


	if (!UpdateReplaceIndex(iReplaceIndex, szFileNameConverted))
		return false;

	bool bReplace = iReplaceIndex >= 0;
	int iCallbackType = 0;
	if (pCallback)
		iCallbackType = pCallback->m_iType;

	// if the same callback is applied to cbReplace, then the previous information about the type will be lost
	CZipFileHeader* pHeader = m_centralDir.AddNewFile(fh, iReplaceIndex); // must be converted when adding because of InsertFastElement
	if (bReplace)
		MakeSpaceForReplace(iReplaceIndex, uTotalToMove + fh.GetSize(true), szFileNameConverted);

	if (pCallback)
	{
		pCallback->m_iType = iCallbackType;
		pCallback->Init(szFileNameConverted, zip.GetArchivePath());
		pCallback->SetTotal(fh.m_uComprSize);
	}

	if (m_centralDir.IsFindFastEnabled())
		pHeader->SetFileName(szFileName);

	// must be written as not converted
	pHeader->WriteLocal(m_storage);

	// made a correction to what was set in WriteLocal
	pHeader->m_uOffset -= m_centralDir.GetBytesBefore();

	// we keep in converted in memory
	if (m_centralDir.m_bConvertAfterOpen)
		 pHeader->SetFileName(szFileNameConverted);


	// skip reading the local file header

	pFile->Seek(uStartOffset, CZipAbstractFile::begin);


	DWORD uPack = uTotalToMove > m_info.m_pBuffer.GetSize() ? m_info.m_pBuffer.GetSize() : uTotalToMove;
	char* buf = (char*)m_info.m_pBuffer;

	DWORD size_read;

	int iAborted = 0;
	bool bBreak = false;
	if (uPack)
		do
		{
			size_read = pFile->Read(buf, uPack);
			if (!size_read)
				break;
			if (uTotalMoved + size_read > uTotalToMove)
			{
				size_read = uTotalToMove - uTotalMoved;
				if (!size_read)  // this is for protection
					break;
				bBreak = true;
			}

			m_storage.Write(buf, size_read, false);
			uTotalMoved += size_read;
			if (pCallback)
				if (!(*pCallback)(size_read))
				{
					if (uTotalToMove != uTotalMoved)
					{
						if (!bIsSpan && !bReplace)
						{
							m_centralDir.RemoveLastFile();
							iAborted = CZipException::abortedSafely;
						}
						else
							iAborted = CZipException::abortedAction;
					}
					else
						iAborted = CZipException::abortedSafely; // we did it!
					break;

				}
		}
		while (!bBreak);

	if (iAborted)
		CZipException::Throw(iAborted); // throw to distuingiush from other return codes

	// copying from non-span to span or from span without data description to span
	// so add the data descriptor

	m_centralDir.m_pOpenedFile = NULL;
	if (bNeedDataDescr && uTotalMoved == uTotalToMove)
	{
		const int iToWrite = ZIPARCHIVE_DATADESCRIPTOR_LEN + 4;
		CZipAutoBuffer buf(iToWrite);
		memcpy(buf, m_storage.m_gszExtHeaderSignat, 4);
		pHeader->GetCrcAndSizes(buf + 4);
		m_storage.Write(buf, iToWrite, true);

	}
	m_storage.Flush();
	if (uTotalMoved < uTotalToMove)
		ThrowError(CZipException::badZipFile);


	if (pCallback)
		pCallback->CallbackEnd();

	return true;
}

bool CZipArchive::GetFromArchive(CZipArchive& zip, CZipWordArray &aIndexes, bool bKeepSystComp)
{
	aIndexes.Sort(true);
	int iFiles = aIndexes.GetSize();
	m_info.Init();
	try
	{
		for (int i = 0; i < iFiles; i++)
		{
			int iFileIndex = aIndexes[i];
			if (!m_centralDir.IsValidIndex(iFileIndex))
			if (!GetFromArchive(zip, iFileIndex, -1, bKeepSystComp, GetCallback(cbGetFromArchive)))
			{
				m_info.ReleaseBuf();
				return false;
			}
		}
	}
	catch (...)
	{
		m_info.ReleaseBuf();
		throw;
	}
	m_info.ReleaseBuf();
	if (m_bAutoFlush)
		Flush();
	return true;
}
bool CZipArchive::RenameFile(WORD uIndex, LPCTSTR lpszNewName)
{
	if (IsClosed())
	{
		TRACE(_T("%s(%i) : ZipArchive is closed.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_storage.IsSpanMode())
	{
		TRACE(_T("%s(%i) : You cannot rename files in the disk spannig archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (m_iFileOpened)
	{
		TRACE(_T("%s(%i) : You cannot rename a file if there is a file opened.\n"),__FILE__,__LINE__);
		return false;
	}
	CZipFileHeader fh, fhNew;
	if (!GetFileInfo(fh, uIndex))
		return false;
	CZipString szNewName(lpszNewName);
	if (fh.IsDirectory())
		CZipPathComponent::AppendSeparator(szNewName);
	else
		CZipPathComponent::RemoveSeparators(szNewName);
	if (fh.GetFileName().Collate(szNewName) == 0)
		return true;
	fhNew.SetSystemCompatibility(m_iArchiveSystCompatib);

	fhNew.SetFileName(szNewName);
	ZipCompatibility::FileNameUpdate(fhNew, false);
	ZipCompatibility::FileNameUpdate(fh, false); // in case the conversion changes the filename size
	WORD uFileNameLen = fh.GetFileNameSize();
	WORD uNewFileNameLen = fhNew.GetFileNameSize();
	int iDelta = uNewFileNameLen - uFileNameLen;
	int iOffset = 0;
	CZipAutoBuffer buf, *pBuf;
	m_centralDir.RemoveFromDisk(); // does m_storage.Flush();
	if (iDelta != 0)
	{
		// we need to make more or less space

		m_info.Init();
		DWORD uStartOffset = fh.m_uOffset + 30 + uFileNameLen;
		DWORD uFileLen = m_storage.m_pFile->GetLength();
		DWORD uEndOffset = uFileLen - m_centralDir.GetBytesBefore();
		CZipActionCallback* pCallback = GetCallback(cbRename);
		if (pCallback)
		{
			// do it right and sent the notification
			pCallback->Init(fh.GetFileName(), GetArchivePath());
			pCallback->SetTotal(uEndOffset - uStartOffset);
		}
		bool bForward = iDelta > 0;
		if (bForward)
			m_storage.m_pFile->SetLength(uFileLen + iDelta); // ensure the seek is correct

		MovePackedFiles(uStartOffset, uEndOffset, std::abs(iDelta), pCallback, bForward);
		if (pCallback)
			pCallback->CallbackEnd();

		if (!bForward)
			m_storage.m_pFile->SetLength(uFileLen + iDelta); // delta < 0; shrink the file

		m_info.ReleaseBuf();

		int iSize = GetCount();
		for (int i = uIndex + 1; i < iSize; i++)
			m_centralDir[i]->m_uOffset += iDelta;
		buf.Allocate(4+uNewFileNameLen);
		WORD uExtraFieldSize = fh.GetExtraFieldSize();
		memcpy(buf, &uNewFileNameLen, 2);
		memcpy(buf + 2, &uExtraFieldSize, 2); // to write everything at once
		memcpy(buf + 4, fhNew.m_pszFileName, uNewFileNameLen);
		pBuf = &buf;
		iOffset = -4;
	}
	else
		pBuf = &fhNew.m_pszFileName;

	m_storage.m_pFile->Seek(m_centralDir.GetBytesBefore() + fh.m_uOffset + 30 + iOffset, CZipAbstractFile::begin);
	m_storage.m_pFile->Write(buf, buf.GetSize());
	m_centralDir.RenameFile(uIndex, szNewName);
	if (m_bAutoFlush)
		Flush();

	return true;
}

bool CZipArchive::UpdateReplaceIndex(int& iReplaceIndex, LPCTSTR lpszNewFileName)
{
	if (iReplaceIndex == -2)
		iReplaceIndex = FindFile(lpszNewFileName);
	if (iReplaceIndex < 0)
	{
		if (iReplaceIndex != -1)
			iReplaceIndex = -1;
		return true;
	}

	if (GetSpanMode()!=0)
	{
		TRACE(_T("%s(%i) : You cannot replace files in a disk-spanning archive.\n"),__FILE__,__LINE__);
		return false;
	}

	if (!m_centralDir.IsValidIndex(iReplaceIndex))
	{
		TRACE(_T("%s(%i) : Not valid replace index.\n"),__FILE__,__LINE__);
		return false;
	}
	if (iReplaceIndex == GetCount() - 1) // replacing last file in the archive
	{
		RemoveLast(true);
		iReplaceIndex = -1;
	}
	return true;
}

void CZipArchive::MakeSpaceForReplace(int iReplaceIndex, DWORD uTotal, LPCTSTR lpszFileName)
{

	ASSERT(iReplaceIndex < GetCount() - 1);
	DWORD uReplaceStart = m_storage.m_pFile->GetPosition() - m_centralDir.GetBytesBefore();
	DWORD uReplaceEnd = m_centralDir.m_headers[iReplaceIndex + 1]->m_uOffset;
	DWORD uReplaceTotal = uReplaceEnd - uReplaceStart;
	int iDelta = uTotal - uReplaceTotal;

	if (iDelta != 0)
	{

		//	m_info.Init(); don't - the calling functions will
		CZipActionCallback* pCallback = GetCallback(CZipArchive::cbReplace);
		DWORD uFileLen = m_storage.m_pFile->GetLength();
		DWORD uUpperLimit = uFileLen - m_centralDir.GetBytesBefore(); // will be added in MovePackedFiles
		if (pCallback)
		{
			pCallback->Init(lpszFileName, GetArchivePath());
			pCallback->SetTotal(uUpperLimit - uReplaceEnd);
		}

		bool bForward = iDelta > 0;
		if (bForward)
			m_storage.m_pFile->SetLength(uFileLen + iDelta); // ensure the seek is correct

		MovePackedFiles(uReplaceEnd, uUpperLimit, std::abs(iDelta), pCallback, bForward);

		if (!bForward)
			m_storage.m_pFile->SetLength(uFileLen + iDelta); // delta < 0; shrink the file

		m_storage.m_pFile->Seek(uReplaceStart, CZipAbstractFile::begin);
		int iSize = m_centralDir.m_headers.GetSize();
		for (int i = iReplaceIndex + 1; i < iSize; i++)
			m_centralDir.m_headers[i]->m_uOffset += iDelta;
		if (pCallback)
			pCallback->CallbackEnd();
	}
}

void CZipArchive::MovePackedFiles(DWORD uStartOffset, DWORD uEndOffset, DWORD uMoveBy, CZipActionCallback* pCallback, bool bForward)
{
	ASSERT(m_info.m_pBuffer.GetSize() > 0);
	uStartOffset += m_centralDir.GetBytesBefore();
	uEndOffset += m_centralDir.GetBytesBefore();


	DWORD uTotalToMove = uEndOffset - uStartOffset;
	DWORD uPack = uTotalToMove > m_info.m_pBuffer.GetSize() ? m_info.m_pBuffer.GetSize() : uTotalToMove;
	char* buf = (char*)m_info.m_pBuffer;


	DWORD size_read;
	bool bBreak = false;
	do
	{

		if (uEndOffset - uStartOffset < uPack)
		{
			uPack = uEndOffset - uStartOffset;
			if (!uPack)
				break;
			bBreak = true;

		}
		DWORD uPosition = bForward ? uEndOffset - uPack : uStartOffset;

		m_storage.m_pFile->Seek(uPosition, CZipAbstractFile::begin);
		size_read = m_storage.m_pFile->Read(buf, uPack);
		if (!size_read)
			break;

		if (bForward)
			uPosition += uMoveBy;
		else
			uPosition -= uMoveBy;
		m_storage.m_pFile->Seek(uPosition, CZipAbstractFile::begin);
		m_storage.m_pFile->Write(buf, size_read);
		if (bForward)
			uEndOffset -= size_read;
		else
			uStartOffset += size_read;
		if (pCallback)
			if (!(*pCallback)(size_read))
				ThrowError(CZipException::abortedAction);
	}
	while (!bBreak);

	if (uEndOffset != uStartOffset)
		ThrowError(CZipException::internal);

}
