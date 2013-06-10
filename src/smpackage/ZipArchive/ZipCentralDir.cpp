///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipCentralDir.cpp $
// $Archive: /ZipArchive/ZipCentralDir.cpp $
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
#include "ZipCentralDir.h"
#include "ZipArchive.h"
#include "ZipFileMapping.h"
#include "ZipPlatform.h"


#define CENTRALDIRSIZE	22

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
char CZipCentralDir::m_gszSignature[] = {0x50, 0x4b, 0x05, 0x06};
CZipCentralDir::CZipCentralDir()
{
	m_bConvertAfterOpen  = true;
	m_bFindFastEnabled = false;
	m_bCaseSensitive = false;
	m_pCompare = GetCZipStrCompFunc(ZipPlatform::GetSystemCaseSensitivity());
	m_pStorage = NULL;
	m_pOpenedFile = NULL;
	m_iBufferSize = 32768;
	
}

void CZipCentralDir::Init()
{
	m_info.m_bOnDisk = false;
	m_info.m_uBytesBeforeZip = m_info.m_uCentrDirPos = 0;
	m_pOpenedFile = NULL;
	m_pszComment.Release();

}

CZipCentralDir::~CZipCentralDir()
{
	Clear();
}

void CZipCentralDir::Read()
{
	ASSERT(m_pStorage);
	WORD uCommentSize;
	m_info.m_uCentrDirPos = Locate();
	m_pStorage->m_pFile->Seek(m_info.m_uCentrDirPos, CZipAbstractFile::begin);
	CZipAutoBuffer buf(CENTRALDIRSIZE);

	int uRead = m_pStorage->m_pFile->Read(buf, CENTRALDIRSIZE);
	if (uRead != CENTRALDIRSIZE)
		ThrowError(CZipException::badZipFile);
	memcpy(&m_szSignature,			buf, 4);
	memcpy(&m_info.m_uThisDisk,		buf + 4, 2);
	memcpy(&m_info.m_uDiskWithCD,	buf + 6, 2);
	memcpy(&m_info.m_uDiskEntriesNo,buf + 8, 2);
	memcpy(&m_info.m_uEntriesNumber,buf + 10, 2);
	memcpy(&m_info.m_uSize,			buf + 12, 4);
	memcpy(&m_info.m_uOffset,		buf + 16, 4);
	memcpy(&uCommentSize,			buf + 20, 2);
	buf.Release();


	m_pStorage->UpdateSpanMode(m_info.m_uThisDisk);
	// if m_uThisDisk is not zero, it is enough to say that it is a multi disk archive
	ASSERT((!m_info.m_uThisDisk && (m_info.m_uEntriesNumber == m_info.m_uDiskEntriesNo) && !m_info.m_uDiskWithCD) || m_info.m_uThisDisk);

			

	if (!m_pStorage->IsSpanMode() && !m_info.CheckIfOK_1())
		ThrowError(CZipException::badZipFile);

	if (uCommentSize)
	{
		m_pszComment.Allocate(uCommentSize);
		uRead = m_pStorage->m_pFile->Read(m_pszComment, uCommentSize);
		if (uRead != uCommentSize)
			ThrowError(CZipException::badZipFile);
	}
	
	m_info.SetBytesBeforeZip(m_pStorage->IsSpanMode() != 0);

	if (!m_info.CheckIfOK_2())
		ThrowError(CZipException::badZipFile);

	m_info.m_bOnDisk = true;
	m_pStorage->ChangeDisk(m_info.m_uDiskWithCD);

	if (!m_info.m_uSize)
		return;

	ReadHeaders();
}

