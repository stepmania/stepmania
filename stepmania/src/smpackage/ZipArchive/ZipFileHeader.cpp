////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFileHeader.cpp $
// $Archive: /ZipArchive_STL/ZipFileHeader.cpp $
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
#include "ZipFileHeader.h"
#include "ZipAutoBuffer.h"
#include "ZipArchive.h"
#include "ZipPlatform.h"
#include "ZipCompatibility.h"
#include <time.h>

#define FILEHEADERSIZE	46
#define LOCALFILEHEADERSIZE	30

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char CZipFileHeader::m_gszSignature[] = {0x50, 0x4b, 0x01, 0x02};
char CZipFileHeader::m_gszLocalSignature[] = {0x50, 0x4b, 0x03, 0x04};
CZipFileHeader::CZipFileHeader()
{
	m_uExternalAttr = 0;//ZipPlatform::GetDefaultAttributes();
	m_uModDate = m_uModTime = 0;
	m_uMethod = Z_DEFLATED;
// 	SetSystemCompatibility(ZipPlatform::m_sSystemID);
}

CZipFileHeader::~CZipFileHeader()
{

}

// read the header from the central dir
bool CZipFileHeader::Read(CZipStorage *pStorage)
{
// 	// just in case
// 	m_pszComment.Release();
// 	m_pszFileName.Release();
	WORD uFileNameSize, uCommentSize, uExtraFieldSize;
	CZipAutoBuffer buf(FILEHEADERSIZE);
	pStorage->Read(buf, FILEHEADERSIZE, true);		
	memcpy(&m_szSignature,		buf, 4);
	memcpy(&m_uVersionMadeBy,	buf + 4, 2);
	memcpy(&m_uVersionNeeded,	buf + 6, 2);
	memcpy(&m_uFlag,			buf + 8, 2);
	memcpy(&m_uMethod,			buf + 10, 2);
	memcpy(&m_uModTime,			buf + 12, 2);
	memcpy(&m_uModDate,			buf + 14, 2);
	memcpy(&m_uCrc32,			buf + 16, 4);
	memcpy(&m_uComprSize,		buf + 20, 4);
	memcpy(&m_uUncomprSize,		buf + 24, 4);
	memcpy(&uFileNameSize,		buf + 28, 2);
	memcpy(&uExtraFieldSize,	buf + 30, 2);
	memcpy(&uCommentSize,		buf + 32, 2);
	memcpy(&m_uDiskStart,		buf + 34, 2);
	memcpy(&m_uInternalAttr,	buf + 36, 2);
	memcpy(&m_uExternalAttr,	buf + 38, 4);
	memcpy(&m_uOffset,			buf + 42, 4);
	buf.Release();

	if (memcmp(m_szSignature, m_gszSignature, 4) != 0)
		return false;

	int iCurDsk = pStorage->GetCurrentDisk();
	m_pszFileName.Allocate(uFileNameSize); // don't add NULL at the end
	pStorage->Read(m_pszFileName, uFileNameSize, true);
	if (uExtraFieldSize)
	{
		ASSERT(!m_pExtraField.IsAllocated());
		m_pExtraField.Allocate(uExtraFieldSize);
		pStorage->Read(m_pExtraField, uExtraFieldSize, true);
	}
	if (uCommentSize)
	{
		m_pszComment.Allocate(uCommentSize);
		pStorage->Read(m_pszComment, uCommentSize, true);
	}
	
	return pStorage->GetCurrentDisk() == iCurDsk; // check that the whole header is on the one disk
}

