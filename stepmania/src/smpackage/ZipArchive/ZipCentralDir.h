////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipCentralDir.h $
// $Archive: /ZipArchive/ZipCentralDir.h $
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
* \file ZipCentralDir.h
*	Interface for the CZipCentralDir class.
*
*/

#if !defined(AFX_CENTRALDIR_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_)
#define AFX_CENTRALDIR_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

#include "ZipException.h"
#include "ZipFileHeader.h"
#include "ZipAutoBuffer.h"
#include "ZipCollections.h"
#include "ZipCompatibility.h"
#include "ZipExport.h"
#define ZIPARCHIVE_DATADESCRIPTOR_LEN 12

/**
	A class representing the central directory record in the archive.
*/
class ZIP_API CZipCentralDir  
{
public:
	/**
		Used in fast finding files by the filename.
		A structure for the internal use only.
		\see CZipCentralDir::m_findarray
		\see CZipArchive::GetFindFastElement
		\see CZipArchive::FindFile
		\see CZipArchive::EnableFindFast
	*/
	struct ZIP_API CZipFindFast
	{
		CZipFindFast()
		{
			m_uIndex = 0;
			m_pHeader= NULL;
		}
		CZipFindFast(CZipFileHeader* pHeader, WORD uIndex):m_pHeader(pHeader), m_uIndex(uIndex){}
		/**
			A pointer to the structure in CZipCentralDir. We extract a name from it.
		*/
		CZipFileHeader* m_pHeader;

		/**
			The index in the central directory of the \e m_pHeader.
		*/
		WORD m_uIndex;
	};
	

	/**
		Stores general information about the central directory record.
	*/
	struct ZIP_API Info
	{
		DWORD m_uCentrDirPos;	///< the position of the beginning of the central directory (as located by #Locate)
		DWORD m_uBytesBeforeZip;///< The number of bytes before the actual zip archive in a file. It is non-zero for self-extracting archives.
		WORD m_uThisDisk;		///< number of the disk with the central directory end record (the number of disks in the disk-spanning archive)
		WORD m_uDiskWithCD;		///< number of the disk with the start of the central directory
		WORD m_uDiskEntriesNo;	///< total number of entries in the central dir on this disk
		WORD m_uEntriesNumber;	///< total number of entries in the central dir
		DWORD m_uSize;			///< size of the central directory (valid only if #m_bOnDisk is \c true; use #GetSize instead)
		DWORD m_uOffset;		///< offset of start of central directory with respect to the starting disk number 
								///< (as written in the central directory record);
								///< valid only if #m_bOnDisk is \c true
		bool m_bOnDisk;			///< \c true if the central directory is physically present in the archive

	protected:
		friend CZipCentralDir;
		bool CheckIfOK_1()
		{
			return ((DWORD)m_uCentrDirPos >= m_uOffset + m_uSize);
		}
		void SetBytesBeforeZip(bool bIsSpan)
		{
			m_uBytesBeforeZip = bIsSpan ? 0 : m_uCentrDirPos - m_uSize - m_uOffset;
		}
		bool CheckIfOK_2()
		{
			return (m_uSize || !m_uEntriesNumber) && (m_uEntriesNumber || !m_uSize);
		}
		void DiskChange(int iCurrentDisk)
		{
			m_uThisDisk = (WORD)iCurrentDisk;
			if (m_uEntriesNumber)
			{
				m_uDiskEntriesNo = 0;	
			}
			else
			{
				m_uDiskWithCD = m_uThisDisk;
				m_uOffset = 0;
			}
		}
	};

	CZipCentralDir();
	virtual ~CZipCentralDir();

	static char m_gszSignature[]; ///< central dir signature

	char m_szSignature[4];	///< end of central dir signature (must be 0x06054b50)

	CZipAutoBuffer m_pszComment;	///< the archive comment
	CZipAutoBuffer m_pLocalExtraField; ///< a local extra field
	CZipFileHeader* m_pOpenedFile;	///< points to a currently opened file or NULL if no file is opened

	/**
		Called by CZipArchive::OpenInternal.
	*/
	void Init();