DWORD CZipCentralDir::Locate()
{

	// maximum size of end of central dir record
	long uMaxRecordSize = 0xffff + CENTRALDIRSIZE;
	DWORD uFileSize = m_pStorage->m_pFile->GetLength();

	if ((DWORD)uMaxRecordSize > uFileSize)
		uMaxRecordSize = uFileSize;

	CZipAutoBuffer buf(m_iBufferSize);

	long uPosInFile = 0;
	int uRead = 0;
	// backward reading
	while (uPosInFile < uMaxRecordSize)
	{
		uPosInFile = uRead + m_iBufferSize;
		if (uPosInFile > uMaxRecordSize)
			uPosInFile = uMaxRecordSize;

		int iToRead = uPosInFile - uRead;

		m_pStorage->m_pFile->Seek(-uPosInFile, CZipAbstractFile::end);
		int iActuallyRead = m_pStorage->m_pFile->Read(buf, iToRead);
		if (iActuallyRead != iToRead)
			ThrowError(CZipException::badZipFile);
		// search from the very last bytes to prevent an error if inside archive 
		// there are packed other arhives
		for (int i = iToRead - 4; i >=0 ; i--)
			if (!memcmp((char*)buf + i, m_gszSignature, 4))	
				return uFileSize - (uPosInFile - i);

		uRead += iToRead - 3;

	}
	
	ThrowError(CZipException::cdirNotFound);
	return 0;
}

void CZipCentralDir::ThrowError(int err) const
{
	CZipException::Throw(err, m_pStorage->m_pFile->GetFilePath());
}


void CZipCentralDir::ReadHeaders()
{
	m_pStorage->m_pFile->Seek(m_info.m_uOffset + m_info.m_uBytesBeforeZip, CZipAbstractFile::begin);
	RemoveHeaders(); //just in case
	for (int i = 0; i < m_info.m_uEntriesNumber; i++)
	{
		CZipFileHeader* pHeader = new CZipFileHeader;
		m_headers.Add(pHeader);

		if (!pHeader->Read(m_pStorage))
			ThrowError(CZipException::badZipFile);
		ConvertFileName(true, true, pHeader);
	}
	SortHeaders(); // this is necessary when deleting files and removing data descriptors
	if (m_bFindFastEnabled)
		BuildFindFastArray(m_bCaseSensitive);
}

void CZipCentralDir::SortHeaders()
{
	// we cannot use the Sort method of the CZipWordArray, 
	//	because we store pointers (and we need to store pointers 
	//	to make sure that the address of the CZipFileHeader structure 
	//	remains the same while working with the library)
	int iSize = m_headers.GetSize();
	if (iSize)
		qsort((void*)&(m_headers[0]),iSize , sizeof(CZipFileHeader*), CompareHeaders);
}

void CZipCentralDir::Clear(bool bEverything)
{
	m_pOpenedFile = NULL;
	m_pLocalExtraField.Release();
	if (bEverything)
	{
		RemoveHeaders();
		m_findarray.RemoveAll();
		m_pszComment.Release();
	}
}


bool CZipCentralDir::IsValidIndex(int uIndex)const
{
	return uIndex < m_headers.GetSize() && uIndex >= 0;
}


void CZipCentralDir::OpenFile(WORD uIndex)
{
	WORD uLocalExtraFieldSize;
	m_pOpenedFile = (*this)[uIndex];
	m_pStorage->ChangeDisk(m_pOpenedFile->m_uDiskStart);
	m_pStorage->m_pFile->Seek(m_pOpenedFile->m_uOffset + m_info.m_uBytesBeforeZip, CZipAbstractFile::begin);
	if (!m_pOpenedFile->ReadLocal(m_pStorage, uLocalExtraFieldSize))
		ThrowError(CZipException::badZipFile);


	m_pLocalExtraField.Release(); // just in case
	if (uLocalExtraFieldSize)
	{
		int iCurrDsk = m_pStorage->GetCurrentDisk();
		m_pLocalExtraField.Allocate(uLocalExtraFieldSize);
		m_pStorage->Read(m_pLocalExtraField, uLocalExtraFieldSize, true);
		if (m_pStorage->GetCurrentDisk() != iCurrDsk)
			ThrowError(CZipException::badZipFile);
	}
}

void CZipCentralDir::CloseFile(bool bAfterException)
{
	if (!m_pOpenedFile)
		return;
	m_pLocalExtraField.Release();
	if (!bAfterException && m_pOpenedFile->IsDataDescr())
	{
		CZipAutoBuffer buf(12);
		m_pStorage->Read(buf, 4, false);
		// in span mode, files that are divided between disks have bit 3 of flag set
		// which tell about the presence of the data descriptor after the compressed data
		// This signature may be in the disk spanning archive that is one volume only
		// (it is detected as a non disk spanning archive)
		if (memcmp(buf, CZipStorage::m_gszExtHeaderSignat, 4) != 0) // there is no signature
				m_pStorage->m_pFile->Seek(-4, CZipAbstractFile::current);

		
		m_pStorage->Read(buf, 12, false);
		if (!m_pOpenedFile->CheckCrcAndSizes(buf))
			ThrowError(CZipException::badZipFile);
	}
	m_pOpenedFile = NULL;
}

