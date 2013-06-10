////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipStorage.h $
// $Archive: /ZipArchive/ZipStorage.h $
// $Date$ $Author$.
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
* \file ZipStorage.h
* Interface for the CZipStorage class.	
*
*/

#if !defined(AFX_ZIPSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_)
#define AFX_ZIPSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZipFile.h"	
#include "ZipAutoBuffer.h"
#include "ZipString.h"
#include "ZipMemFile.h"
#include "ZipExport.h"



/**
	A base class for functional objects (functors) that are used as a callbacks during various actions.
	You need to derive your own class and overload \c Callback method to use it.
	Do not derive from CZipCallback directly but from CZipSpanCallback (as a callback when there is a need 
	for a disk change in a disk-spanning archive) or from CZipActionCallback for other actions.
*/
struct ZIP_API CZipCallback
{
	/**
		Method called as a callback. 
		Return \c false from inside the method to abort the current operation. If it is a span callback functor,
		a CZipException with CZipException::aborted code will be thrown, otherwise the code will be CZipException::abortedAction or CZipException::abortedSafely.
		The following actions can be safely aborted (without having the archive corrupted):
		- counting bytes before deleting files
		- testing
		- saving central directory on non-disk-spanning archive 
		(saved data is removed in case of break	and you can save it again);
		it the archive is disk-spanning and if saving is aborted, the archive
		will not be damaged, but saved part of the central directory will be not removed
		and the new central directory will have to be saved after it

		\note Overrride this method in the derived class. If you define this method inside the class declaration, it should be inlined
			by the compiler making the action progress faster.
	*/
	virtual bool Callback(int iProgress) = 0;


	CZipString m_szExternalFile;	///< if the action (adding, extracting or disk-spanning) uses an external file, its filename is stored here	
};

/**
	Derive from this a class to be used as a callback functor for the disk change callback.
	You need to override member function CZipCallback::Callback. The meaning of \e iProgress parameter is the reason for calling:
		- -1 : a disk needed for reading		<BR>
	other codes occurs during writing:
		- >=0 : min. number of bytes needed
		- -2 : the file with the archive name already exists on the disk
		- -3 : the disk is probably write - protected
		- -4 : couldn't create a file
	

	Return \c false from the callback function to abort operation: the proper exception will be thrown.

	\see CZipCallback::Callback
	\see CZipArchive::SetSpanCallback
*/
struct ZIP_API CZipSpanCallback : public CZipCallback
{
	DWORD m_uDiskNeeded;		///< the number of a disk needed (counting from 1)
};



/**
	Derive from this a class to be used as a callback functors when adding, extracting, deleting, testing a file
	or saving central directory.
	You need to override member function CZipCallback::Callback. The meaning of \e iProgress parameter is the count
	of data just processed. It is a smallest number of bytes after which the callback method is called and it depends
	on the value of \e nBufSize in the CZipArchive methods that uses the callback feature. In case of saving the central 
	directory action it is the count of file headers just written (see CZipArchive::cbSave)
	\see CZipCallback::Callback
	\see CZipArchive::SetCallback
*/
struct ZIP_API CZipActionCallback : public CZipCallback
{

	CZipActionCallback()
	{		
		m_uTotalToDo = 0;
		m_uTotalSoFar = 0;
	}
	
	/**
		The type of the callback. It is set to one of CZipArchive::CallbackType values when the action begins.
		It's useful if you have more than one callback assigned to the same functor.
	*/
	int m_iType;

	/**
		Used by the ZipArchive library to init the callback function with the filenames. Resets #m_uTotalToDo and #m_uTotalSoFar variables to 0.
		#m_iType variable is already set to the proper value. Called at the beginning of the action.
	*/
	virtual void Init(LPCTSTR lpszFileInZip = NULL, LPCTSTR lpszExternalFile = NULL) 
	{
		m_szFileInZip = lpszFileInZip;
		m_szExternalFile = lpszExternalFile;
		m_uTotalToDo = 0;  // not yet known
		m_uTotalSoFar = 0; // nothing yet done
	}

