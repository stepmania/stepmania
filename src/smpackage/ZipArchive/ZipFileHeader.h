////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFileHeader.h $
// $Archive: /ZipArchive/ZipFileHeader.h $
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

/**
* \file ZipFileHeader.h
* Interface for the CZipFileHeader class.
*
*/

#if !defined(AFX_FILEHEADER_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_)
#define AFX_FILEHEADER_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZipExport.h"
#include "ZipStorage.h"
#include "ZipAutoBuffer.h"
#include "sys/types.h"
#include "ZipCompatibility.h"
#define ZIPARCHIVE_ENCR_HEADER_LEN 12

/**
	Representation of a single file stored in the zip archive.
	Modify all the class attributes BEFORE adding a file to and archive
	( using CZipArchive::OpenNewFile ).
	It is not possible to modify the data of the existing files in the archive
	(it would mean updating all the information in the local headers 
	and the offsets would be changed as well when the filename would changed its size)
*/
class ZIP_API CZipFileHeader  
{
	friend class CZipCentralDir;
	friend class CZipArchive;
	friend void ZipCompatibility::FileNameUpdate(CZipFileHeader&, bool);
public:	
	CZipFileHeader();
	virtual ~CZipFileHeader();


// 
/**
	Change slash to backslash or vice-versa	in #m_pszFileName.
	\param	bWindowsStyle
		if \c true, change slash to backslash; otherwise vice versa;
*/

/**
	\return	the filename size in characters (without NULL)
*/
	WORD GetFileNameSize()const {return (WORD)m_pszFileName.GetSize();}

/**
	\return	the comment size in characters (without NULL)
*/
	WORD GetCommentSize() const {return (WORD)m_pszComment.GetSize();}

/**
	\return	the extra field size in characters
*/
	WORD GetExtraFieldSize()const{return (WORD)m_pExtraField.GetSize();}

/**
	\return	the filename
*/
	CZipString GetFileName()const;

/**
	Set the filename
	\param	lpszFileName
		
	\return	
		\c true, if conversion from UNICODE to single byte was successful 
		(or if there was no conversion needed or possible); otherwise \c false;
*/
	bool SetFileName(LPCTSTR lpszFileName);

/**
	\return	the file comment
*/
	CZipString GetComment() const;

/**
	Set the file comment.
	\param	lpszComment
		
	\return	
		\c true, if conversion from UNICODE to single byte was successful 
		(or if there was no conversion needed or possible); otherwise \c false;

*/
	bool SetComment(LPCTSTR lpszComment);

/**
	\return	 \c true if the data descriptor is present	
*/
	bool IsDataDescr()const {	return (m_uFlag & (WORD) 8) != 0;}

	
	/**
		\return 
			get the effective compressed size: if the file is encrypted, return less by the size of the encryption header
	*/
	DWORD GetEffComprSize()
	{
		return m_uComprSize - (IsEncrypted() ? ZIPARCHIVE_ENCR_HEADER_LEN : 0);
	}
	/**
		\return 
			\c false if the file should be stored instead of being compressed
	*/
	bool CompressionEfficient()
	{
		DWORD uBefore = m_uUncomprSize;
		// ignore the length of encryption header 
		DWORD uAfter = GetEffComprSize();
		return  uAfter <= uBefore;
	}

	/**
		\return the compression ratio of the file
	*/
	float GetCompressionRatio()
	{
		return m_uUncomprSize ? ((float)m_uComprSize) * 100 / m_uUncomprSize : 0;
	}

/**
	\return	\c if the file is encrypted ( a password is needed to extract this file)
	\see CZipArchive::SetPassword
*/
	bool IsEncrypted()const {	return (m_uFlag & (WORD) 1) != 0;}

	char m_szSignature[4];			///< central file header signature
	WORD m_uVersionMadeBy;			///< version made by and system compatibility
	WORD m_uVersionNeeded;			///< version needed to extract
	WORD m_uFlag;					///< general purpose bit flag  
	WORD m_uMethod;					///< compression method 
	WORD m_uModTime;				///< last mod file time
	WORD m_uModDate;				///< last mod file date 
	DWORD m_uCrc32;					///< crc-32    
	DWORD m_uComprSize;				///< compressed size 
	DWORD m_uUncomprSize;			///< uncompressed size     
//         filename length                 2 bytes
// 	WORD m_uFileNameSize;
//         extra field length              2 bytes
// 	WORD m_uExtraFieldSize;
//         file comment length             2 bytes
// 	WORD m_uCommentSize;
	WORD m_uDiskStart;				///< disk number start
	WORD m_uInternalAttr;			///< internal file attributes 
protected:
	DWORD m_uExternalAttr;			///< external file attributes 
public:
	DWORD m_uOffset;				///< relative offset of local header 
	CZipAutoBuffer m_pExtraField;	///< extra field (variable size)
	static char m_gszSignature[];		///< central file header signature
	static char m_gszLocalSignature[];	///< local file header signature
	

/**
	Set #m_uModDate, #m_uModTime
	(file modification time)
	\param ttime
	\see GetTime
*/
	void SetTime(const time_t& ttime);

/**
	\return	the modification time
	\see SetTime
*/
	time_t GetTime()const;

/**
	\param bLocal
		if \c true return the local file header size or in the central directory otherwise
	\return	the total size of the structure depending on the \e bLocal parameter
*/
	DWORD GetSize(bool bLocal = false)const;

/**
	\return	the system compatibility of the current file as
	the one of ZipCompatibility::ZipPlatforms values;
	usually the same as CZipArchive::GetSystemComatibility.
	Software can use this information e.g. to determine the line 
	record format for text files etc.
	ZipArchive library uses it to perform a proper attributes conversion.

	\see CZipArchive::GetSystemComatibility
	\see ZipPlatform::GetSystemID
*/
	int GetSystemCompatibility()const
	{
		return (m_uVersionMadeBy & 0xFF00) >> 8;
	}

/**
	\return	the attributes of the file converted to the current system compatible value
	\note 
		- Throws exception if the archive system or the current system 
	is not supported by the ZipArchive library.
		- (only Linux version): After obtaining the attributes, you need to shift them right by 16
*/

