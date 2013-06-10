///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipArchive.h $
// $Archive: /ZipArchive/ZipArchive.h $
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
//
// Check the site http://www.artpol-software.com for the updated version of the library.
////////////////////////////////////////////////////////////////////////////////
//   
//	The following information files are distributed along with this library:
//		License.txt		- licensing information
//		gpl.txt			- General Public License text
//		Readme.txt		- general information
//		ChangeLog.txt	- revision history
//		faq.txt			- frequently asked questions
//		Appnote.txt		- details on the zip format
//							( also available at ftp://ftp.pkware.com/appnote.zip)
//
// 



/**
* \file ZipArchive.h
*	Interface for the CZipArchive class.
*
*/

#if !defined(AFX_ZIPARCHIVE_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_)
#define AFX_ZIPARCHIVE_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_

/**
	\namespace ziparchv
	A helper namespace.
	\par Members
		- CZipFileMapping

*/



	
/**
    \struct CZipFileMapping ZipFileMapping.h

	Maps a file to the memory. A system-specific implementation.	
	Stored in ziparchv namespace.

	\c #include "ZipFileMapping.h"
*/


// to ensure that the correct files are copied
// (see "Compatibility" chapter in the documentation).
// Rebuild the project after copying the files.
#ifdef ZIP_ARCHIVE_MFC_PROJ 
	#ifndef ZIP_ARCHIVE_MFC
		#error You need to copy files from the MFC subdirectory\
			to the ZipArchive root directory and rebuild the project
	#endif 
#elif defined ZIP_ARCHIVE_STL_PROJ
	#ifndef ZIP_ARCHIVE_STL
		#error You need to copy files from the STL subdirectory\
			to the ZipArchive root directory and rebuild the project
	#endif 
#endif //

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
	#pragma warning( disable : 4275 ) // non dll-interface class used as base for dll-interface
#endif

#ifdef __GNUC__
	#include "zlib.h"
#else
	#include "../../zlib/zlib.h"
#endif

#include "ZipException.h"
#include "ZipAutoBuffer.h"
#include "ZipCentralDir.h"	
#include "ZipStorage.h"
#include "ZipPathComponent.h"
#include "ZipString.h"
#include "ZipExport.h"


/**
	Structure used as a parameter in CZipArchive::AddNewFile(CZipAddNewFileInfo& );
	Use one of constructors provided and then adjust the member variables that are
	set to default values by #Defaults method.
*/
struct ZIP_API CZipAddNewFileInfo
{
	CZipAddNewFileInfo(LPCTSTR lpszFilePath, bool bFullPath = true)
		: m_szFilePath(lpszFilePath),m_bFullPath(bFullPath)
	{
		m_pFile = NULL;
		Defaults();
	}
	CZipAddNewFileInfo(LPCTSTR lpszFilePath, LPCTSTR lpszFileNameInZip)
		: m_szFilePath(lpszFilePath), m_szFileNameInZip(lpszFileNameInZip)
	{
		m_pFile = NULL;
		Defaults();
	}
	CZipAddNewFileInfo(CZipAbstractFile* pFile, LPCTSTR lpszFileNameInZip)
		: m_pFile(pFile), m_szFileNameInZip(lpszFileNameInZip)
	{
		Defaults();
	}
	/**
		the full path to the file to be added; if it is empty you need to set #m_pFile
	*/
	CZipString m_szFilePath;		

	/**
		the file name that will be stored in the archive (if the file is a directory,
		there is a path separator automatically added at the end); #SetRootPath
		function has no effect on this parameter
	*/
	CZipString m_szFileNameInZip;
	
	/**
	It has only the meaning when #m_szFileNameInZip is not specified and #m_szFilePath is not empty. 

	- if \c true, include full path of the file inside the archive (even if #m_szRootPath is set)<BR>
	
	- if \c false only the filename without a path is stored in the archive <BR>
	
	in this case if #m_szRootPath is set previously with #SetRootPath 
	and if the beginning of #m_szFilePath equals #m_szRootPath
	then the filename is set to the remaining part of m_szFilePath
	(you can say to m_lpszFilePath minus #m_szRootPath)		
	*/
	bool m_bFullPath;

	/**
		the compression level, see #OpenNewFile
	*/
	int m_iComprLevel;				

	/**
		the smartness level of  of the library; can be one or more #Smartness
		values (you can OR them);<BR>
		if zipsmCheckForEff is specified and due to it the file needs to be
		reinserted into the archive without a compression and the callback
		functor is specified, the callback functor's method Callback is called with the first argument set to 
		DWORD (-1)	(you need to remember the last argument if you need the size of the file)		
	*/
	int m_iSmartLevel;

	/**
		the index of the existing file in the archive to be replaced by the file being added (the new file
		goes into the same physical place as the old file - the archive may of course grow or shrink as a result)

- >= 0 the index of the file to be replaced
- -1 do not replace any file and add the new file at the end of the archive (default)
- -2 if the new file has the same name as a file already in the archive then replace it or add at the end of the archive if it's filename is unique; it uses CZipArchive::FindFile with
	the arguments \e iCaseSensitive and \e bFileNameOnly set to default values

		\note 
		- you replace files in disk-spanning archives (i.e. use a value different from -1)
		- if the space size taken by the old file is different from the space size needed by the new file, the callback is called while moving data (see CZipArchive::cbReplace)
		- this replaces the file physically, so no information from the file being replaced is retained (such as attributes, modification time, etc.)
		- if you use an invalid index, the function will fail; if you specify the last file in the archive to be replaced, it'll be removed and the usual action taken
		- the new file encryption does not depend on the old file encryption but only on the current password settings (see CZipArchive::SetPassword)
		- if #m_iComprLevel is not 0 then a temporary archive is created in the temporary directory 
		(see  CZipArchive::SetTempPath) or in the memory (if you use CZipArchive::zipsmMemoryFlag in #m_iSmartLevel)
		- <B>the most complicated scenario </B>is when you try to replace the file and use CZipArchive::zipsmCheckForEff flag in #m_iSmartLevel and the file compression proves to be 
		inefficient (you can use the sample application \e ZipArc to observe the following process): 
			- first you get CZipArchive::cbAdd callback - the file is being compressed to a temporary archive, 
			- then the file compression proves to be inefficient and the file needs to be stored instead of compressed, 
			but first the space inside archive must be adjusted to fit the file being added in the place of file being replaced - you	get	CZipArchive::cbReplace, 
			- then, at the end, the file is being stored - you get CZipArchive::cbAddStore callback
		
		\see SetAdvanced 
		\see CZipArchive::WillBeDuplicated		 
	*/
	int m_iReplaceIndex;		

	/**
		the size of the buffer used while file operations
	*/
	unsigned long m_nBufSize;		

	/**			
		instead of from a physical file, the data for compression is taken from the CZipAbstractFile object (e.g. from CZipMemFile)
	
	  \note 
	  - you have to leave #m_szFilePath empty if you set #m_pFile to not NULL
	  - the time of the file in zip is set to the current time, and the attributes to the default
	  file attributes (depending on the system)
	  - you cannot add directories this way
	*/
	CZipAbstractFile* m_pFile;

	/**
		Set default values for #m_iSmartLevel, #m_iReplaceIndex, #m_nBufSize and #m_iComprLevel
	*/
	void Defaults();
};


/**
	The representation of the zip archive file.	
	This class provides all the operations on the zip archive.	

*/
class ZIP_API CZipArchive  
{
	
public:
	/**
	The purpose of this structure is to hold the data that allow communication
	with the zlib library
	*/
	struct ZIP_API CZipInternalInfo  
	{
	
		CZipInternalInfo()
		{
			m_iBufferSize = 65536;
		}
		virtual ~CZipInternalInfo(){}

	/**
		Allocate internal buffer of m_iBufferSize size
	*/
		void Init()
		{
			m_pBuffer.Allocate(m_iBufferSize);
		}
		void ReleaseBuf()
		{
			m_pBuffer.Release();
		}
		z_stream m_stream;		///< zlib library data stream
		DWORD m_uUncomprLeft;	///< bytes left to uncompress
		DWORD m_uComprLeft;		///< bytes left to decompress
		DWORD m_uCrc32;			///< crc32 file control value

		/**
			The size of the buffer used in decompressing data.
			Set before opening the archive.
			It is usually set with CZipArchive::SetAdvanced
			(specify this value as the second argument).
			\see CZipArchive::SetAdvanced
		*/
		DWORD m_iBufferSize;

		/**
			This buffer caches data during compression and decompression.
		*/
		CZipAutoBuffer m_pBuffer;
	};

