////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipException.h $
// $Archive: /ZipArchive/ZipException.h $
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
* \file ZipException.h
* Interface for the CZipException class.
*
*/


#if !defined(AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_)
#define AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning( push )
#pragma warning (disable:4702) // disable "Unreachable code" warning in Throw function in the Release mode
#endif // _MSC_VER > 1000


#include "ZipString.h"
#include "ZipBaseException.h"
#include "ZipExport.h"

#define ZIP_ENABLE_ERROR_DESCRIPTION
 
/**
	A class representing exceptions specific to the ZipArchive library.
	Library exception class derived in the MFC version from \c CException 
	and in non-MFC version from \c std::exception.	
*/
class ZIP_API CZipException : public CZipBaseException
{
public:

/**
	\param	iCause
		error cause (takes one of the #ZipErrors enumeration codes)
	\param	lpszZipName
		the name of the file where the error occurred (if applicable)
*/
		CZipException(int iCause = generic, LPCTSTR lpszZipName = NULL);

		CZipException::CZipException(CZipException& e)
		{
			m_szFileName = e.m_szFileName;
			m_iCause = e.m_iCause;
		}

/**
		Throw an exception.
		Throw CZipException* in the MFC version of the library
		(the object must be deleted with Delete() method)
		and CZipException in other versions.
		
	The arguments are the same as in CZipException().

	\param	iZipError
	\param	lpszZipName
	
	\see CZipException()
	
*/

	static void Throw(int iZipError = CZipException::generic, LPCTSTR lpszZipName = NULL)
	{
		#ifdef _MFC_VER
			throw new CZipException(iZipError, lpszZipName);
		#else
			CZipException e(iZipError, lpszZipName);
			throw e;
		#endif
	}

	 
/**
	Convert a zlib library error code to a \link #ZipErrors CZipException error code \endlink
	\param	iZlibError
		zlib library error code
	\return	\link #ZipErrors CZipException error code \endlink
*/

	static int ZlibErrToZip(int iZlibError);


#ifdef ZIP_ENABLE_ERROR_DESCRIPTION

    /**
       Return the error description.
	   \note You need to have defined ZIP_ENABLE_ERROR_DESCRIPTION
	   (in file ZipException.h); undefine this value if you don't want to 
	   store the messages in the library.
       
       
     */
	CZipString GetErrorDescription();

	
    /**
	Return the description of the error based on system variables
	(this function is provided only for compatibility with MFC \c CException::GetErrorMessage)

    \param lpszError 
		a pointer to a buffer that will receive the error message
		if \c NULL

	\param nMaxError
		the maximum number of characters the buffer can hold, including the NULL terminator


     \return 
			\c TRUE if the error string is not empty
	\note 
	- The function will not copy more than \c nMaxError – 1 characters 
		to the buffer, and it always adds a trailing null to end the string;
		if the buffer is too small, the error message will be truncated.
	- You need to have defined ZIP_ENABLE_ERROR_DESCRIPTION
		(in file ZipException.h); undefine this value if you don't want to 
		store the messages in the library.

     */
	BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, UINT* = NULL);

#endif //ZIP_ENABLE_ERROR_DESCRIPTION

	/**
		The name of the zip file where the error occurred.
	*/
	CZipString m_szFileName;

	/**
		The codes of errors thrown by the ZipArchive library
	*/
	enum ZipErrors
	{
		noError,			///< no error 
// 			 1 - 42 reserved for errno (from STL) values - used only in non-MFC versions
// 			 43 - 99 reserved
		generic		= 100,	///< unknown error
		badZipFile,			///< damaged or not a zip file
		badCrc,				///< crc mismatched
		noCallback,			///< no disk-spanning callback functor set
		aborted,			///< callback functor's method Callback returned \c false while disk change in the disk-spanning archive
		abortedAction,		///< callback functor's method Callback returned \c false in CZipArchive class members: AddNewFile, ExtractFile, TestFile, DeleteFile or DeleteFiles 
		abortedSafely,		///< the same as above, you may be sure that the operation was successfully completed before or it didn't cause any damage in the archive (break when counting before deleting files; see CZipArchive::cbDeleteCnt)
		nonRemovable,		///< the disk selected for pkzipSpan archive is non removable
		tooManyVolumes,		///< limit of the maximum volumes reached (999)
		tooLongFileName,	///< the filename of the file added to the archive is too long
		badPassword,		///< incorrect password set for the file being decrypted
		dirWithSize,		///< during testing: found the directory with the size greater than 0
		internal,			///< internal error
		notRemoved,			///< error while removing a file (under Windows call GetLastError() to find out more)
		notRenamed,			///< error while renaming a file (under Windows call GetLastError() to find out more)
		platfNotSupp,		///< the platform that the zip file is being created for is not supported
		cdirNotFound,		///< the central directory was not found in the archive (it is thrown also when the last disk of multi-disk archive is not in the drive when opening the archive)
		streamEnd	= 500,	///< zlib library error
		needDict,			///< zlib library error
		errNo,				///< zlib library error
		streamError,		///< zlib library error
		dataError,			///< zlib library error
		memError,			///< zlib library error thrown by CZipMemFile as well
		bufError,			///< zlib library error
		versionError,		///< zlib library error
	};



	/**
		A cause of the error - takes one of the #ZipErrors enumeration codes.
	*/
	int m_iCause;

	
	virtual ~CZipException();
protected:

#ifdef ZIP_ENABLE_ERROR_DESCRIPTION


    /**
       Return the error description 
       
       \param iCause : error number
	   \param bNoLoop: if \c true tells not to search for en error description,
	   it the error is generic
       
       \return 
     */
	CZipString GetInternalErrorDescription(int iCause, bool bNoLoop = false);


    /**
       Return the description of the error based on system variables
       
      
       \return 
     */
	CZipString GetSystemErrorDescription();


#endif //ZIP_ENABLE_ERROR_DESCRIPTION

#ifdef _MFC_VER
	DECLARE_DYNAMIC(CZipException)
	#pragma warning( pop )
#endif
};


#endif // !defined(AFX_ZIPEXCEPTION_H__E3546921_D728_11D3_B7C7_E77339672847__INCLUDED_)