// add new header using the argument as a template
CZipFileHeader* CZipCentralDir::AddNewFile(const CZipFileHeader & header, int iReplaceIndex)
{
	CZipFileHeader* pHeader = new CZipFileHeader(header);
	m_pOpenedFile = pHeader;
	WORD uIndex;
	DWORD uOffset = 0;
	bool bReplace = IsValidIndex(iReplaceIndex);
	if (bReplace)
	{
		CZipFileHeader* pfh = m_headers[iReplaceIndex];
		uOffset = pfh->m_uOffset + m_info.m_uBytesBeforeZip;
		RemoveFile(pfh, iReplaceIndex, false);
		m_headers.InsertAt(iReplaceIndex, pHeader);
		uIndex = (WORD)iReplaceIndex;
	}
	else
		uIndex = m_headers.Add(pHeader);

	if (m_bFindFastEnabled)
		InsertFindFastElement(pHeader, uIndex); // GetCount > 0, 'cos we've just added a header
	RemoveFromDisk();
	if (bReplace)
		m_pStorage->m_pFile->Seek(uOffset, CZipAbstractFile::begin);
	else
		m_pStorage->m_pFile->SeekToEnd();
	return pHeader;
}


void CZipCentralDir::RemoveFromDisk()
{
	if (m_info.m_bOnDisk)
	{
		ASSERT(!m_pStorage->IsSpanMode()); // you can't add files to the existing disk span archive or to delete them from it
		m_pStorage->m_pFile->SetLength(m_info.m_uBytesBeforeZip + m_info.m_uOffset);
		m_info.m_bOnDisk = false;
	}
	else
		m_pStorage->Flush(); // if remove from disk is requested, then the archive modification will follow, so flush the buffers
}


void CZipCentralDir::CloseNewFile()
{
	CZipAutoBuffer buf(ZIPARCHIVE_DATADESCRIPTOR_LEN + 4);
	short iToWrite = 0;
	bool bIsSpan = m_pStorage->IsSpanMode() != 0;
	bool bEncrypted = m_pOpenedFile->IsEncrypted();
	if (m_pOpenedFile->IsDataDescr())
	{
		if (bIsSpan || bEncrypted)
		{
			memcpy(buf, m_pStorage->m_gszExtHeaderSignat, 4);
			iToWrite += 4;
		}
	}
	else /*if (!IsSpan)*/
	{
		ASSERT(!bIsSpan && !bEncrypted);
		m_pStorage->Flush();
		// the offset contains bytes before zip (set while writting the local header)
		m_pStorage->m_pFile->Seek(m_pOpenedFile->m_uOffset + 14, CZipAbstractFile::begin);
		// we don't have to restore the pointer, because before adding a new file, 
		// the pointer is moved to the end
	}

	m_pOpenedFile->GetCrcAndSizes(buf + iToWrite);
	iToWrite += ZIPARCHIVE_DATADESCRIPTOR_LEN;

	// offset set during writing the local header
	m_pOpenedFile->m_uOffset -= m_info.m_uBytesBeforeZip;
	
	// write the data descriptor and a disk spanning signature at once
	m_pStorage->Write(buf, iToWrite, true);
	if (!bIsSpan)
	{
		if (bEncrypted)
		{
			// write the information to the local header too
			m_pStorage->Flush();
			m_pStorage->m_pFile->Seek(m_info.m_uBytesBeforeZip + m_pOpenedFile->m_uOffset + 14, CZipAbstractFile::begin);
			m_pStorage->Write(buf + 4, ZIPARCHIVE_DATADESCRIPTOR_LEN, true);
		}
		m_pStorage->Flush();
	}

	m_pOpenedFile = NULL;

}

