////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipInternalInfo.h $
// $Archive: /ZipArchive/ZipInternalInfo.h $
// $Date$ $Author$
////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyright 2000-2002 by Tadeusz Dracz (http://www.artpol-software.com/)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details see the file License.txt
////////////////////////////////////////////////////////////////////////////////

/**
* \file ZipInternalInfo.h
* Interface for the CZipInternalInfo structure.
*
*/
#if !defined(AFX_ZIPINTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_)
#define AFX_ZIPINTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZipAutoBuffer.h"
#include "zlib/zlib.h"

/**
	The purpose of this structure is to hold the data that allow communication
	with the zlib library
*/
struct CZipInternalInfo  
{
	CZipInternalInfo();
	virtual ~CZipInternalInfo();

/**
	Allocate internal buffer of m_iBufferSize size
*/
	void Init();
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

#endif // !defined(AFX_ZIPINTERNALINFO_H__C6749101_590C_4F74_8121_B82E3BE9FA44__INCLUDED_)