	CZipArchive();
	virtual ~CZipArchive();

/**
	In non-UNICODE version just copy \e szSingle to \e szWide.
	In UNICODE version works the same way as ZipPlatform::SingleToWide
	\param	szSingle
	\param	szWide
		
	\return	(in non-UNICODE version the number of characters copied)
	\see ZipPlatform::SingleToWide
*/
	static int SingleToWide(const CZipAutoBuffer &szSingle, CZipString& szWide);

/**
	In non-UNICODE version just copy \e lpWide to \e szSingle.
	In UNICODE version works the same way as ZipPlatform::WideToSingle
	\param	lpWide
	\param	szSingle
	\return	(in non-UNICODE version the number of characters copied)
	\see ZipPlatform::WideToSingle

*/
	static int WideToSingle(LPCTSTR lpWide, CZipAutoBuffer &szSingle);


/** 
	Set the password for the file to be opened or created.
	Use this method BEFORE opening or adding a file, but AFTER opening an archive
	\param	lpszPassword
		set it to NULL to clear password
	\return	\c false if the password contains ASCII characters
		with values 128 or higher or the file inside archive is	opened		

*/
	bool SetPassword(LPCTSTR lpszPassword = NULL);

/**
	\return	the current archive password or an empty string if there is no password
*/
	CZipString GetPassword()const ;

/**
	Set the buffer sizes. No buffer can be set smaller than 1024.
	Use this method before opening the archive. The optimal size for 
	the write buffer in the disk spanning archive is the size of the volume.

	\param	iWriteBuffer
		the write cache size used
	\see CZipStorage::m_iWriteBufferSize
	\param	iGeneralBuffer
		buffer used in extracting, compressing, deleting, getting (#GetFromArchive) files, renaming and replacing
	\see CZipInternalInfo::m_iBufferSize
	\param	iSearchBuffer
		the buffer used in searching for the central dir
	\see CZipCentralDir::m_iBufferSize
	\see GetAdvanced
*/
	void SetAdvanced(int iWriteBuffer = 65536, int iGeneralBuffer = 65536, int iSearchBuffer = 32768);

	/**
		retreive buffer size as set by #SetAdvanced
	*/
	void GetAdvanced(int* piWriteBuffer = NULL, int* piGeneralBuffer = NULL, int* piSearchBuffer= NULL)
	{
		if (piWriteBuffer)
			*piWriteBuffer = 	m_storage.m_iWriteBufferSize;
		if (piGeneralBuffer)
			*piGeneralBuffer = m_info.m_iBufferSize;
		if (piSearchBuffer)
			*piSearchBuffer = m_centralDir.m_iBufferSize;
	}

	/**
		Enum values used as \e iWhich parameter in #SetCallback function.
		You can assign several values to the same functor (OR them)
		\see CZipActionCallback
	*/
	enum CallbackType
	{
		cbNothing	= 0x0000,	///< you can use it for your own purposes
		cbAdd		= 0x0001,	///< called when adding a file with one of #AddNewFile functions
		cbAddTmp	= 0x0002,	///< called while adding a file (only on a disk-spanning archive) when the smartness level contains CZipArchive::zipsmCheckForEff or CZipArchive::zipsmCheckForEffInMem 
								///< and if just compressed file is being moved from a temporary place (file or memory) to the archive 
		cbAddStore	= 0x0004,	///< called while adding a file and if it's compressing has proven to be inefficient and it is now being stored (instead of compressed) in the archive;
								///< smartness level must contain CZipArchive::zipsmCheckForEff or CZipArchive::zipsmCheckForEffInMem; the archive can be disk-spanning or not

		cbExtract	= 0x0008,	///< called when extracting a file with one of #ExtractFile functions

		cbDeleteCnt	= 0x0010,	///< called before the actual deletion takes place and the map of holes and continuous areas is being created (it safe to break it by returning \c false from the callback method)
		cbDelete	= 0x0020,	///< called when moving data while deleting file(s) with function #DeleteFile or one of #DeleteFiles functions

		cbTest		= 0x0040,	///< called when testing a file with #TestFile function
		cbSave		= 0x0080,	///< called when saving the central directory with CZipCentralDir::Write (usually on close or flush); it is safe to break on non-disk spanning archive - the saved part of 
								///< the central directory will be removed from disk
		cbGetFromArchive 
					= 0x0100,	///< called when using one of #GetFromArchive methods
		cbRename	= 0x0200,	///< called when during the renaming a file there is a need to make less or more space for the new filename
		cbReplace	= 0x0400,   ///< called when moving data while replacing files to make less or more space for the new file
		cbNextValue = 0x0800,	///< for CZipActionCallback overrides and user defined callbacks
		

		cbSubActions= cbAddTmp | cbAddStore | cbDeleteCnt | cbReplace,				///< sub actions - they are part of bigger actions (#cbAddTmp | #cbAddStore | #cbDeleteCnt | #cbReplace)
		cbActions	= cbAdd | cbExtract | cbDelete | cbTest | cbSave | cbGetFromArchive | cbRename,	///< main actions (#cbAdd | #cbExtract | #cbDelete | #cbTest | #cbSave | #cbGetFromArchive | #cbRename)
		cbAll		= cbActions | cbSubActions							///< assign all callbacks to the same functor
	};


    /**
       
		Set callback functors for the following operations on the zip archive: adding, extracting, testing or deleting files.
		See \ref sectCallb information on how to use functors.
		\param pCallback
			the address of the functional object which class is derived from CZipActionCallback
			(you need to override member function CZipCallback::Callback in the derived class)
			Set it to \c NULL to clear callback for the selected actions.
		\param iWhich
			can be one or more (use logical OR) #CallbackType values.

		\see CZipActionCallback
		\see GetCallback
		
     */
	void SetCallback(CZipActionCallback* pCallback = NULL, int iWhich = cbAll);

	/**
		\return the callback \e iWhich set with #SetCallback
	*/
	CZipActionCallback* GetCallback(CallbackType iWhich)
	{
		return m_callbacks.Get(iWhich);
	}

	/**
		Set the callback functor used during operations on a 
		PKZIP compatible disk spanning archive to change disks.
		Set it before opening the archive. If you open the archive
		in the \ref PKSpan "PKZIP compatible mode" and don't set the callback functor, 
		the exception CZipException::noCallback will be thrown.

		Callback functor's method CZipSpanCallback::Callback is called when there is a need for a disk change.
		Calling CZipArchive methods from inside this method may result in an unexpected behavior.
		\param pCallback
			the address of the functional object which class is derived from CZipSpanCallback
			(you need to override member function CZipCallback::Callback in the derived class)
			Set it to \c NULL to clear the callback.

		\see CZipSpanCallback
	*/
	void SetSpanCallback(CZipSpanCallback* pCallback = NULL){m_storage.m_pChangeDiskFunc = pCallback;}


	
	/**
		Archive open modes used in <CODE> #Open(LPCTSTR , int , int ) </CODE>
		and <CODE> #Open(CZipMemFile, int ) </CODE>
	*/
	enum OpenMode
	{
		zipOpen,			///< open an existing archive
		/**
			Open an existing archive as a read only file.
			This mode is intended to use in a self extract code or opening 
			archive on storage without write access (e.g. CD-ROMS),
			If you try to modify the archive in this mode,
			an exception will be thrown.
		*/
		zipOpenReadOnly,
		zipCreate,			///< create a new archive
		zipCreateSpan		///< create a disk spanning archive
	};



/**
	Open or create a zip archive.

	The archive creation mode depends on \e iMode and \e iVolumesSize values:
	- if \e iMode == #zipCreateSpan and \e iVolumeSize <= 0 then create disk spanning 
		archive in \ref PKSpan "PKZIP compatible mode" (pkzipSpan)
	- if \e iMode == #zipCreateSpan and \e iVolumeSize > 0 then create disk spanning 
		archive in \ref TDSpan "TD compatible mode" (tdSpan)
	- if \e iMode == #zipOpen and the existing archive is a spanned archive
		then pkzipSpan mode is assumed if the archive is on a removable device
		or tdSpan otherwise;			<BR>
		if you want to open tdSpan archive on a removable device, set \e iVolumeSize
		to a value different from 0	
	- if \e iMode == #zipCreate then \e iVolumeSize doesn't matter

	\param	szPathName
		the path to the archive
	\param	iMode
		one of the #OpenMode values
	\param	iVolumeSize
	the volume size in the disk spanning archive; 
	the size of the volume may be from 1 to INT_MAX , 
	and the bigger - the faster is creation and extraction (no file changes between volumes) - but the size of the whole archive is the same. 
	If you're creating disk-spanning archive in \ref TDSpan "TD span compatible mode" 
	and plan later to convert it to \ref PKSpan "PKZIP compatible mode" (see \ref sectSpan), it is good to set this this value to 
	about the size of the diskette (a little less just in case).
	
	\note Throws exceptions.
	\see Open(CZipMemFile&, int);
	\see GetSpanMode
*/
	void Open(LPCTSTR szPathName, int iMode = zipOpen, int iVolumeSize = 0);


/**
	Open or create the archive in memory. The CZipMemFile object is not closed
	after closing the archive, so that is it possible to work with it afterwards.
	\param	mf
		CZipMemFile structure to create archive in or extract from
		\note This is important: you shouldn't destroy CZipMemFile object before
		closing the archive, because you'll get an error;
	\param	iMode
		Open mode. 
		The following modes are valid:  #zipOpen, #zipOpenReadOnly, #zipCreate
	\note	Throws exceptions.
	\see Open(LPCTSTR, int, int);
*/
	void Open(CZipMemFile& mf, int iMode = zipOpen);

/**
	Set #m_szRootPath to a specified value. Use it if you don't want to set
	\e bFullPath argument in #AddNewFile or #ExtractFile to true and you 
	don't want to strip the whole path neither, but only a specific beginning.
	Use it AFTER opening the archive and before using #AddNewFile or #ExtractFile.
	See \ref q9 "the FAQ" for the example of use.
	\param	szPath
		Set it to the string that you want to be omitted from the beginning of the path of the file
		in the archive <BR>
		if \c NULL - clears the #m_szRootPath and no path beginning will be matched against it

	\note Set the case-sensitivity with #SetCaseSensitivity

	\see AddNewFile 
	\see ExtractFile
	\see GetRootPath

*/
	void SetRootPath(LPCTSTR szPath = NULL);