void CZipCentralDir::Write(CZipActionCallback* pCallback)
{
	if (m_info.m_bOnDisk)
		return;
	if (!m_pStorage->IsSpanMode())
	{
		m_pStorage->Flush();
		m_pStorage->m_pFile->SeekToEnd();
	}

// 	else
// 		// we are at the end already

	m_info.m_uEntriesNumber = (WORD)m_headers.GetSize();
	m_info.m_uSize = 0;
	bool bDontAllowDiskChange = false;
	// if there is a disk spanning archive in creation and it is only one-volume,
	//	(current disk is 0 so far, no bytes has been written so we know they are 
	//  all in the buffer)	make sure that it will be after writing central dir 
	// and make it a non disk spanning archive
	if (m_pStorage->IsSpanMode() && m_pStorage->GetCurrentDisk() == 0)
	{
		DWORD uVolumeFree = m_pStorage->VolumeLeft();
		// calculate the size of data descriptors already in the buffer or on the disk
		// (they will be removed in the non disk spanning archive):
		// multi span signature at the beginnig (4 bytes) + the size of the data 
		// descr. for each file (multi span signature + 12 bytes data)
		// the number of bytes to add: central dir size - total to remove;
		DWORD uToGrow = GetSize(true) - (4 + m_info.m_uEntriesNumber * (4 + 12)); 
		if (uVolumeFree >= uToGrow) 
		// lets make sure it will be one-disk archive
		{
			// can the operation be done only in the buffer?
			if (!m_pStorage->m_iBytesWritten && // no bytes on the disk yet
				(m_pStorage->GetFreeInBuffer() >= uToGrow)) // is the buffer big enough?
			{
					RemoveDataDescr(true);
					bDontAllowDiskChange = true; // if the disk change occurs somehow, we'll throw an error later
			}
			else
			{
				m_pStorage->Flush();
				if (RemoveDataDescr(false))
					bDontAllowDiskChange = true; // if the disk change occurs somehow, we'll throw an error later
			}
		}
	}

	try
	{
		WriteHeaders(pCallback, bDontAllowDiskChange || !m_pStorage->IsSpanMode());

		m_info.m_uThisDisk = (WORD)m_pStorage->GetCurrentDisk();
		DWORD uSize = WriteCentralEnd();
		if (bDontAllowDiskChange)
		{
			if (m_pStorage->GetCurrentDisk() != 0)
				ThrowError(CZipException::badZipFile);
		}
		// if after adding a central directory there is a disk change, 
		// update the information and write it again
		if (m_info.m_uThisDisk != m_pStorage->GetCurrentDisk())
		{
			m_info.DiskChange(m_pStorage->GetCurrentDisk());

			if (m_pStorage->m_uBytesInWriteBuffer >= uSize)
				// if the data is still in the buffer, simply remove it
				m_pStorage->m_uBytesInWriteBuffer -= uSize;
			else
			{
				m_pStorage->Flush();
				m_pStorage->m_iBytesWritten -= uSize;
				m_pStorage->m_pFile->SeekToBegin();	
			}
			
			WriteCentralEnd();
		}
	}
	catch (...)
	{
		if (bDontAllowDiskChange)
		{
			m_pStorage->FinalizeSpan();
			m_info.m_uThisDisk = 0;
		}
		throw;
	}
	m_info.m_bOnDisk = true;
}