	/**
		Called by the ZipArchive functions that use the callback feature after calculating total data to process.
		\param uTotalToDo
			total data to process; set #m_uTotalToDo to this value
	*/
	virtual void SetTotal(DWORD uTotalToDo)
	{
		m_uTotalToDo = uTotalToDo;
// 		m_uTotalSoFar = 0; // already done in CZipArchive::CZipClbckStrg::Get
	}



	/**
		Total number of data to process. The value of this variable is set after calling by the library #SetTotal method (it is 0 before).
		Depending on the action it is set then to:
		- adding file: the size the external file being added (or if callback is CZipArchive::cbAddTmp, the size of compressed data: CZipFileHeader::m_uComprSize)
		- extracting file: the size of uncompressed data (CZipFileHeader::m_uUncomprSize)
		- testing file: the same as above
		- deleting file: the count of bytes to move - the size of all files to remain above the first file to delete (calculated from offsets CZipFileHeader::m_uOffset
		- saving central directory: the number of files in the archive

	*/
	DWORD m_uTotalToDo;				
	DWORD m_uTotalSoFar;			///< total number of bytes processed so far 
	CZipString m_szFileInZip;		///< file in zip archive being currently processed


	/**
		\return the number of bytes left to process 
	*/
	DWORD LeftToDo() {return m_uTotalToDo - m_uTotalSoFar;}


	/**
		Called after the action finishes (it is not called in case of an exception, but
		it is called before throwing CZipException::abortedAction or CZipException::abortedSafely)
	*/
	virtual void CallbackEnd()
	{
		ASSERT(m_uTotalSoFar == m_uTotalToDo);
	};

	/**
		Used internally to call Callback and increase #m_uTotalSoFar by \e iProgress
	*/
	virtual bool operator()(int iProgress)
	{
		m_uTotalSoFar += iProgress;
		return Callback(iProgress);
	}

	/**
		Used only when creating map before deletion (see CZipArchive::cbDeleteCnt) or 
		saving the central directory record. You'll be notified every nth step (n is \e m_iStep value) with \e iProgress set to 
		\e m_iStep . Do not set it to low values or you can have a long waiting on archives
		with huge number of files. 

		\b Default: 256.
	*/
	static int m_iStep;


	/**
		Used internally to return #m_iStep value but not 0 (return 1 in this case).
	*/
	int GetStep(){return m_iStep ? m_iStep : 1;} // do not allow 0 (crash)
		
};


/**
	A low-level class - operates physically on archive (CZipArchive operates logically)
*/
class ZIP_API CZipStorage  
{
	friend class CZipCentralDir;
public:
	
	/**
		The type of the disk spanning archive.
		\see CZipArchive::GetSpanMode
	*/
	enum ZipSpanMode {
		noSpan,			///< no disk spanning
		pkzipSpan,		///< \ref PKSpan "disk spanning compatible with PKZIP"
		tdSpan,			///< \ref TDSpan "TD mode disk spanning archive"
		/**
			Detect the type automatically.
			If the archive is on the removable device assume PKZIP compatible,
			otherwise TD mode compatible.
		*/
		suggestedAuto,	
		/**
			If the disk spanning archive is on the removable device 
			assume it is TD mode compatible
		*/
		suggestedTd
	};

	CZipStorage();
	virtual ~CZipStorage();

/**
	Open the archive in memory (new or existing).
	The parameters are the same as CZipArchive::OpenMode.
	\param	mf
	\param	iMode
	\note Throws exceptions.

	\see CZipArchive::Open(LPCTSTR, int, int)
*/
	void Open(CZipMemFile& mf, int iMode);

/**
	Open or create an archive.
	The parameters are the same as CZipArchive::Open.
	\param	szPathName
	\param	iMode
	\param	iVolumeSize
	\note Throws exceptions.
	
	\see CZipArchive::Open(CZipMemFile& , int);
*/
	void Open(LPCTSTR szPathName, int iMode, int iVolumeSize);