	/**
		Read the central directory from the archive.
		\note Throws exceptions.
	*/
	void Read();

	/**
		Open the file.
		\param uIndex
			zero-based index of the file to open
		\note Throws exceptions.
	*/
	void OpenFile(WORD uIndex);

/**	
	Test if the given file header index is valid.
	\param	uIndex
		a zero-based index 
	\return	\c true if the file with the given index exists inside the archive; otherwise \c false;
*/
	bool IsValidIndex(int uIndex)const;

/**
	Remove the file header from the central directory.
	\param	pHeader
		the header to remove
	\param iIndex if index is not known set it to -1
	\param bShift 
	\note Throws exceptions.
*/
	void RemoveFile(CZipFileHeader* pHeader, int iIndex = -1, bool bShift = true);


    /**
       Remove last file from the central directory.
       
     */
	void RemoveLastFile(CZipFileHeader* pHeader = NULL, int iIndex = -1)
	{
		if (iIndex == -1)
		{
			iIndex = m_headers.GetSize() - 1;
			if (iIndex == -1)
				return;
		}
		if (!pHeader)
			pHeader = m_headers[iIndex];
		DWORD uNewSize = pHeader->m_uOffset + GetBytesBefore();
		// then remove
		RemoveFile(pHeader, iIndex);

		m_pStorage->Flush();
		m_pStorage->m_pFile->SetLength(uNewSize);
		m_info.m_bOnDisk = false; // it is true when AutoFlush is set to true
	}


	/**
		Remove all files
		\note Throws exceptions.
	*/
	void RemoveAll();
/**
	Cleanup the structure.
	\param	bEverything
		- \c true - clear some attributes and remove all the file headers from memory
		- \c false - do not remove the file headers. It is called in that manner
		from CZipArchive::CloseFileAfterTestFailed so that the 
		next file can be tested for the integrity
	\see CZipArchive::CloseFileAfterTestFailed
*/
	void Clear(bool bEverything = true);

/**
	Add a new file to the central directory.
	\param	header
		copy data from it to the new file header	
	\param iReplaceIndex if different from -1, the index of the file to be replaced
	\return the pointer to the new header
	\note Throws exceptions.
*/
	CZipFileHeader* AddNewFile(const CZipFileHeader & header, int iReplaceIndex = -1);

	/**
		return the header filename, converted if needed
	*/
	CZipString GetProperHeaderFileName(const CZipFileHeader* pHeader) const
	{
		if (!m_bConvertAfterOpen)
		{
			CZipFileHeader fh = *pHeader;
			ConvertFileName(true, false, &fh);
			return fh.GetFileName();
		}
		else
			return pHeader->GetFileName();
	}

/**
	Remove physically the central directory from the archive.
	Called during adding or deleting files.
	\note Throws exceptions.
*/
	void RemoveFromDisk();

/**
	Get the central directory size.
	\param	bWhole
		if \c true, include the size of the file headers
	\return	the size of the central directory
	\see CZipArchive::GetCentralDirSize
*/
	DWORD GetSize(bool bWhole = false) const;

	/**
		Close a file inside archive opened for reading.
		\param bAfterException \c true if closing after exception
		\note Throws exceptions.
	*/
	void CloseFile(bool bAfterException = false);

	/**
		Close a file inside archive opened for reading.
		\note Throws exceptions.
	*/
	void CloseNewFile();

	/**
		Write the central directory to the archive.
		\note Throws exceptions.
	*/
	void Write(CZipActionCallback* pCallback);
	
	/**
		\see CZipArchive::EnableFindFast
	*/
	void EnableFindFast(bool bEnable, bool bCaseSensitive);

	/**
		\see CZipArchive::FindFile
		\note \e bSporadically set to \c false rebuilds #m_findarray if necessary
	*/
	int FindFile(LPCTSTR lpszFileName, bool bCaseSensitive, bool bSporadically, bool bFileNameOnly);