void CZipCentralDir::WriteHeaders(CZipActionCallback* pCallback, bool bOneDisk)
{
	m_info.m_uDiskEntriesNo = 0;
	m_info.m_uDiskWithCD = (WORD)m_pStorage->GetCurrentDisk();
	m_info.m_uOffset = m_pStorage->GetPosition() - m_info.m_uBytesBeforeZip;
	if (!m_info.m_uEntriesNumber)
		return;

	WORD iDisk = m_info.m_uDiskWithCD;
	int iStep = 0; // for the compiler

	if (pCallback)
	{
		pCallback->Init();
		pCallback->SetTotal(m_info.m_uEntriesNumber);
		iStep = CZipActionCallback::m_iStep;// we don't want to wait forever
	}

	int iAborted = 0;
	for (int i = 0; i < m_info.m_uEntriesNumber; i++)
	{
		CZipFileHeader* pHeader = (*this)[i];
		

		CZipString szRemember;
		if (m_bConvertAfterOpen)
			// if CZipArchive::Flush is called we will be still using the archive, so restore changed name
			szRemember = pHeader->GetFileName();

		ConvertFileName(false, true, pHeader);
		m_info.m_uSize += pHeader->Write(m_pStorage);

		if (m_bConvertAfterOpen)
			pHeader->SetFileName(szRemember);

		if (m_pStorage->GetCurrentDisk() != iDisk)
		{
			m_info.m_uDiskEntriesNo = 1;
			iDisk = (WORD)m_pStorage->GetCurrentDisk();
			// update the information about the offset and starting disk if the 
			// first header was written on the new disk
			if (i == 0)
			{
				m_info.m_uOffset = 0;
				m_info.m_uDiskWithCD = iDisk;
			}
		}
		else 
			m_info.m_uDiskEntriesNo++;
		if (pCallback && !(i%iStep))
			if (!pCallback->Callback(iStep))
			{
				
				if (bOneDisk) 
				{
					if (!m_pStorage->IsSpanMode())
						m_pStorage->EmptyWriteBuffer();
					else
						m_pStorage->Flush(); // must be flush before - flush was not called in span mode
					
					// remove saved part from the disk
					m_pStorage->m_pFile->SetLength(m_info.m_uBytesBeforeZip + m_info.m_uOffset);
//	 				We can now abort safely
					iAborted = CZipException::abortedSafely;
				}
				else
					iAborted = CZipException::abortedAction;
				break;
			}
	}

	if (pCallback)
		pCallback->CallbackEnd();

	if (iAborted)
		ThrowError(iAborted);
}

DWORD CZipCentralDir::WriteCentralEnd()
{
	DWORD uSize = GetSize();
	CZipAutoBuffer buf(uSize);
	WORD uCommentSize = (WORD)m_pszComment.GetSize();
	memcpy(buf, m_gszSignature, 4);
	memcpy(buf + 4, &m_info.m_uThisDisk, 2);
	memcpy(buf + 6, &m_info.m_uDiskWithCD, 2);
	memcpy(buf + 8, &m_info.m_uDiskEntriesNo, 2);
	memcpy(buf + 10, &m_info.m_uEntriesNumber, 2);
	memcpy(buf + 12, &m_info.m_uSize, 4);
	memcpy(buf + 16, &m_info.m_uOffset, 4);
	memcpy(buf + 20, &uCommentSize, 2);
	memcpy(buf + 22, m_pszComment, uCommentSize);
	m_pStorage->Write(buf, uSize, true);
	return uSize;
}

void CZipCentralDir::RemoveAll()
{
	m_findarray.RemoveAll();
	RemoveHeaders();
}

void CZipCentralDir::RemoveFile(CZipFileHeader* pHeader, int iIndex, bool bShift)
{

	if (iIndex == -1)
	{
		int iCount = m_headers.GetSize();
		for (int i = 0; i < iCount; i++)
			if (pHeader == m_headers[i])
			{
				iIndex = i;
				break;
			}
	}
	ASSERT(iIndex != -1 || pHeader);
	if (!pHeader)
		pHeader = m_headers[iIndex];

	if (m_bFindFastEnabled)
	{
		int i = FindFileNameIndex(pHeader->GetFileName());
		ASSERT(i != -1);
		int uIndex = m_findarray[i].m_uIndex;
		m_findarray.RemoveAt(i);
		// shift down the indexes
		
		if (bShift)
		{
			int iSize = m_findarray.GetSize();
			for (int j = 0; j < iSize; j++)
			{
				if (m_findarray[j].m_uIndex > uIndex)
					m_findarray[j].m_uIndex--;
			}
		}
	}


	
	if (iIndex != -1)
	{
		delete pHeader;
		m_headers.RemoveAt(iIndex);
	}
	
}


DWORD CZipCentralDir::GetSize(bool bWhole) const
{
	DWORD uHeaders = 0;
	int iCount = m_headers.GetSize();
	if (bWhole)
	{
		for (int i = 0; i < iCount; i++)
		{
			const CZipFileHeader* pHeader = m_headers[i];
			uHeaders += pHeader->GetSize();
		}
	}
	return CENTRALDIRSIZE + m_pszComment.GetSize() + uHeaders;
}