    /**
       Return the current value of #m_szRootPath
       \return CZipString 
     */
	CZipString GetRootPath()const 
	{
		return m_szRootPath;
	}

	/**
		The levels of smartness of the adding files action (see #AddNewFile)
		\note If you wish to use \e zipsmCheckForEff, you should use 
		\e zipsmNotCompSmall as well, because it will save you the time
		( the small file will be surely larger after compression, so that
		we can add it not compressed straight away);the compression level 
		is always ignored for a directory and set to 0
	*/

	enum Smartness
	{
		zipsmLazy			= 0x0000,		///< do not bother (you know what you're doing after all)
		zipsmCPassDir		= 0x0001,		///< clear password for directories
		zipsmCPFile0		= 0x0002,		///< clear password for files of 0 size
		zipsmNotCompSmall	= 0x0004,		///< do not compress files smaller than 5 bytes (they are always stored larger than uncompressed)
		zipsmCheckForEff	= 0x0008,		///< check whether the compressed file is larger than uncompressed and if so, remove it and store without the compression; 
										///< in the disk spanning mode, the temporary file is used for that: if the file compression is efficient, the data is not compressed
										///< again, but moved from the temporary file to the archive; you can use #SetTempPath() to set the path where the 
										///< file will be created or you can let the library figure it out (the library tries first the system default temporary directory,
										///< if it is not present or there is not enough space there, it tries the current directory, if it fails, no temporary file is created
										///< and the compression goes the usual way

		zipsmMemoryFlag		= 0x0010,  ///< combine it with zipsmCheckForEff or use zipsmCheckForEffInMem, you can also use this when replacing files see note at CZipAddNewFileInfo::m_iReplaceIndex

		zipsmCheckForEffInMem = zipsmMemoryFlag | zipsmCheckForEff, ///< the same as #zipsmCheckForEff, but the temporary file is created created in memory instead (the temporary directory set with #SetTempPath() is ignored); has the meaning only with a disk-spanning archive,
																	///< non-disk spanning archives don't need a temporary file 
		zipsmSmartPass	= zipsmCPassDir | zipsmCPFile0,			///< smart password policy (a combination of \e zipsmCPassDir and \e zipsmCPFile0)
		zipsmSmartAdd = zipsmNotCompSmall | zipsmCheckForEff,	///< smart adding (a combination of \e zipsmNotCompSmall and \e zipsmCheckForEff)
		zipsmSafeSmart = zipsmSmartPass | zipsmNotCompSmall,    ///< safe smart (smartest without checking for efficiency)
		zipsmSmartest = zipsmSmartPass | zipsmSmartAdd,			///< smart at its best

		
		zipsmInternal01		= 0xf000   ///< for internal use only

	};

	
	/**
		Add a new file to the archive. You can set the callback functor with #SetCallback.
		\param info see CZipAddNewFileInfo
		\return	\c if it returns false then the file was not added, but the internal
		state allows you to add other files (which is not possible after throwing 
		an exception)
		
		\note 
		- If you abort while adding a file in a non-disk-spanning archive, the added data will be removed from the archive
		- Throws exceptions.
		
		\see SetCallback
		\see <CODE> AddNewFile(LPCTSTR, LPCTSTR, int, int, unsigned long) </CODE>
		\see <CODE> AddNewFile(LPCTSTR, int, bool, int, unsigned long) </CODE>
		\see <CODE> AddNewFile(CZipMemFile&, LPCTSTR, int, int, unsigned long)</CODE>
	*/
	bool AddNewFile(CZipAddNewFileInfo& info);


	/**
		\see AddNewFile(CZipAddNewFileInfo& ); the parameters are equivalent to CZipAddNewFileInfo member variables
	
	*/
	bool AddNewFile(LPCTSTR lpszFilePath, int iComprLevel = -1, bool bFullPath = true,
		int iSmartLevel = zipsmSafeSmart, unsigned long nBufSize = 65536);


	/**
		\see AddNewFile(CZipAddNewFileInfo& ); the parameters are equivalent to CZipAddNewFileInfo member variables
	*/
	bool AddNewFile(LPCTSTR lpszFilePath,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel = -1,                             
							 int iSmartLevel = zipsmSafeSmart,
                             unsigned long nBufSize = 65536);

	/**
		\see AddNewFile(CZipAddNewFileInfo& ); the parameters are equivalent to CZipAddNewFileInfo member variables
	*/
	bool AddNewFile(CZipMemFile& mf,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel = -1,                             
							 int iSmartLevel = zipsmSafeSmart,
                             unsigned long nBufSize = 65536);



/**
	Add a new file to the opened zip archive. The zip archive cannot be 
	an existing disk spanning archive (existing at the moment of opening archive),
	because modifying such an archive is not possible (at least not with this
	version ).

	\param	header
		The structure that provides additional information about the added file.
		The following fields are valid:
		- \e m_uMethod - file compression method; can be 0 (storing) or Z_DEFLATE (deflating)
			otherwise Z_DEFLATE is assumed
		- \e m_uModDate, \e m_uModTime - Use CZipFileHeader::SetTime to set them.
			If \e lpszFilePath is not NULL this fields are overwritten and updated automatically.
			See CZipFileHeader::SetTime
		- \e m_uExternalAttr - Attributes of the file.
			If \e lpszFilePath is not NULL this field is overwritten and updated automatically.
			Use #SetFileHeaderAttr to set them. See #SetFileHeaderAttr.
		- \e m_szFileName - A filename (may be with path) to be stored inside archive
			to represent this file. See CZipFileHeader::SetFileName
		- \e m_szComment - A file comment.	See CZipFileHeader::SetComment
		- \e m_pExtraField - LOCAL extra field, use #SetExtraField after opening 
			a new file, but before closing it to set the not local extra field 
			in the header in the central directory. See #SetExtraField	<BR>
	Other fields are ignored - they are updated automatically.
	If the function returns \c true, \link #GetSystemCompatibility 
	system compatibility \endlink for this object is
	set to the correct value (the same as #m_iArchiveSystCompatib),
		additionally if \e lpszFilePath was not NULL, the attributes and 
	the time fields are filled with information retrieved from 
	the file pointed by \e lpszFilePath.		
	\param	iLevel
		The level of compression (-1, 0 - 9).
		The are several preset values for the compression level:
		- Z_DEFAULT_COMPRESSION	: -1 (equals 6)
		- Z_NO_COMPRESSION		: 0
		- Z_BEST_SPEED			: 1
		- Z_BEST_COMPRESSION	: 9			
	\param	lpszFilePath
		The path to the file to retrieve date stamp and attributes from.
		These values are stored inside the archive.
	\param uInternal
		for internal use only

	\return	\c false in the following cases:
	- the \e lpszFilePath is not NULL and the file attributes and data was not correctly retrieved
	- a file is already opened for extraction or compression
	- archive is an existing disk span archive
	- maximum file count inside archive already reached (65536)
	\note Throws exceptions.

*/
	bool OpenNewFile(CZipFileHeader & header, int iLevel = Z_DEFAULT_COMPRESSION, LPCTSTR lpszFilePath = NULL, DWORD uInternal = 0);

/**
	Compress the contents of the buffer and write it to a new file.
	\param	pBuf
		the buffer containing the data to be compressed and written
	\param	iSize
		the number of bytes to be written from the buffer
	\return	\c false if the new file hasn't been opened yet
	\note Throws exceptions.
	\see OpenNewFile
*/
	bool WriteNewFile(const void *pBuf, DWORD iSize);


/**
	Set the extra field in the central directory of the currently opened file.
	Must be used after opening a new file in the archive, but before closing it
	To set the local extra field, set it in the CZipFileHeader structure passed
	as the argument to the #OpenNewFile
	\param	pBuf
		the bufer with the data to be copied
	\param	iSize
		the size of the extra field in the buffer
	\see OpenNewFile
*/
	void SetExtraField(const char *pBuf, WORD iSize);

/**
	Close the new file in the archive.
	\return \c false if there is no new file opened 
	\param bAfterException 
		it will close the new file without writing anything (call it also
		with this parameter set to \c true after an exception other than CZipException::abortedSafely was
		thrown from one of #AddNewFile functions)
	\note Throws exceptions.
	\see OpenNewFile
*/
	bool CloseNewFile(bool bAfterException = false);

/**
	Acquire a file with the given index from another archive. 
	The compressed data of the file from another archive are copied 
	without decompressing to the current archive.
	You can set the callback functor with #SetCallback.	
	\param zip 
		an opened archive to get the file from (must not be multi-disk)
	\param	uIndex
		a zero-based index of the file to get from the \e zip archive
	\param iReplaceIndex the same as CZipAddNewFileInfo::m_iReplaceIndex
	\param bKeepSystComp
		if \c false, which is default, then the file from \e zip archive
		system compatibility is converted to the current archive system
		compatibility (if they differ)	
	\return \c false if the operation could not be performed (either of archives is closed, 
	a file inside either of archives is opened, \e zip archive is multi-disk or the current
	archive is an existing multi-disk archive)
	\note Throws exceptions <BR>
	(when an exception is thrown, you may need to call #CloseNewFile with \e bAfterException set to \c true, to make the archive reusable).	
	\note it is safe to abort the action (by returning false from the callback call) in non-disk spanning archive and when no replacing is taking place
	(the file not entirely added is removed from the archive)
	\see SetCallback
	\see GetFromArchive(CZipArchive& , CZipWordArray &, bool)
	\see GetFromArchive(CZipArchive& , CZipStringArray &, bool)
	\see FindMatches
	\see SetAdvanced 
*/
	bool GetFromArchive(CZipArchive& zip, WORD uIndex, int iReplaceIndex = -1, bool bKeepSystComp = false)
	{
				
		m_info.Init();
		bool bRet;
		try
		{
			bRet = GetFromArchive(zip, uIndex, iReplaceIndex, bKeepSystComp, GetCallback(cbGetFromArchive));
		}
		catch(...)
		{
			m_info.ReleaseBuf();
			throw;
		}
		m_info.ReleaseBuf();
		if (bRet && m_bAutoFlush)
			Flush();

		return bRet;
	}