time_t CZipFileHeader::GetTime()const
{
	struct tm atm;
	atm.tm_sec = (m_uModTime & ~0xFFE0) << 1;
	atm.tm_min = (m_uModTime & ~0xF800) >> 5;
	atm.tm_hour = m_uModTime >> 11;

	atm.tm_mday = m_uModDate & ~0xFFE0;
	atm.tm_mon = ((m_uModDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (m_uModDate >> 9) + 80;
	atm.tm_isdst = -1;
	return mktime(&atm);
}

// write the header to the central dir
DWORD CZipFileHeader::Write(CZipStorage *pStorage)
{
	WORD uFileNameSize = GetFileNameSize(), uCommentSize = GetCommentSize(),
		uExtraFieldSize = GetExtraFieldSize();
	DWORD iSize = FILEHEADERSIZE + uFileNameSize + uCommentSize + uExtraFieldSize;
	CZipAutoBuffer buf(iSize);
	memcpy(buf, &m_szSignature, 4);
	memcpy(buf + 4, &m_uVersionMadeBy, 2);
	memcpy(buf + 6, &m_uVersionNeeded, 2);
	memcpy(buf + 8, &m_uFlag, 2);
	memcpy(buf + 10, &m_uMethod, 2);
	memcpy(buf + 12, &m_uModTime, 2);
	memcpy(buf + 14, &m_uModDate, 2);
	memcpy(buf + 16, &m_uCrc32, 4);
	memcpy(buf + 20, &m_uComprSize, 4);
	memcpy(buf + 24, &m_uUncomprSize, 4);
	memcpy(buf + 28, &uFileNameSize, 2);
	memcpy(buf + 30, &uExtraFieldSize, 2);
	memcpy(buf + 32, &uCommentSize, 2);
	memcpy(buf + 34, &m_uDiskStart, 2);
	memcpy(buf + 36, &m_uInternalAttr, 2);
	memcpy(buf + 38, &m_uExternalAttr, 4);
	memcpy(buf + 42, &m_uOffset, 4);

	memcpy(buf + 46, m_pszFileName, uFileNameSize);

	if (uExtraFieldSize)
		memcpy(buf + 46 + uFileNameSize, m_pExtraField, uExtraFieldSize);
	
	if (uCommentSize)
		memcpy(buf + 46 + uFileNameSize + uExtraFieldSize, m_pszComment, uCommentSize);
	
	pStorage->Write(buf, iSize, true);
	return iSize;
}

bool CZipFileHeader::ReadLocal(CZipStorage *pStorage, WORD& iLocExtrFieldSize)
{
	char buf[LOCALFILEHEADERSIZE];
	pStorage->Read(buf, LOCALFILEHEADERSIZE, true);
	if (memcmp(buf, m_gszLocalSignature, 4) != 0)
		return false;
	
	bool bIsDataDescr = (((WORD)*(buf + 6)) & 8) != 0;
	
	WORD uFileNameSize = GetFileNameSize();
	WORD uTemp; 
	memcpy(&uTemp, buf+6, 2); // give the priority to the local flag
	if ((uTemp & 0xf) != (m_uFlag & 0xf))
		m_uFlag = uTemp;
	if ((memcmp(buf + 8, &m_uMethod, 2) != 0)
		|| (m_uMethod && (m_uMethod != Z_DEFLATED))
		|| (memcmp(buf + 26, &uFileNameSize, 2) != 0))
		return false;

// jeszcze mo¿naby porównaæ nazwy plików

	if (!bIsDataDescr/* || !pStorage->IsSpanMode()*/)
		if (!CheckCrcAndSizes(buf + 14))
			return false;

	memcpy(&iLocExtrFieldSize, buf + 28, 2);
	pStorage->m_pFile->Seek(uFileNameSize, CZipAbstractFile::current);

	return true;
}

void CZipFileHeader::SetTime(const time_t & ttime)
{
	tm* gt = localtime(&ttime);
    WORD year = (WORD)(gt->tm_year + 1900);
    if (year <= 1980)
		year = 0;
	else
		year -= 1980;
    m_uModDate = (WORD) (gt->tm_mday + ((gt->tm_mon + 1)<< 5) + (year << 9));
    m_uModTime = (WORD) ((gt->tm_sec >> 1) + (gt->tm_min << 5) + 
		(gt->tm_hour << 11));
}
//	the buffer contains crc32, compressed and uncompressed sizes to be compared 
//	with the actual values
bool CZipFileHeader::CheckCrcAndSizes(char *pBuf) const
{
	return (memcmp(pBuf, &m_uCrc32, 4) == 0) && (memcmp(pBuf + 4, &m_uComprSize, 4) == 0)
		&& (memcmp(pBuf + 8, &m_uUncomprSize, 4) == 0);
}

// write the local header
void CZipFileHeader::WriteLocal(CZipStorage& storage)
{
	// extra field is local by now
	WORD uFileNameSize = GetFileNameSize(),	uExtraFieldSize = GetExtraFieldSize();
	DWORD iLocalSize = LOCALFILEHEADERSIZE + uExtraFieldSize + uFileNameSize;
	CZipAutoBuffer buf(iLocalSize);
	memcpy(buf, m_gszLocalSignature, 4);
	memcpy(buf + 4, &m_uVersionNeeded, 2);
	memcpy(buf + 6, &m_uFlag, 2);
	memcpy(buf + 8, &m_uMethod, 2);
	memcpy(buf + 10, &m_uModTime, 2);
	memcpy(buf + 12, &m_uModDate, 2);
	memcpy(buf + 14, &m_uCrc32, 4);
	memcpy(buf + 18, &m_uComprSize, 4);
	memcpy(buf + 22, &m_uUncomprSize, 4);
	memcpy(buf + 26, &uFileNameSize, 2);
	memcpy(buf + 28, &uExtraFieldSize, 2);
	memcpy(buf + 30, m_pszFileName, uFileNameSize);
	memcpy(buf + 30 + uFileNameSize, m_pExtraField, uExtraFieldSize);

	// possible disk change before writing to the file in the disk spanning mode
	// so write the local header first 
	storage.Write(buf, iLocalSize, true);
	// it was only local information, use CZipArchive::SetExtraField to set the file extra field in the central directory
	m_pExtraField.Release();

	m_uDiskStart = (WORD)storage.GetCurrentDisk();
	m_uOffset = storage.GetPosition() - iLocalSize;
}

// prepare the data before adding a new file
bool CZipFileHeader::PrepareData(int iLevel, bool bSpan, bool bEncrypted)
{
	memcpy(m_szSignature, m_gszSignature, 4);
	m_uInternalAttr = 0;
	m_uVersionNeeded = IsDirectory() ? 0xa : 0x14; // 1.0 or 2.0
	SetVersion((WORD)(0x14)); 

	m_uCrc32 = 0;
	m_uComprSize = 0;
	m_uUncomprSize = 0;
	if (iLevel == 0)
		m_uMethod = 0;

	if ((m_uMethod != Z_DEFLATED) && (m_uMethod != 0))
		m_uMethod = Z_DEFLATED;

	m_uFlag  = 0;
	if (m_uMethod == Z_DEFLATED)
		switch (iLevel)
		{
		case 1:
			m_uFlag  |= 6;
			break;
		case 2:
			m_uFlag  |= 4;
			break;
		case 8:
		case 9:
			m_uFlag  |= 2;
			break;
		}

	if (bSpan || bEncrypted)
		m_uFlag  |= 8; // data descriptor present

	if (bEncrypted)
	{
		m_uComprSize = ZIPARCHIVE_ENCR_HEADER_LEN;	// encrypted header
		m_uFlag  |= 1;		// encrypted file
	}

	return !(m_pszComment.GetSize() > USHRT_MAX || m_pszFileName.GetSize() > USHRT_MAX
		|| m_pExtraField.GetSize() > USHRT_MAX);
}

void CZipFileHeader::GetCrcAndSizes(char * pBuffer)const
{
	memcpy(pBuffer, &m_uCrc32, 4);
	memcpy(pBuffer + 4, &m_uComprSize, 4);
	memcpy(pBuffer + 8, &m_uUncomprSize, 4);
}

DWORD CZipFileHeader::GetSize(bool bLocal)const
{
	if (bLocal)
		return LOCALFILEHEADERSIZE + GetExtraFieldSize() + GetFileNameSize();
	else
		return FILEHEADERSIZE + GetExtraFieldSize() + GetFileNameSize() + GetCommentSize();
}


bool CZipFileHeader::SetComment(LPCTSTR lpszComment)
{
	return CZipArchive::WideToSingle(lpszComment, m_pszComment)	!= -1;
}

CZipString CZipFileHeader::GetComment() const
{
	CZipString temp;
	CZipArchive::SingleToWide(m_pszComment, temp);
	return temp;

}

bool CZipFileHeader::SetFileName(LPCTSTR lpszFileName)
{

	return CZipArchive::WideToSingle(lpszFileName, m_pszFileName) != -1;
}

CZipString CZipFileHeader::GetFileName()const
{
	CZipString temp;
	CZipArchive::SingleToWide(m_pszFileName, temp);
	return temp;
}

bool CZipFileHeader::IsDirectory()const
{
	return ZipPlatform::IsDirectory(GetSystemAttr());
}

DWORD CZipFileHeader::GetSystemAttr()const
{
	int iSystemComp = GetSystemCompatibility();
	if (ZipCompatibility::IsPlatformSupported(iSystemComp))
	{
		if (!m_uExternalAttr && CZipPathComponent::HasEndingSeparator(GetFileName()))
			return ZipPlatform::GetDefaultDirAttributes(); // can happen
		else
			return ZipCompatibility::ConvertToSystem(m_uExternalAttr, iSystemComp, ZipPlatform::GetSystemID());
	}
	else
		return CZipPathComponent::HasEndingSeparator(GetFileName()) ? ZipPlatform::GetDefaultDirAttributes() : ZipPlatform::GetDefaultAttributes();
}


void CZipFileHeader::SetSystemAttr(DWORD uAttr)
{
	m_uExternalAttr = ZipCompatibility::ConvertToSystem(uAttr, ZipPlatform::GetSystemID(), GetSystemCompatibility());
}