	/**
		Close the disk-spanning archive and reopens as an existing disk-spanning archive or set mode to #noSpan 
	*/
	void FinalizeSpan();

	
/**
	Called only by CZipCentralDir::Read() when opening an existing archive.
	\param	uLastDisk
		the disk number the central directory is on
	\note Throws exceptions.

*/
	void UpdateSpanMode(WORD uLastDisk);


/**
	Write chunk of data to the archive.
	\param	pBuf
		buffer with data
	\param	iSize
		bytes to write
	\param	bAtOnce
		if \c true, the whole data must fit in the current volume, 
		otherwise the disk change is performed
	\note Throws exceptions.
*/
	void Write(const void *pBuf, DWORD iSize, bool bAtOnce);

/**
	Read chunk of data from the archive.
	\param	pBuf
		buffer to receive data
	\param	iSize
		bytes to read
	\param	bAtOnce
		if \c true, the specified number of bytes must be read from the same 
		volume (no disk change allowed)
	\note Throws exceptions.
*/
	DWORD Read(void* pBuf, DWORD iSize, bool bAtOnce);

	/**
		Return the position in the file, taking into account the bytes in the write buffer.
		\note Throws exceptions.
	*/
	DWORD GetPosition() const {return (DWORD)(m_pFile->GetPosition()) + m_uBytesInWriteBuffer;}


	/**
		Flush the data from the read buffer to the disk.
		\note Throws exceptions.
	*/
	void Flush();


	/**
		Forces any data remaining in the file buffer to be written to the disk
	*/
	void FlushFile()
	{
		if (!m_bInMemory && !IsReadOnly())
			m_pFile->Flush();
	}

/**
	A method used to change disks during writing to the disk spanning archive.
	\param	iNeeded
		no of bytes needed on the disk
	\param	lpszFileName
		the archive filename
	\note Throws exceptions.
*/
	void NextDisk(int iNeeded, LPCTSTR lpszFileName = NULL);


/**
	\return a zero-based number of the current disk
*/
	int GetCurrentDisk() const {return m_iCurrentDisk;}

 
/**
	Change the disk during extract operations.
	\param	iNumber
		a zero-based disk number requested
	\return	
*/
	void ChangeDisk(int iNumber);

/**
	Detect the span mode.
	\return	
		- -1 - existing disk spanning archive opened
		- 0 - no disk spanning archive
		- 1 - disk spanning archive in creation

*/
	int IsSpanMode() const
	{
		return m_iSpanMode == noSpan ? 0 : (m_bNewSpan ? 1 : -1);
	}

	/**
		return \c true if the archive cannot be modified.
	*/
	bool IsReadOnly()
	{
		return m_bReadOnly || IsSpanMode() < 0;
	}

/**
	
	\param	bAfterException
		set to \c true after the library has throw an exception.
		The simplified mode is used then.
		In this case it'll be possible to reuse the object to operate on another
		archive, but the current archive file will not be valid anyway.
	\return the filepath of the archive (used by CZipArchive::Close)
	\note Throws exceptions.
*/
	CZipString Close(bool bAfterException);


	/**
		The size of the write buffer. 
		Set before opening the archive. 
		It is usually set with CZipArchive::SetAdvanced
		(specify this value as the first argument).
		\see CZipArchive::SetAdvanced
	*/
	int m_iWriteBufferSize;


	/**
		The physical archive file (on a storage device).
		Not used when opening archive in memory
		with Open(CZipMemFile& , int).
	*/
	CZipFile m_internalfile;

	/**
		The buffer representing the archive. 
		It is a physical file or a memory buffer depending on what
		method was used to open the archive. In the first case it is 
		a pointer to #m_internalfile.
		\see Open(LPCTSTR, int, int);
		\see Open(CZipMemFile& mf, int)
		
	*/
	CZipAbstractFile* m_pFile;	

	/**
		Takes one of the values of #ZipSpanMode.	
	*/
	int m_iSpanMode;

	/**
		A callback functor which method \c Callback is called when there is a need for a disk change 
		while operating on a #pkzipSpan archive.
		\see CZipArchive::SetSpanCallback
	*/
	CZipSpanCallback* m_pChangeDiskFunc;

	/**
		The signature of the extended header	
	*/
	static char m_gszExtHeaderSignat[];
protected:

	/**
		Flush without writing. Can be used only on non-disk spanning archives.
	*/
	void EmptyWriteBuffer()
	{
		m_uBytesInWriteBuffer = 0;
	}

/**
	Open a physical file.
	\param	lpszName
		the name of the file to open
	\param	uFlags
		file open flags
	\param	bThrow
		if \c true then throw an exception in case of failure
	\return	\c true if successful
*/
	bool OpenFile(LPCTSTR lpszName, UINT uFlags, bool bThrow = true);
/**
	Throw an exception with the given code.
	\param	err
	\see CZipException::Throw
*/
	void ThrowError(int err);

/**
	Return the number of bytes left on the current volume.		
*/
	DWORD VolumeLeft() const;

	/**
		Rename last file in TD mode disk spanning archive when done with creating
	*/
	CZipString RenameLastFileInTDSpan();
/**
	Write data to the internal buffer.
	\param	*pBuf
		the buffer to copy data from
	\param	uSize
		bytes to write
	\note Throws exceptions.
*/
	void WriteInternalBuffer(const char *pBuf, DWORD uSize);

/**
	\return	the number of free bytes on the current removable disk
*/
	DWORD GetFreeVolumeSpace() const;

/**
	Call the callback functor.
	Throw an exception if the callback functor's method \c Callback returns \c false.
	\param	iCode
		a code to be passed to the callback functor
	\param	szTemp
		a string to be used as a filename (the second argument
		of CZipException::Throw) when the exception must be thrown
	\note Throws exceptions.
	\see CZipArchive::SetSpanCallback
	\see CZipException::Throw
*/
	void CallCallback(int iCode, CZipString szTemp);


/**
	Construct the name of the volume in #tdSpan mode.
	\param	bLast
		must be \c true if constructing the last volume name (an extension "zip" is given)
	\param	lpszZipName
		the name of the archive
	\return	
		the new volume name
*/
	CZipString GetTdVolumeName(bool bLast, LPCTSTR lpszZipName = NULL) const;

	/**
		Change the disk in #tdSpan mode
	*/
	CZipString ChangeTdRead();

	/**
		Change the disk in #pkzipSpan mode
	*/
	CZipString ChangePkzipRead();
	

	/**
		Used only in \ref TDSpan "TD span mode" . The value it holds depends on the open mode.
		- Opened existing disk spanning archive - store the number of the last
		disk ( the one with "zip" extension).
		- Disk spanning archive in creation - the size of the volume.

		\see CZipArchive::Open
		\see CZipArchive::GetSpanMode

	*/
	int m_iTdSpanData;
	
	/**
		The extension of the last volume.
	*/
	CZipString m_szSpanExtension;

	/**
		\return	the count bytes left free in the write buffer
	*/
	DWORD GetFreeInBuffer() const {return m_pWriteBuffer.GetSize() - m_uBytesInWriteBuffer;}
	
	/**
		Number of bytes available in the write buffer.		
	*/
	DWORD m_uBytesInWriteBuffer;

/**
	The value it holds depends on the open mode:
	\par
	- #tdSpan : the total size of the current volume
	- #pkzipSpan: a free space on the current volume
*/
	DWORD m_uCurrentVolSize;


	/**
		number of bytes left free in the write buffer		
	*/
	DWORD m_uVolumeFreeInBuffer;

	/**
		Write buffer caching data.
	*/
	CZipAutoBuffer m_pWriteBuffer;


	/**
		Used only during disk spanning archive creation.
		Tells how many bytes have been written physically to the current volume.
	*/
	DWORD m_iBytesWritten;

	/**
		\c True, if the current archive is a new disk spanning archive.
	*/
	bool m_bNewSpan;

	/**
		The current disk in a disk spanning archive.
		Disk no 0 is the first disk.
	*/
	int m_iCurrentDisk;

	/**
		It is set to \e true when an archive is created in memory; \e false otherwise.
	*/
	bool m_bInMemory;

	/**
		It is set to \e true if OpenMode::zipOpenReadOnly was specified when opening the archive
	*/
	bool m_bReadOnly;
	
};

#endif // !defined(AFX_ZIPSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_)