	/**
	Acquire files with the given indexes from another archive. 
	
	\param	aIndexes
		an array of zero-based indexes of the files inside the \e zip archive	

	\see GetFromArchive(CZipArchive& , WORD, int, bool)

	\note 
	- To get files which filenames match a specified pattern, use #FindMatches function
	*/
	bool GetFromArchive(CZipArchive& zip, CZipWordArray &aIndexes, bool bKeepSystComp = false);
	
	/**
	Acquire files with the given indexes from another archive. 
	\param	aNames
		an array of filenames inside the \e zip archive; 	

	\see GetFromArchive(CZipArchive& , WORD, int, bool)
	\see EnableFindFast

	\note 
	- Set the case-sensitivity with #SetCaseSensitivity
	- Enables FindFast if not enabled
	
	*/
	
	bool GetFromArchive(CZipArchive& zip, CZipStringArray &aNames, bool bKeepSystComp = false)
	{
		CZipWordArray indexes;
		zip.GetIndexes(aNames, indexes);
		return GetFromArchive(zip, indexes, bKeepSystComp);
		
	}

	/**
		Get indexes of the files stored int \e aNames array and put them into \e aIndexes
		\param	aNames
			an array of filenames inside the archive; 
		\param aIndexes
			an array of indexes to be build
		\note 
		- Set the case-sensitivity with #SetCaseSensitivity
		- Enables FindFast if not enabled
			
	*/
	void GetIndexes(const CZipStringArray &aNames, CZipWordArray& aIndexes);

/**
	Extract the file from the archive. You can set the callback functor with #SetCallback.
	The argument \e lpszNewName may point to the full path and is influenced by \e bFullPath 
	argument (if \e lpszNewName contains drive specification then it is removed)
	\param	uIndex
		the index of the file to extract
	\param	lpszPath
	\verbatim
		The PATH only to extract the file to. May not be NULL. If you wish to 
		use UNC path you need to replace \\\\ at the beginning of UNC path with \\\\?\UNC\ .
	\endverbatim
	\param	bFullPath <BR>
		- if \c true, then extract with the full path - in this case the resulting
		file path is \e lpszPath plus the path stored in the archive or plus \e lpszNewName 
		if \e lpszNewName is not NULL. <BR>
		- if \c false, the destination file path is \e lpszPath + \e the filename only
		extracted from the path stored in the archive or from \e lpszNewName if
		\e lpszNewName is specified; <BR>
		if #m_szRootPath is set previously with #SetRootPath then to \e lpszPath
		is added the path stored in the archive (or \e lpszNewName if
		\e lpszNewName is specified) that has removed the beginning that equals 
		#m_szRootPath (if there is no common beginning then is behaves like 
		#m_szRootPath was empty)
	\param	lpszNewName 
			The new name of the file after extraction. 
			If NULL the original filename stored in the archive is used.
			May point to the full path but, if \e bFullPath is \c false, only the filename is extracted from this argument,			
	\param	nBufSize 
		the size of the buffer used while file operations
	\return	\c true if successful
	\note 
	- To extract files which filenames match a specified pattern, use #FindMatches function
	- Throws exceptions.
	\see SetCallback
	\see <CODE> ExtractFile(WORD, CZipMemFile&, DWORD) </CODE> 
	\see FindMatches
*/
	bool ExtractFile(WORD uIndex, LPCTSTR lpszPath, bool bFullPath = true,
		LPCTSTR lpszNewName = NULL, DWORD nBufSize = 65536);

	
	/**
		The same as <CODE> #ExtractFile(WORD , LPCTSTR , bool , LPCTSTR , DWORD ) </CODE>
		but instead to a physical file, this function decompress the data into CZipMemFile object
		\note 
		- if you pass CZipMemFile object already with data, its contents are NOT overwirtten, but the decompressed data is appended at the end
		- if you try to extract a directory, the function will return \c false
	*/
	bool ExtractFile(WORD uIndex, CZipMemFile& mf, DWORD nBufSize = 65536);

/**
	Open the file with the given index in the archive for extracting.
	Not successful opening the file doesn't lock the whole archive, so 
	you can try to open another one (after catching an exception if it was 
	thrown). Throw exception CZipException::badPassword if the password
	was not set for the encrypted file.
	\param	uIndex
		the index of the file
	\return	\c true if successful
	\note Throws exceptions.
*/
	bool OpenFile(WORD uIndex);

/**
	Decompress currently opened file to the buffer.
	\param	pBuf
		buffer to receive data
	\param	iSize
		the size of the buffer
	\return	the number of bytes read
	\see OpenFile
	\note Throws exceptions.
*/
	DWORD ReadFile(void *pBuf, DWORD iSize);


/**
	Test the file with the given index for the integrity. You can set the callback functor with #SetCallback.
 	The method throws exceptions but performs all the necessary cleanup
	before, so that the next file can be tested after catching the exception.
	\param	uIndex
		index of the file to test
	\param	uBufSize
		the size of the buffer used during extraction
	\return \c false if the incorrect action has been taken by 
	the user or the programmer (it is when #OpenFile or #GetFileInfo returned \c false or \e uBufSize is 0).
	If the file didn't passed the test or there was a disk I/O error or the password supplied was incorrect, an exception is thrown.
	\note Throws exceptions.
	\see SetCallback
	
*/
	bool TestFile(WORD uIndex, DWORD uBufSize = 65536);

/**
	Perform the necessary cleanup after an exception was thrown
	while testing the archive so that next files in the archive can be tested.
	Called by #TestFile. Does not remove the file headers
	information from the central directory.
	\see TestFile
	\see CZipCentralDir::Clear
*/
	void CloseFileAfterTestFailed();

/**
	Get the local extra filed of the currently opened 
	for extraction file in the archive.
	\param	pBuf
		the buffer to receive the data
	\param	iSize
		the size of the buffer
	\return	If \e pBuf is NULL or iSize is 0, returns the size of the local extra field.
	Returns -1 if there is no file opened for the extraction.
*/
	int GetLocalExtraField(char* pBuf, int iSize)const ;

/**
	The same as CZipArchive::CloseFile(LPCTSTR), but additionally
	closes \e file.
	\param	file
		OPENED CZipFile structure of the extracted file
	\return	
	\note Throws exceptions.
	\see CZipArchive::CloseFile(LPCTSTR)
*/
	int CloseFile(CZipFile &file);


/**
	Close the file opened for extraction in the archive and copy its date and 
	attributes to the file pointed by \e lpszFilePath
	\param	lpszFilePath 
		Points to the path of the file to have the date and attributes information updated.
	\param bAfterException
		Set to \c true to close the file inside archive after an exception has been 
		thrown, to allow futher operations on the archive.
	\warning Close the file pointed by \e lpszFilePath before using this method, 
		because the system may not be able to retrieve information from it.
	\return	<BR>
	-  "1" = ok
	-  "-1" = some bytes left to uncompress - probably due to a bad password or corrupted archive
	-  "-2" = setting extracted file date and attributes was not successful	
	\note Throws exceptions.
*/
	int CloseFile(LPCTSTR lpszFilePath = NULL, bool bAfterException = false);

/**
	Delete the file from the archive with the given index. 
	You can set the callback functor with #SetCallback.
	If you plan to delete more than one file, use one of <EM>DeleteFiles </EM>functions rather than calling DeleteFile
	successively, because these functions are optimized for deleting multiple files
	\param	uIndex
		a zero-based index
	\note Throws exceptions.
	\see SetCallback
	\see DeleteFiles(CZipWordArray&)
	\see DeleteFiles(const CZipStringArray&)
	\see FindMatches
*/
	void DeleteFile(WORD uIndex);

/**
	Delete files from the archive.
	You can set the callback functor with #SetCallback.
	Sorts \e aIndexes array in an ascending order.
	\param	aIndexes
		an array of zero-based indexes of the files inside the archive	
	\note 
	- To remove files which filenames match a specified pattern, use #FindMatches function
	- Throws exceptions.
	\see SetCallback
	\see DeleteFile
	\see DeleteFiles(const CZipStringArray& )
	\see FindMatches
*/
	void DeleteFiles(CZipWordArray &aIndexes);


/**
	Delete files from the archive.
	You can set the callback functor with #SetCallback.
	\param	aNames
		an array of filenames inside the archive; 	
	\note 
	- Set the case-sensitivity with #SetCaseSensitivity
	- Enables FindFast if not enabled
	- Throws exceptions.
	\see SetCallback
	\see DeleteFile
	\see DeleteFiles(CZipWordArray&)
	\see EnableFindFast
*/
	void DeleteFiles(const CZipStringArray &aNames);


/**
	Set the global comment in the archive.
	\param	lpszComment
		the file comment		
	\return \c false if the archive is closed or if it is an existing disk spanning archive
	\note Throws exceptions.
*/
	bool SetGlobalComment(LPCTSTR lpszComment);


/**
	\return the global comment or an empty string if the archive is closed	
*/
	CZipString GetGlobalComment()const ;


/**
	Set the comment of the file with the given index inside the archive.
	\param	uIndex
		zero-based index of the file in the archive
	\param	lpszComment
		a comment to add
	\return	\c false if the comment change is impossible
	\note Throws exceptions.
*/
	bool SetFileComment(WORD uIndex, LPCTSTR lpszComment);

/**
	\return the path of the currently opened archive volume	
*/
	CZipString GetArchivePath()const;

/**
	\return	<BR>
	- a one-based number of the current disk
	- 0 if there is no current disk (the archive is closed)
	\note Useful mostly while working with the disk-spanning archive in creation to find out
	how many disks were already created. To find out how many disks are in an existing disk-spanning archive,
	use the function #GetCentralDirInfo
*/
	int GetCurrentDisk()const ;

/**
	Return the disk spanning mode of the current archive.

	\return	<BR>
	- -2 - existing TD mode compatible disk spanning archive
	- -1 - existing PKZIP compatible 
	- 0 - no disk spanning
	- 1 - PKZIP compatible in creation
	- 2 - TD compatible in creation

  \see \ref PKSpan, \ref TDSpan

*/
	int GetSpanMode()const 
	{
		return m_storage.m_iSpanMode * m_storage.IsSpanMode();
	}