bool CZipCentralDir::RemoveDataDescr(bool bFromBuffer)
{
	ziparchv::CZipFileMapping fm;
	char* pFile;
	DWORD uSize;
	if (bFromBuffer)
	{
		uSize = m_pStorage->m_uBytesInWriteBuffer;
		pFile = m_pStorage->m_pWriteBuffer;
	}
	else
	{
		uSize = m_pStorage->m_pFile->GetLength();
		// we cannot use CZipMemFile in multidisk archive
		// so it MUST be CZipFile
		if (!fm.CreateMapping(static_cast<CZipFile*>(m_pStorage->m_pFile)))
			return false;
		pFile = fm.GetMappedMemory();
	}

	DWORD uOffsetToChange = 4;
	DWORD uPosInBuffer = 0;
	DWORD uExtraHeaderLen;
	int iCount = m_headers.GetSize();
	for (int i = 0; i < iCount; i++)
	{
		// update the flag value in the local and central header
// 		int uDataDescr = (m_headers[i]->m_uFlag & 8) ? (4 + 12) : 0;

		CZipFileHeader* pHeader = m_headers[i];


		char* pSour = pFile + pHeader->m_uOffset;
		
		if (!pHeader->IsEncrypted())
		{
			// removing data descriptor
			pHeader->m_uFlag &= ~8;
			// update local header:
			// write modified flag in the local header
			memcpy(pSour + 6, &pHeader->m_uFlag, 2);
			uExtraHeaderLen = 4/*ext. header signature*/ + 12/*data descriptor*/;
		}
		else
			// do not remove data descriptors from encrypted files
			uExtraHeaderLen = 0;

		// update crc32 and sizes' values
		pHeader->GetCrcAndSizes(pSour+ 14);

		DWORD uToCopy = (i == (iCount - 1) ? uSize : m_headers[i + 1]->m_uOffset)
			- pHeader->m_uOffset - uExtraHeaderLen;

		memmove(pFile + uPosInBuffer, pSour, uToCopy);

		uPosInBuffer += uToCopy;
		pHeader->m_uOffset -= uOffsetToChange;
		uOffsetToChange += uExtraHeaderLen;
	}

	if (bFromBuffer)
		m_pStorage->m_uBytesInWriteBuffer = uPosInBuffer;
	else
	{
		m_pStorage->m_iBytesWritten = uPosInBuffer;
		fm.RemoveMapping();
		m_pStorage->m_pFile->SetLength(uPosInBuffer);
	}
	return true;
}

void CZipCentralDir::RemoveHeaders()
{
	int iCount = m_headers.GetSize();
	for (int i = 0; i < iCount; i++)
		delete m_headers[i];
	m_headers.RemoveAll();
}



void CZipCentralDir::ConvertAll()
{
	ASSERT(!m_bConvertAfterOpen);
	int iCount = m_headers.GetSize();
	for (int i = 0; i < iCount; i++)
		ConvertFileName(true, false, m_headers[i]);
	m_bConvertAfterOpen = true;
}

void CZipCentralDir::BuildFindFastArray( bool bCaseSensitive )
{
	m_findarray.RemoveAll();
	m_bCaseSensitive = bCaseSensitive;
	m_pCompare = GetCZipStrCompFunc(bCaseSensitive);
	int iCount = m_headers.GetSize();
	if (!m_bConvertAfterOpen)
	{
		for (int i = 0; i < iCount; i++)
		{
			CZipFileHeader fh = *m_headers[i];
			ConvertFileName(true, false, &fh);
			InsertFindFastElement(&fh, i); // this method requires the name to be already converted
		}
	}
	else
		for (int i = 0; i < iCount; i++)
			InsertFindFastElement(m_headers[i], i);
}

void CZipCentralDir::EnableFindFast(bool bEnable, bool bCaseSensitive)
{
	if (m_bFindFastEnabled == bEnable)
		return;
	m_bFindFastEnabled = bEnable;
	if (bEnable)
		BuildFindFastArray(bCaseSensitive);
	else
		m_findarray.RemoveAll();
}

