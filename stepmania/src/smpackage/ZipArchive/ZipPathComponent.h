////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipPathComponent.h $
// $Archive: /ZipArchive/ZipPathComponent.h $
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
* \file ZipPathComponent.h
* Interface for the CZipPathComponent class.
*
*/
#if !defined(AFX_ZIPPATHCOMPONENT_H__9B222C08_AD11_4138_96CC_1237511E3E37__INCLUDED_)
#define AFX_ZIPPATHCOMPONENT_H__9B222C08_AD11_4138_96CC_1237511E3E37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZipString.h"
#include "ZipExport.h"

/**
	A class splitting a file path into components.
*/
class ZIP_API CZipPathComponent  
{
public:
	CZipPathComponent(){}
	virtual ~CZipPathComponent();

	static const TCHAR m_cSeparator; ///< A system - specific default path separator. Defined in ZipPlatform.cpp.
/**
	Append a path separator to \e szPath if it is not already there.
*/
	static void AppendSeparator(CZipString& szPath)
	{
		RemoveSeparators(szPath);
		szPath += m_cSeparator;
	}

	/**
		Remove separators from the end of \e szPath
	*/
	static void RemoveSeparators(CZipString& szPath)
	{
// 		szPath.TrimRight(m_cSeparator);
		szPath.TrimRight(_T("\\/"));
	}

	/**
		Remove separators from the beginning of \e szPath

	*/

	static void RemoveSeparatorsLeft(CZipString& szPath)
	{
		szPath.TrimLeft(_T("\\/"));
	}


    /**
	
	  \return \c true if \e c is a slash or a backslash
       
       
     */
	static bool IsSeparator(TCHAR c)
	{
		return c == _T('\\') || c == _T('/');
	}
	
	/**
		\return \c true if the path has a path separator at the and
	*/
	static bool HasEndingSeparator(const CZipString& szPath)
	{
		int iLen = szPath.GetLength();
		if (iLen)
			return IsSeparator(szPath[iLen - 1]);
		else
			return false;
	}

/**
	Construct the object and set a path.
	\param	lpszFullPath
		the full path of the file
	\see SetFullPath
*/
	CZipPathComponent(LPCTSTR lpszFullPath)
	{
		SetFullPath(lpszFullPath);
	}
	
	// full path of the file (not a directory alone)
/**
	Set the path.
	\param	lpszFullPath
		a full path to the file (including a filename - the last element in the
		path is assumed to be a filename)
*/
	void SetFullPath(LPCTSTR lpszFullPath);

/**
	\return	the filename (without an extension)
*/
	CZipString GetFileTitle() const { return m_szFileTitle;}

/**
	Set the file title (without an extension).
	\param	lpszFileTitle
		
	\return	
*/
	void SetFileTitle(LPCTSTR lpszFileTitle) { m_szFileTitle = lpszFileTitle;}

	
/**
	Set the extension alone.
	\param	lpszExt
		may but not have to contain a dot at the beginning
*/
	void SetExtension(LPCTSTR lpszExt) 
	{
		m_szFileExt = lpszExt;
		m_szFileExt.TrimLeft(_T('.'));
	}

/**
	\return	the file extension without a dot
*/
	CZipString GetFileExt() const { return m_szFileExt;}
/**
	\return	the drive (no path separator at the end)
*/
	CZipString GetFileDrive() const { return m_szDrive;}
/**
	\return	the full path without the drive (no separator at the beginning)
*/
	CZipString GetNoDrive() const ;

/**
	\return	the filename including an extension
*/
	CZipString GetFileName() const
	{
		CZipString szFullFileName = m_szFileTitle;
		if (!m_szFileExt.IsEmpty())
		{
			szFullFileName += _T(".");
			szFullFileName += m_szFileExt;
		}
		return szFullFileName;
	}
/**
	\return	the full path of the file (including the filename)
*/
	CZipString GetFullPath() const
	{
		CZipString szFullPath = GetFilePath();
		CZipString szFileName = GetFileName();
		if (!szFileName.IsEmpty())
		{
			szFullPath  += m_cSeparator;
			szFullPath  += szFileName;
		}
		return szFullPath;

	}
/**
	\return	the path of the file (without the separator at the end)
*/
	CZipString GetFilePath() const
	{
			CZipString szDrive = m_szDrive;
			CZipString szDir = m_szDirectory;
			if (!szDrive.IsEmpty() && !szDir.IsEmpty())
				szDrive += m_cSeparator;

			return m_szPrefix + szDrive + szDir;	

	}
protected:
	/**
		\name Path components
	*/
	//@{
	CZipString m_szDirectory,	///< a directory(ies) (one or more) without the path separators at the end and the beginning
		m_szFileTitle,			///< a filename without an extension
		m_szFileExt,			///< a file extension without a dot
		m_szDrive,				///< a drive (if the system path standard uses it) without a path separator at the end
		m_szPrefix;				///< a prefix (e.g. for the UNC path or Unicode path under Windows)
	//@}
	
};

#endif // !defined(AFX_ZIPPATHCOMPONENT_H__9B222C08_AD11_4138_96CC_1237511E3E37__INCLUDED_)