	/**
		\see CZipArchive::GetFindFastIndex
	*/
	int GetFindFastIndex(int iFindFastIndex)const
	{
		if (!IsValidIndex(iFindFastIndex) || !m_bFindFastEnabled)
		{
	// 		ASSERT(FALSE); // 
			return -1;
		}
		
		return m_findarray[iFindFastIndex].m_uIndex;
	}

	/**
		Points to CZipArchive::m_storage.
	*/
	CZipStorage* m_pStorage;
	
	
	/**
		The size of the buffer used in searching for the central dir.
		Set before opening the archive.
		It is usually set with CZipArchive::SetAdvanced
		(specify this value as the third argument).
		\see CZipArchive::SetAdvanced
	*/
	int m_iBufferSize;
	



	/**
		Holds all the files inside archive info.
		\see CZipFileHeader
	*/
	CZipArray<CZipFileHeader*> m_headers;
	
	CZipFileHeader* operator[](int iIndex)
	{
		return m_headers[iIndex];
	}
	const CZipFileHeader* operator[](int iIndex) const
	{
		return m_headers[iIndex];
	}

	
	/**
		- If \c true, the conversion of the filenames takes 
		place after opening the archive (after reading the central directory 
		from the file), and before writing the central directory back to
		the archive.
		- If \c false, the conversion takes place on each call to CZipArchive::GetFileInfo

		Change is value with CZipArchive::SetConvertAfterOpen.

		Set it to \c true when you plan to use CZipArchive::FindFile or get the stored files information. <BR>
		Set it to \c false when you plan mostly to modify the archive.

		\b Default: \c true
		\note Set it before opening the archive.
		\see CZipArchive::SetConvertAfterOpen
		\see ConvertFileName
	*/
	bool m_bConvertAfterOpen;
	

/**
	Convert the filename of the CZipFileHeader depending on the current system
	and the system the zip file was created on (change slash to backslash or
	vice versa, perform ANSI-OEM conversion if necessary).
	\param	bFromZip
		if \c true, convert from archive format
	\param	bAfterOpen
		if \c true, called after opening the archive or before closing
	\param	pHeader		
		the header to have filename converted; if \c NULL convert the currently
		opened file
	\see ZipCompatibility::FileNameUpdate
	\see m_bConvertAfterOpen
*/
	void ConvertFileName(bool bFromZip, bool bAfterOpen, CZipFileHeader* pHeader = NULL) const
	{
		if (bAfterOpen != m_bConvertAfterOpen)
			return;
		if (!pHeader)
		{
			pHeader = m_pOpenedFile;
			ASSERT(pHeader);
		}
		ZipCompatibility::FileNameUpdate(*pHeader, bFromZip);
	}

/**
	Convert all the filenames to the system form.
	Called by CZipArchive::FindFile
	\see CZipArchive::FindFile
*/
	void ConvertAll();

/**
	\param	lpszFileName
		the name of the file to find, must be exactly the same (apart from case)
		as it appears in the archive
	\return	the index in #m_findarray with the appropriate CZipFindFast structure
	or \c -1 if there is no file with the given name
	\see CZipArchive::FindFile
*/
	int FindFileNameIndex(LPCTSTR lpszFileName) const;

	DWORD GetBytesBefore() const {return m_info.m_uBytesBeforeZip;}
	/**
		Get the information about the central directory
	*/
	void GetInfo(Info& info) const {info = m_info;}
	/**
		\return the value of m_bFindFastEnabled
	*/
	bool IsFindFastEnabled(){return m_bFindFastEnabled;}
	/**
		Called by CZipArchive::RenameFile
	*/
	void RenameFile(WORD uIndex, LPCTSTR lpszNewName);
protected:
	/**
		Sort the files inside the archive headers by the order in the archive.
	*/
	void SortHeaders();
	static int CompareHeaders(const void *pArg1, const void *pArg2)
	{
		CZipFileHeader* pw1 = *(CZipFileHeader**)pArg1;
		CZipFileHeader* pw2 = *(CZipFileHeader**)pArg2;
		if ((pw1->m_uOffset < pw2->m_uOffset && pw1->m_uDiskStart == pw2->m_uDiskStart)
			|| (pw1->m_uDiskStart < pw2->m_uDiskStart))
			return -1;
		else if ((pw1->m_uOffset > pw2->m_uOffset && pw1->m_uDiskStart == pw2->m_uDiskStart)
			|| (pw1->m_uDiskStart > pw2->m_uDiskStart))
			return 1;
		else
		{
			ASSERT(FALSE);
			// two files with the same offsets on the same disk???
			CZipException::Throw(CZipException::badZipFile);
			return 0; // just for the compiler comfort
		}
	}


/**
	Build #m_findarray.
*/
	void BuildFindFastArray( bool bCaseSensitive );