	DWORD GetSystemAttr()const;

	/**
		\return 
		the original attributes as stored in the archive (without performing any conversion)
	*/
	DWORD GetOriginalAttributes() const {return m_uExternalAttr;}

/**
	\return	\c true, if this object represents a directory; otherwise \c false;
	If #m_uExternalAttr value is zero, the function check for a presence of a path 
	separator at the end of the file name. If it is present, it is assumed to be a directory.
*/
	bool IsDirectory()const;


protected:

/**
	Set the system compatibility of the file.
	\param	iSystemID
		one of ZipCompatibility::ZipPlatforms values 
	\see CZipArchive::GetSystemCompatibility
*/
	void SetSystemCompatibility(int iSystemID)
	{
		m_uVersionMadeBy &= 0x00FF;
		m_uVersionMadeBy |= (WORD)(iSystemID << 8);
	}

/**
	Set the system attributes
	if you wish to set the attributes of this structure use CZipArchive::SetFileHeaderAttr()
	\param	uAttr
		attributes to set		
	\note Throws exceptions if the archive system or the current system 
		is not supported by the ZipArchive library.
	\see CZipArchive::SetFileHeaderAttr
	\see GetSystemAttr
	\see SetSystemCompatibility
*/
	void SetSystemAttr(DWORD uAttr);

/**
	Set the version made by number.

*/
	void SetVersion(WORD uVersion)
	{
		if ((m_uVersionMadeBy & 0x00FF) != (uVersion & 0x00FF)) 
		{
			m_uVersionMadeBy &= 0xFF00;
			m_uVersionMadeBy |= (WORD)(uVersion & 0x00FF);
		}
	}

	/**
		a filename
	*/
	CZipAutoBuffer m_pszFileName;	

	/**
		a file comment	
	*/
	CZipAutoBuffer m_pszComment;

/**
	Fill the buffer with the current values of crc and compressed and 
	uncompressed sizes of the file.

		
*/
	void GetCrcAndSizes(char* pBuffer)const;

/**
	Check whether the actual values of crc and compressed and 
	uncompressed sizes of the file are the same as defined in 
	the file header.
	\param	pBuf
		buffer with the mentioned data
	\return	\c true if they are the same; otherwise \c false;
*/
	bool CheckCrcAndSizes(char* pBuf) const;


/**
	Prepare the data for the class while adding a new file.
	Called by CZipArchive::OpenNewFile
	\param	iLevel
		a compression level
	\param	bSpan
		\c true, if the data descriptor will be present because of spanning archive
	\param	bEncrypted
		\c true, if the file will be encrypted
	\return	\c true if the sizes of the filename, extra field and comments
	does not exceed \c unsigned \c short maximum value; otherwise \c false;
*/
	bool PrepareData(int iLevel, bool bSpan, bool bEncrypted);
/**	
	Write the local file header to the \e storage
	\param	storage
	\note Throws exceptions.
*/
	void WriteLocal(CZipStorage& storage);

/**
	Read the file header from the central directory record from \e pStorage.
	\param	*pStorage
	\return	\c true if the whole file header is on one disk in 
	a multi-disk archive; otherwise \c false;
	\note Throws exceptions.
*/
	bool Read(CZipStorage *pStorage);
/**
	Read the local file header from \e pStorage and check for consistency.
	\param	*pStorage
	\param	iLocExtrFieldSize
		receives local extra field size
	\return	\c false, if something goes wrong; otherwise \c true;
	\note Throws exceptions.
*/
	bool ReadLocal(CZipStorage *pStorage, WORD& iLocExtrFieldSize);
/**
	Write the file header to \e pStorage.
	\param	*pStorage
	\return	the size of the file header
	\note Throws exceptions.
*/
	DWORD Write(CZipStorage *pStorage);

};

#endif // !defined(AFX_FILEHEADER_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_)