	/**
		case-sensitivity values used as argument \e iCaseSensitive in #FindFile
	*/
	enum FFCaseSens
	{
		ffDefault,  ///< use the default case-sensitivity as set with #SetCaseSensitivity function;
					 ///< if CZipCentralDir::m_findarray was build before with a different case-sensitivity,
					 ///< it is rebuild again, if it hasn't been build so far, it is build now with the
					 ///< default case-sensitivity
		ffCaseSens,  ///< perform a case-sensitive search (if the \c CZipArchive is non-case-sensitive,
					 ///< a less effective search is perfomed); does not rebuild CZipCentralDir::m_findarray,
					 ///< but if the array hasn't been built yet, it is build now as \b non-case-sensitive
					 ///< (you can use \c SetCaseSensitivity(true) and then #ffDefault to build it as case-sensitive)
		ffNoCaseSens ///< perform a non-case-sensitive search (if the \c CZipArchive is case-sensitive,
					 ///< a less effective search is performed); does not rebuild CZipCentralDir::m_findarray,
 					 ///< but if the array hasn't been built yet, it is build now as \b case-sensitive
					 ///< (you can use \c SetCaseSensitivity(false) and then #ffDefault to build it as non-case-sensitive)

	};

/**
	Find the file in the archive.
	If the archive wasn't opened with CZipCentralDir::m_bConvertAfterOpen set to \c true, 
	this function automatically convert all the filenames with the function
	CZipCentralDir::ConvertAll and set CZipCentralDir::m_bConvertAfterOpen to \c true.
	This function requires \link CZipCentralDir::m_bFindFastEnabled FindFast \endlink
	feature enabled.
	\param	lpszFileName
		the name of the file to be found in the archive; must be with path unless
		you set \e bFileNameOnly to \c true. Use path separators the same as they are for your system 
		(\e "\" for Windows and \e "/" for Unix/Linux)
	\param iCaseSensitive can be one of #FFCaseSens values
	\param bFileNameOnly
		if \c true, the function tries to find a filename without a path (a less effective search is performed); if you wish to find
		a directory name, do not end it with the path separator, which is required if you set \e bFileName to \c false		
	\return	<BR>
	- the index of the file found 
	- -1 if there is no such a file in the archive
	\see CZipCentralDir::FindFileNameIndex
	\see EnableFindFast
	\see CZipCentralDir::ConvertAll
	\see SetConvertAfterOpen
*/
	int FindFile(LPCTSTR lpszFileName, int iCaseSensitive = ffDefault, bool bFileNameOnly = false);

/**
	Get the info of the file with the given index.
	\param	fhInfo
		the structure to receive info			
	\param	uIndex
		a zero-based index of the file inside the archive
	\return	\c true if successful
*/
	bool GetFileInfo(CZipFileHeader & fhInfo, WORD uIndex) const;


/**
	\param	bOnlyFiles
		if \c true, the directories are not inluded in a total count;
		default is \c false
	\return	the number of files in the archive
*/
	int GetCount(bool bOnlyFiles = false)const 
	{
		int iTotalCount = m_centralDir.m_headers.GetSize();
		if (bOnlyFiles)
		{
			int iCount = 0;
			for (int i = 0; i < iTotalCount; i++)
				if (!m_centralDir.m_headers[i]->IsDirectory())
					iCount++;
			return iCount;
		}
		else
			return iTotalCount;
	}

	/**
		values used in #Close function as parameter \e iAfterException
	*/
	enum CloseAfterException
	{
		afNoException,		///< normal close, no exception was thrown before by CZipArchive object
		afAfterException,	///< an exception has been thrown, don't write any data but perform necessary cleaning to reuse CZipArchive object for another archive
		afWriteDir			///< the same as above, but write the central directory end structure to the archive, so that we can save at least the files that have
							///< been added properly and maybe try to repair the archive later
	};

/**
	Close the archive.
	\param	iAfterException
		one of #CloseAfterException enum values
	\param bUpdateTimeStamp
		if \c true, set the modification date of the zip file to the date of the newest file in the archive;		
		in disk-spanning mode only the last archive file will have the time stamp updated; 
		you can use this option even without performing any additional processing on the archive, just open and close it
	\note Throws exceptions if \e iAfterException is different from \c afAfterException
*/
	void Close(int iAfterException = afNoException, bool bUpdateTimeStamp = false);


/**
	Test if the archive is closed (a whole or the current volume only)
	\param	bArchive <BR>
		- \c true: test for the whole archive
		- \c false: test for the volume file only		

	\return	\c true if closed
*/
	bool IsClosed(bool bArchive = true)const ;

	/**
		Write the central directory to the archive and flushes the internal buffers to the disk, 
		so that the archive is finalized on the disk, but you can still modify it. Use it after 
		opening (or creating) and modifying the archive if you want to prevent the loss 
		of the data you've compressed so far in case of the program crash. <BR>
		If you use it on a disk spanning archive in creation it will not be closed, but its state
		will be changed from "archive in creation" to "an existing span archive". Use it when you finish adding files to the disk-spanning archive and want to 
		begin extracting or testing it. It follows that you can call it only once in this case. However, if 
		after finalizing the disk spanning archive it turns out that it is one disk only, it is converted to 
		a normal archive and you can use it as such. If you want to know what is the state of the archive after using
		this function call #GetSpanMode.
		\note 
		- Cannot be used on existing disk spanning archives (they are not modifable anyway)
		- If you have an archive with a huge central directory, it'll influence the perfomance calling this function without a reason.
		- Throws exceptions.
		\see GetSpanMode
		\see SetAutoFlush
		\see GetAutoFlush

	*/
	void Flush();