	/**
		Used in fast finding files by the filename.
		\see CZipFindFast
		\see m_bFindFastEnabled
		\see CZipArchive::FindFile
	*/
	CZipArray<CZipFindFast> m_findarray;

	/**
		If \c true, the there is an additional array build, to speed up the 
		finding process
		CZipArchive::FindFile uses this array to perform a 
		binary search.
		\b Default: \c false
		\see CZipArchive::EnableFindFast
		\see CZipArchive::FindFile
		\see CZipCentralDir::m_findarray
	*/
	bool m_bFindFastEnabled;



/**
	The \e lpszFileName and \e bCaseSensitive arguments 
	are the same as in the #FindFileNameIndex. The function get CZipFindFast
	structure pointed by \e uIndex and compares the filename of CZipFileHeader
	class stored in this structure with \e lpszFileName.
	\param	lpszFileName
	\param	uIndex
		the index from #m_findarray
	\return 
	- 0 if the filenames are the same
	- < 0 if the filename stored in the array is less than \e lpszFileName
	- > 0 if the filename stored in the array is greater than \e lpszFileName
*/
	int CompareElement(LPCTSTR lpszFileName, WORD uIndex) const
	{
		return (m_findarray[uIndex].m_pHeader->GetFileName().*m_pCompare)(lpszFileName);
	}
/**
	Insert a new CZipFindFast element to the #m_findarray.
	Initialize CZipFindFast object with \e pHeader and \e uIndex values.
*/
	void InsertFindFastElement(CZipFileHeader* pHeader, WORD uIndex);

	 
	
	/**
	A compare function (Collate or CollateNoCase) set once so as not
	to check every time which one to use<BR>
	ZIPSTRINGCOMPARE is defined in CZipString.h as: <BR>
	<B><CODE> typedef int (CZipString::*ZIPSTRINGCOMPARE)( LPCTSTR ) const; </CODE></B>
	*/
	ZIPSTRINGCOMPARE m_pCompare;

	/**
		The way the m_findarray is sorted
	*/
	bool m_bCaseSensitive;

	/**
		\see Info
	*/
	Info m_info;

	/**
		\return the location of the beginning of the central dir end record in the archive
		\note Throws exceptions.
	*/
	DWORD Locate();	
	/**
		Read the file headers from the file.
		\note Throws exceptions.
	*/
	void ReadHeaders();

	/**
		Free the memory allocated for the CZipFileHeader structures.
	*/
	void RemoveHeaders();
/**
	Remove data descriptors from the write buffer in the disk spanning volume
	that turned out to be one-disk only.
	We do not remove them from password encrypted files.

	\param	bFromBuffer
		if \c true, remove from the buffer in memory otherwise from the file on a disk
	\return	\c false if the file mapping to memory was not successful
	Can happen only when \e bFormBuffer is \c false.
	\note Throws exceptions.
*/
	bool RemoveDataDescr(bool bFromBuffer);
/**
	Write the file headers to the archive.
	\note Throws exceptions.
*/
	void WriteHeaders(CZipActionCallback* pCallback, bool bOneDisk);
/**
	Write the central directory end record.
	\return	the size of the record
	\note Throws exceptions.
*/
	DWORD WriteCentralEnd();

/**
	Throw an exception with the given code.
	\param	err
	\see CZipException::Throw
*/
	void ThrowError(int err) const;
	
	
};


#endif // !defined(AFX_CENTRALDIR_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_)
