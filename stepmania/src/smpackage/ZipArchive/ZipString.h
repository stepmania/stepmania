////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipString.h $
// $Archive: /ZipArchive/ZipString.h $
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

#ifndef ZIPSTRING_DOT_H
#define ZIPSTRING_DOT_H
#include "stdafx.h"
#include "ZipExport.h"

typedef CString CZipString;

/**
	A poiter type to point to CZipString to Collate or CollateNoCase
	or Compare or CompareNoCase
*/
typedef int (CZipString::*ZIPSTRINGCOMPARE)( LPCTSTR ) const;



/**
	return a pointer to the function in CZipString structure, 
	used to compare elements depending on the arguments
*/
	ZIP_API ZIPSTRINGCOMPARE GetCZipStrCompFunc(bool bCaseSensitive, bool bCollate = true);

#endif  /* ZIPSTRING_DOT_H */