	/**
		Set the CZipArchive object to call #Flush after each operation that modifies the archive
		(adding a new file, deleting file(s), modifying the global or a file comment).
		It is useful when we want to prevent the loss of data in case of the program crash - the zip file will be then finalized on the disk.
		Use it after opening the archive.
		\note 
		- You can set AutoFlush only for non-disk spanning archives, however you can call #Flush once for a disk-spanning archive in creation.
		- If you have an archive with a huge central directory, setting Auto-Flush will influence the performance.
		\see Flush	
		\see GetAutoFlush
	*/
	void SetAutoFlush(bool bAutoFlush = true);
	

	/**
		return the current #m_bAutoFlush value
		\see Flush	
		\see SetAutoFlush
	*/
	bool GetAutoFlush()const {return m_bAutoFlush;}

/**
	Return the system compatibility of the current archive.
	System compatibility value for the single file in the archive
	(represented by CZipFileHeader) influences file attributes conversion
	(the file attributes are defined differently across the platforms).
	When opening an existing archive CZipArchive assumes the system compatibility
	of the whole archive to be the same as of the first file in the archive
	(if present). In other cases the current system value is assumed which is
	taken from ZipPlatform::GetSystemID during creating or opening an archive
	\remark
	If the existing archive during opening is empty, ZipPlatform::GetSystemID
	is assumed to be the default system for the files that will be added to the archive.
		
	\return	
		one of the enum values defined in \link ZipCompatibility::ZipPlatforms
		ZipCompatibility.h \endlink
	\see ZipCompatibility::ZipPlatforms
	\see ZipPlatform::GetSystemID
	\see CZipFileHeader::GetSystemCompatibility
*/
	int GetSystemCompatibility() const {return m_iArchiveSystCompatib;}

/**
	Set the system compatibility of the archive. By default it is set to the 
	current system value (the one returned by ZipPlatform::GetSystemID() function).
	Use it after opening the archive, but before adding a new file or using 
	SetFileHeaderAttr() function
	\param iSystemComp
		can be one of ZipCompatibility::ZipPlatforms values
	\return
		return \c false if the value \e iSystemComp is not supported 
		(ZipCompatibility::IsPlatformSupported returns \c false for the value)
		or it is not possible to set it right now
*/
	bool SetSystemCompatibility(int iSystemComp);

/**
	Set the attributes for CZipFileHeader structure to be used
	in #OpenNewFile method as an argument.
	This special procedure is taken, because the system compatibility must
	be set for CZipFileHeader prior to the value, which must be identical to
	the return value of #GetSystemCompatibility method.
	\param	header
		the structure to have attributes set
	\param	uAttr
		attributes to set
	\note Throws exceptions if the archive system or the current system 
	is not supported by the ZipArchive library.
	\see GetSystemCompatibility
*/
	void SetFileHeaderAttr(CZipFileHeader& header, DWORD uAttr);


/**
	A helper for a various purposes (needed e.g. by the program that
	cracks the zip archives password)
	\return	the pointer to the static CRC table in the zlib library
	
*/
	static const DWORD* GetCRCTable()
	{
			return get_crc_table();
	}

/**
	Return the underlying archive storage medium.
	\warning A method for a very advanced use - you normally never need it.
	\return	the pointer to #m_storage
	\see CZipStorage
*/
	CZipStorage* GetStorage(){return &m_storage;}


/**
	Set #m_bDetectZlibMemoryLeaks value.
	\param	bDetect
	\note Use before opening a file in the archive.
	\see m_bDetectZlibMemoryLeaks
		
*/
	void SetDetectZlibMemoryLeaks(bool bDetect)
	{
		if (m_iFileOpened != nothing)
		{
			TRACE(_T("CZipArchive::SetDetectZlibMemoryLeaks: Set it before opening a file in the archive"));
			return;
		}
		m_bDetectZlibMemoryLeaks = bDetect;
		
	}

/**
	Set CZipCentralDir::m_bConvertAfterOpen value.
	The default value is \c true.
	Setting this value to \c false is generally not effective and is intended 
	only for quick and short operations on archives with lots of files inside
	(e.g. open archive, make an operation on a file which index you already know
	and close the archive - using #FindFile function already makes setting this 
	value to \c false inefficient)
	
	\param	bConvertAfterOpen
	\note Use before opening the archive.
	\see CZipCentralDir::m_bConvertAfterOpen		
*/
	void SetConvertAfterOpen (bool bConvertAfterOpen)
	{
		if (!IsClosed())
		{
			TRACE(_T("CZipArchive::SetConvertAfterOpen: Set it before opening the archive"));
			return;
		}
		m_centralDir.m_bConvertAfterOpen = bConvertAfterOpen;

	}

/**
	Enable fast finding by the file name of the files inside the archive.
	Set CZipCentralDir::m_bFindFastEnabled to \c true, which is required by #FindFile.
	#FindFileIt is called by #FindFileif necessary. It builds CZipCentralDir::m_findarray
	with the default case-sensitivity (set with #SetCaseSensitivity)
	\note Call it only after opening the archive. 
	\param	bEnable
	\see CZipCentralDir::m_bFindFastEnabled
	\see FindFile
*/
	void EnableFindFast(bool bEnable = true);


    /**
		After you enable FindFast feature with #EnableFindFast, you can retrieve 
		how the files are sorted in CZipCentralDir::m_findarray (you can use it 
		in your own program if you need to display the sorted list and do not want
		to duplicate data)
       
       \param iFindFastIndex
			index in CZipCentralDir::m_findarray (not necessary the same as the one you'd
			pass to #GetFileInfo); the number of items in this array is the same
			as the number of files in the archive (you can retrieve it with 
			#GetCount)
       
       \return 
	        index of the file in central directory (now you can call #GetFileInfo to get
			the information); if the value is \c -1 then you have not called 
			#EnableFindFast before or the archive is closed or the \e iFindFastIndex
			is out of range)
     */
	int GetFindFastIndex(int iFindFastIndex) const
	{
		if (IsClosed())
		{
			TRACE(_T("CZipArchive::GetFindFastIndex: ZipArchive not yet opened.\n"));
			return -1;
		}
		
		return m_centralDir.GetFindFastIndex(iFindFastIndex);
	}


	/**
		Set a temporary path used when compressing files and #zipsmCheckForEff
		is specified as an argument in #AddNewFile and the disk spanning archive
		is in creation.
		\param lpszPath set it to NULL to clear the temporary path and let the 
		library figure it out (it uses the system temporary directory if possible)
		\param bForce if \e lpszPath is not \c NULL and this parameter set to \c true
			the directory is created if it doesn't exists (if the given directory 
			does not exists , the temporary file will not be created)
		\see AddNewFile
		\see Smartness
		\see GetTempPath
	*/
	void SetTempPath(LPCTSTR lpszPath = NULL, bool bForce = true);



	/**
		enum values used in #PredictFileNameInZip
	*/
	enum Predict
	{
	
		prDir,  ///< if an argument is a directory, add a separator at the end
		prFile, ///< treat the argument as a common file
		prAuto  ///< treat the argument as a directory only if it has a path separator at the end
				
	};

    /**
       Given the file path in the form it would be passed to #AddNewFile
	   as \e lpszFilePath argument, the function returns the file name 
	   as it would be stored in the archive. <BR>
	   The function takes into account the root path set with #SetRootPath. You can use this function
	   to eliminate duplicates before adding a list of files.<BR>
       
       \param lpszFilePath
			the file path as it would be passed to #AddNewFile function
	   \param bFullPath
			the same as \e bFullPath in #AddNewFile
       \param iWhat
			one of #Predict values to treat \e lpszFilePath correctly
		\param bExactly
			if \c true, return the filename exactly as it would look inside the archive i.e. convert slash to backslash, and perform ANSI->OEM conversion;
			otherwise return the filename with the path separators that are used by default in the system
       
       \return a modified file path
     */
	CZipString PredictFileNameInZip(LPCTSTR lpszFilePath, bool bFullPath, int iWhat = prAuto, bool bExactly = false)const ;

	/**
		Check if the filename will be duplicted in the archive, if added to the archive in the given form
		\param bFileNameOnly
			if \c true, assume that the filename is duplicated if only the filename (no path) is the same (\e bFullPath is ignored), otherwise the whole filename with path is taken into account.
			
		\b Default: \c false
		
		 The rest of the parameters have the same meaning as in #PredictFileNameInZip.

		\return the zero-based index of the file in the archive that the filename would be duplicated, or -1, if the filename is unique
	*/
	int WillBeDuplicated(LPCTSTR lpszFilePath, bool bFullPath, bool bFileNameOnly = false, int iWhat = prAuto);