int CZipCentralDir::FindFile(LPCTSTR lpszFileName, bool bCaseSensitive, bool bSporadically, bool bFileNameOnly)
{
	// this is required for fast finding and is done only once
	if (!m_bConvertAfterOpen)
	{
		TRACE(_T("%s(%i) : Converting all the filenames.\n"),__FILE__,__LINE__);
		ConvertAll();
	}
	if (!m_bFindFastEnabled)
		EnableFindFast(true, bSporadically ? !bCaseSensitive : bCaseSensitive);
	int iResult = -1;
	if (bFileNameOnly)
	{
		//  a non-effective search (treat an array as unsorted)
		int iSize = m_findarray.GetSize();
		for (int i = 0; i < iSize; i++)
		{
			CZipString sz = GetProperHeaderFileName(m_findarray[i].m_pHeader);
			CZipPathComponent::RemoveSeparators(sz); // to find a dir
			CZipPathComponent zpc(sz);
			sz = zpc.GetFileName();
			if ((sz.*m_pCompare)(lpszFileName) == 0)
			{
				iResult = i;
				break;
			}
		}
	}
	else if (bCaseSensitive == m_bCaseSensitive)
		iResult = FindFileNameIndex(lpszFileName);
	else
	{
		if (bSporadically)
		{
			//  a non-effective search (treat an array as unsorted)
			int iSize = m_findarray.GetSize();
			for (int i = 0; i < iSize; i++)
				if (CompareElement(lpszFileName, (WORD)i) == 0)
				{
					iResult = i;
					break;
				}
		}
		else
		{
			BuildFindFastArray(bCaseSensitive);		
			iResult = FindFileNameIndex(lpszFileName);
		}
	}
	return iResult == -1 ? -1 : m_findarray[iResult].m_uIndex;	
}

void CZipCentralDir::InsertFindFastElement(CZipFileHeader* pHeader, WORD uIndex)
{
	CZipString fileName = pHeader->GetFileName();
	int iSize = m_findarray.GetSize();

	//	Our initial binary search range encompasses the entire array of filenames:
	int start = 0;
	int end = iSize;

	//	Keep halving our search range until we find the right place
	//	to insert the new element:
	while ( start < end )
	{
		//	Find the midpoint of the search range:
		int midpoint = ( start + end ) / 2;

		//	Compare the filename with the filename at the midpoint of the current search range:
		int result = CompareElement(fileName, (WORD)midpoint);

		//	If our filename is larger, it must fall in the first half of the search range:
		if ( result > 0 )
		{
			end = midpoint;
		}

		//	If it's smaller, it must fall in the last half:
		else if ( result < 0 )
		{
			start = midpoint + 1;
		}

		//	If they're equal, we can go ahead and insert here:
		else
		{
			start = midpoint; break;
		}
	}
	m_findarray.InsertAt(start, CZipFindFast(pHeader, WORD(uIndex == WORD(-1) ? iSize : uIndex /* just in case */))); 
}

int CZipCentralDir::FindFileNameIndex(LPCTSTR lpszFileName) const
{
	int start = 0;
	int end = m_findarray.GetUpperBound();

	//	Keep halving our search range until we find the given element:
	while ( start <= end )
	{
		//	Find the midpoint of the search range:
		int midpoint = ( start + end ) / 2;

		//	Compare the given filename with the filename at the midpoint of the search range:
		int result = CompareElement(lpszFileName, (WORD)midpoint);

		//	If our filename is smaller, it must fall in the first half of the search range:
		if ( result > 0 )
		{
			end = midpoint - 1;
		}

		//	If it's larger, it must fall in the last half:
		else if ( result < 0 )
		{
			start = midpoint + 1;
		}

		//	If they're equal, return the result:
		else
		{
			return midpoint;
		}
	}

	//	Signal failure:
	return -1;
}

void CZipCentralDir::RenameFile(WORD uIndex, LPCTSTR lpszNewName)
{
	CZipFileHeader* pHeader = m_headers[uIndex];
	pHeader->SetFileName(lpszNewName);
	if (!m_bConvertAfterOpen)
		ZipCompatibility::FileNameUpdate(*pHeader, false);
	if (m_bFindFastEnabled)
		BuildFindFastArray(m_bCaseSensitive);
}