    /**
       Predict the full filename with path after extraction. The parameters (except for the first) are in the form you'd pass
	   to the <CODE> #ExtractFile(WORD , LPCTSTR , bool , LPCTSTR , DWORD ) </CODE> function. The function takes into account the root path set with #SetRootPath.
       \param lpszFileNameInZip
			the filename of the file inside the archive (may be \c NULL if lpszNewName is not \c NULL)
       \param lpszPath
       \param bFullPath
       \param lpszNewName       
       \return a predicted file path 
     */
	CZipString PredictExtractedFileName(LPCTSTR lpszFileNameInZip, LPCTSTR lpszPath, bool bFullPath, LPCTSTR lpszNewName = NULL)const ;

    /**
       
       Return the current value of #m_szTempPath
       
       \return CZipString
     */
	CZipString GetTempPath()const 
	{
		return m_szTempPath;
	}

/**
		Function used in conjunction with #m_szRootPath to trim paths in #AddNewFile and #ExtractFile
		\param	zpc
		\see SetRootPath

*/
	CZipString TrimRootPath(CZipPathComponent& zpc)const ;

    /**
       Remove \e lpszBeginning from the beginning of the \e szPath. Both argument are
	   considered to be paths so they matches up to the path separator.
       
       \param lpszBeginning
       \param szPath
	   \param pCompareFunction (see: #m_pZipCompare)
	
       
       \return \c true if the path beginning was removed
     */
	static bool RemovePathBeginning(LPCTSTR lpszBeginning, CZipString& szPath, ZIPSTRINGCOMPARE pCompareFunction);

	/**
		Set the default archive case-sensitivity. Affects the following functions:
		- #FindFile
		- #FindMatches
		- #EnableFindFast
		- #TrimRootPath
		- #DeleteFiles
		
		
			
		\param bCaseSensitive
		the default CZipArchive case-sensitivity depends on the system and is set
			as follows:
			- on Windows:	\c false <BR>
			- on Linux:	\c true
		\note Set it before using one of the functions above or leave it as it is by default;
			
	*/

	void SetCaseSensitivity(bool bCaseSensitive) 
	{
		m_bCaseSensitive = bCaseSensitive;
		m_pZipCompare = GetCZipStrCompFunc(bCaseSensitive);
	}

	/**
		Return the central directory information.
		\see GetCentralDirSize
	*/
	void GetCentralDirInfo(CZipCentralDir::Info& info)const;
	

	/**
		Get the central directory size.		
		\see CZipCentralDir::GetSize
		\see GetCentralDirInfo
	*/
	DWORD GetCentralDirSize(bool bWhole = true) const
	{
		return m_centralDir.GetSize(bWhole);
	}

	/**
		return \c true if the archive cannot be modified, because it is an existing disk spanning archive
		or it was opened with #zipOpenReadOnly
	*/
	bool IsReadOnly(){return m_storage.IsReadOnly();}


    /**
		
		If you set \e bIgnore to \c true, CRC is not checked for the files being tested or extracted. 
		This method is useful when working with Java <sup><small>TM</small></sup> Archives (jar). 
		The CRC is checked by default. You can use this function on an opened or closed archive.
       
     */
	void SetIgnoreCRC(bool bIgnore = true){m_bIgnoreCRC = bIgnore;}

	/**
		A class used in wildcard pattern matching.
		This class is based on code by J. Kercheval, created 01/05/1991
		and available as a public domain at http://www.snippets.org.
	*/
	class ZIP_API CWildcard  
	{
	public:
		
		
		enum Match
		{
			matchNone,			///< for internal use
			matchValid,			///< valid match
			matchEnd,			///< premature end of pattern string
			matchAbort,			///< premature end of text string
			matchRange,			///< match failure on [..] construct
			matchLiteral,		///< match failure on literal match
			matchPattern		///< bad pattern
		};
		
		enum Pattern 
		{
			patternEmpty = -4,	///< [..] construct is empty
			patternClose,		///< no end bracket in [..] construct
			patternRange,		///< malformed range in [..] construct
			patternEsc,			///< literal escape at end of pattern
			patternValid,		///< valid pattern
		};
		
		
		/**
		Match the pattern against the string \e lpszText
		A match means the entire string \e lpszText is used up in matching.
		
		  \param lpszText
		  the string to match against
		  \param iRetCode
				if not \c NULL, set to one of #Match values indicating the return code
			\return \c true if \e lpszText matches the pattern.
			\see SetPattern
		*/
		bool IsMatch(LPCTSTR lpszText, int* iRetCode = NULL);

		/**
       
		   \param lpszPattern
       
		   \return \c true if \e lpszPattern has any special wildcard characters.

		 */
		static bool IsPattern(LPCTSTR lpszPattern);

		/**
		   Test the pattern for validity.
       
		   \param lpszPattern
				the pattern to test
		   \param iErrorType
				if not \c NULL, set to one of #Pattern values indicating the return code
		   \return \c true if \e lpszPattern is a well formed regular expression according
				to the CWildcard class syntax (see #SetPattern)
		 */
		static bool IsPatternValid(LPCTSTR lpszPattern, int* iErrorType = NULL);
		
		/**
		Match the pattern \e lpszPattern against the string \e lpszText
		A match means the entire string \e lpszText is used up in matching.
		
		  
			\param lpszPattern
			see #SetPattern
			\param lpszText
			the string to match against
			\return 
			one of #Match values
		*/
		static int Match(LPCTSTR lpszPattern, LPCTSTR lpszText);
		
		CWildcard(){}
		/**
		Initialize the pattern.
		\see SetPattern
		*/
		CWildcard(LPCTSTR lpszPattern, bool bCaseSensitive)
		{
			SetPattern(lpszPattern, bCaseSensitive);
		}
		virtual ~CWildcard(){}
		
		/**
		Set the pattern to \e lpszPattern.
		\param lpszPattern
		
		  In the pattern string:
		  - * matches any sequence of characters(zero or more)
		  - ? matches any character
		  - [SET] matches any character in the specified set,
		  - [!SET] or[^SET] matches any character not in the specified set.
		  
			A set is composed of characters or ranges; a range looks like
			character hyphen character(as in 0 - 9 or A - Z).[0 - 9a - zA - Z_] is the
			minimal set of characters allowed in the[..] pattern construct.
			Other characters are allowed(ie. 8 bit characters) if your system
			will support them.
			
			\note To suppress the special syntactic significance of any of []*?!^-\,
			  and match the character exactly, precede it with a \
		*/
		void SetPattern(LPCTSTR lpszPattern, bool bCaseSensitive)
		{
			m_szPattern = lpszPattern;
			m_bCaseSensitive=bCaseSensitive;
			if (!bCaseSensitive)
				m_szPattern.MakeLower();
		}
		operator LPCTSTR()
		{
			return (LPCTSTR)m_szPattern;
		}
	protected:
		bool m_bCaseSensitive;		
		static int MatchAfterStar(LPCTSTR p , LPCTSTR t);
		CZipString m_szPattern;
	};


    /**
		This function finds the indexes of the files, which filenames match the specified pattern and stores
		these indexes in the array. The indexes can be used then e.g. in deleting (CZipArchive::DeleteFiles) or
		extracting files (CZipArchive::ExtractFile).
       
       \param lpszPattern
			the pattern to match (see CWildcard::SetPattern on how to build the pattern). The case-sensitivity of the pattern 
			is set to the global archive case-sensitivity (set with #SetCaseSensitivity)
       \param ar
			the array which will contain the indexes of the files which filenames match the pattern; the contents of the array are NOT clearead, but the 
			indexes are appended to it
	   \param bFullPath <BR>
			- if \c true, match the filename with path (if present) of the file (if the file is a directory, end it with the path separator or use a pattern that will recognize it)
			- otherwise match only the name of the file (if the file is a directory, do not end it with the path separator)
       
     */
	void FindMatches(LPCTSTR lpszPattern, CZipWordArray& ar, bool bFullPath = true)const;


    /**
       Change the name of the file with the given index.
       
		\param uIndex
			zero-based index of the file
		\param lpszNewName
			new name for the file
		\return 
			
		\note Throws exceptions.
		\see SetAdvanced 
     */
	bool RenameFile(WORD uIndex, LPCTSTR lpszNewName);

	/**

		If \c true, the drive letter is removed from the filename stored inside the archive when adding a new file to the archive. It affects 
		#AddNewFile, #ExtractFile, #PredictFileNameInZip, #PredictExtractedFileName, #WillBeDuplicated methods.

		\b Default: \c true
	*/
	bool m_bRemoveDriveLetter;

protected:
	
	/**
		\param iReplaceIndex index of file to be replaced
		\param uTotal the size of the new file to replace existing
		\param lpszFileName the filename for callback initialization
	*/
	void MakeSpaceForReplace(int iReplaceIndex, DWORD uTotal, LPCTSTR lpszFileName);
	/**
		A structure for the internal use only. Clears the password if necessary and
		restores it later (also in case of an exception).
	*/
	struct ZIP_API CZipSmClrPass
	{
		void ClearPasswordSmartly(CZipArchive* pZip)
		{
			m_pZip = pZip;
			m_szPass = pZip->GetPassword();
			if (!m_szPass.IsEmpty())
				pZip->SetPassword();
		}
		~CZipSmClrPass()
		{
			if (!m_szPass.IsEmpty())
				m_pZip->SetPassword(m_szPass);
		}
		CZipString m_szPass;
		CZipArchive* m_pZip;
	};
	
	/**
		Holds map of holes and areas to remain when deleting files from the archive.
		A structure for the internal use only.
	*/
	struct ZIP_API CZipDeleteInfo
	{
		CZipDeleteInfo(){m_pHeader = NULL; m_bDelete = false;}
		CZipDeleteInfo(CZipFileHeader* pHeader, bool bDelete)
			:m_pHeader(pHeader), m_bDelete (bDelete){}
		CZipFileHeader* m_pHeader;
		bool m_bDelete;
	};


	/**
		Storage for callback functors. A structure for the internal use only.
		\see SetCallback
	*/
	struct ZIP_API CZipClbckStrg : public CZipMap<CallbackType, CZipActionCallback*>		
	{
		void Set(CZipActionCallback* pCallback, CallbackType iType)
		{
			if (pCallback)
			{
				SetAt(iType, pCallback);
			}
			else
				RemoveKey(iType);
		}
		CZipActionCallback* Get(CallbackType iType)
		{
			CZipActionCallback* pCallback = NULL;
			if (Lookup(iType, pCallback))
			{
				pCallback->m_iType = iType;
				return pCallback;
			}
			else
				return NULL;
		}
	
	};
	
	
	/**
		\see CZipClbckStrg
	*/
	CZipClbckStrg m_callbacks;
	

	/**
		Write central directory calling a callback functor if available
	*/
	void WriteCentralDirectory(bool bFlush = true);

	/**
		Default archive case-sensitivity
		\see SetCaseSensitivity
	*/
	bool m_bCaseSensitive;

	/**
		a pointer to the function in CZipString structure, 
		used to compare elements; can point to Compare, CompareNoCase, 
		Collate or CollateNoCase function address in CZipString structure ZIPSTRINGCOMPARE is defined as follows: <BR>
		<CODE>typedef int (CZipString::*ZIPSTRINGCOMPARE)( LPCTSTR ) const;</CODE>
	*/
	ZIPSTRINGCOMPARE m_pZipCompare;

	/**
		Internal data.
		\see CZipInternalInfo 
	*/
	CZipInternalInfo m_info;

	
	/**
		Physical layer of the archive.
		\see CZipStorage
	*/
	CZipStorage m_storage;

	/**
		A central directory object.
		\see CZipCentralDir
	*/
	CZipCentralDir m_centralDir;

	/**
		The open mode of the current file inside archive.	
	*/
	enum OpenFileType
	{
		extract = -1,	///< current file opened for extraction
		nothing,		///< no file inside archive opened
		compress		///< a new file opened for compression
	};
	
	/**
		Takes one of the #OpenFileType enum values.
	*/
	char m_iFileOpened;

	/**
		The current AutoFlush value.
		\see Flush
		\see SetAutoFlush
		\see GetAutoFlush
	*/
	bool m_bAutoFlush;

	/**
		The value set with #SetIgnoreCRC
	*/
	bool m_bIgnoreCRC;

	

/**
	The root path to be omitted in #AddNewFile and #ExtractFile functions
	from the beginning of the full file path. Set by #SetRootPath
	\see TrimRootPath	
	\see SetRootPath
*/
	CZipString m_szRootPath;

	/**
		A temporary path set with #SetTempPath function
		\see SetTempPath
		\see AddNewFile
		\see Smartness
	*/
	CZipString m_szTempPath;

/**
	Open the archive in the given mode.
	Called by #Open(LPCTSTR, int, int) and #Open(CZipMemFile&, int).
	\param	iMode
		an opening mode		
	\note Throws exceptions.
*/
	void OpenInternal(int iMode);

	/**
		The system code of the current archive. All new files in the archive 
		will be created regarding this value. Can be one of the enum values 
		defined in \link ZipCompatibility::ZipPlatforms	ZipCompatibility.h
		\endlink

		\see GetSystemCompatibility
	*/
	int m_iArchiveSystCompatib;

/**
	Free the memory allocated by the zlib library that hasn't been freed
	due to an error in the zlib library (usually never happens).
*/
	void EmptyPtrList();



/**
	Move the range of data in the archive specified by the \e uStartOffset
	and \e uEndOffset by \e uToMove		
	
*/
	void MovePackedFiles(DWORD uStartOffset, DWORD uEndOffset, DWORD uMoveBy, CZipActionCallback* pCallback, bool bForward = false);

	/**
		Remove recently added file (used by AddNewFile) if compressed is larger than uncompressed or if callback method return \c false.
		\param bRemoveAnyway if \c true, do not check for the efficiency, but remove the file

	*/
	bool RemoveLast(bool bRemoveAnyway = false);

	/**
		It is used in GetFromArchive public functions and in AddNewFile and the callback parameter is needed
	*/
	bool GetFromArchive(CZipArchive& zip, WORD uIndex, int iReplaceIndex, bool bKeepSystComp, CZipActionCallback* pCallback);

	/**
		\return whether the code should continue or not
	*/
	bool UpdateReplaceIndex(int& iReplaceIndex, LPCTSTR lpszNewFileName);
	
/**
	\return	 the currently opened for compression or decompression
	file inside the archive NULL if there is no file opened
*/
	CZipFileHeader* CurrentFile();

/**
	If the parameter \e iErr signalizes a zlib library error, throw CZipException
	\param	iErr
		a zlib library error to check
	\note Throws exceptions.
*/
	void CheckForError(int iErr);


/**
	Throw a CZipException error.
	\param	err
		the error code
	\see CZipException::ZipErrors
	\param	bZlib
		if \c true, treat \e err as a zlib error code and perform the conversion to the one of CZipException codes.
	\see CZipException::Throw
*/
	void ThrowError(int err, bool bZlib = false);


	typedef CZipPtrList<void*>::iterator CZipPtrListIter;
	CZipPtrList<void*> m_list; ///< a list holding pointers to the memory areas allocated by the zlib library


	static void* _zliballoc(void* opaque, UINT items, UINT size); ///< memory allocator called by the zlib library
	static void _zlibfree(void* opaque, void* address); ///< memory deallocator called by the zlib library

	
	/**
		Specify whether to control memory allocation and freeing by the zlib library.
	
		\b Default: \c true
		\note Set it before opening a file (new or existing) in the archive.
		\see SetDetectZlibMemoryLeaks
		
	*/
	bool m_bDetectZlibMemoryLeaks;


	/**
		Copyright string.
	*/
	static const TCHAR m_gszCopyright[];
	
/**
		\defgroup Cryptography Cryptography
		Methods performing data encryption and decryption
		and attributes used by them.
*/
	/*@{*/

/**
	Decode \e uCount bytes from the internal buffer.
	\see m_info
	\see CZipInternalInfo::m_pBuffer

*/
	void CryptDecodeBuffer(DWORD uCount);
/**
	Encode the internal buffer.
	\see m_info
	\see CZipInternalInfo::m_pBuffer
*/
	void CryptEncodeBuffer();

/**
	Encode the character \e c and update \link #m_keys encryption keys \endlink.

*/
	void CryptEncode(char &c);
/**
	Create an encryption header for the new file in the archive.
	\param	iCrc
		A control value. Use the two lower bytes of CZipFileHeader::m_uModTime.
		This entails the need for a data description presence at the end of 
		the compressed data. We cannot use the real CRC now, because we don't know
		it yet.
	\param	buf
		a buffer to receive the header
	\see CryptCheck
*/
	void CryptCryptHeader(long iCrc, CZipAutoBuffer& buf);
/**
	\internal
	\return	
*/
	DWORD CryptCRC32(DWORD l, char c);
/**
	Decode the character \e c and update \link #m_keys encryption keys \endlink.
*/
	void CryptDecode(char &c);
	/**
		\internal
	*/
	char CryptDecryptByte();

/**
	Decrypt the encryption header and check its control value.
	The control value depends on the presence of the data descriptor.
	\return	\c true if the control value is correct
	\see CryptCryptHeader
*/
	bool CryptCheck();
/**
	Update \link #m_keys encryption keys \endlink with the given value.			
*/
	void CryptUpdateKeys(char c);
/**
	Initialize \link #m_keys encryption keys \endlink.
*/
	void CryptInitKeys();
	/**
		The archive password. If empty, the new file will not be encrypted.
	*/
	CZipAutoBuffer m_pszPassword;

	/**
		Encryption keys.
		The key values are initialized using a supplied encryption password.
		\see CryptInitKeys
	*/
	DWORD m_keys[3];
		
	/*@}*/
};



#endif // !defined(AFX_ZIPARCHIVE_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_)
