////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFileMapping.h $
// $Archive: /ZipArchive/ZipFileMapping.h $
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


#if !defined(AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_)
#define AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZipFile.h"
namespace ziparchv
{
	

	struct CZipFileMapping
	{
		CZipFileMapping()
		{
			m_hFileMap = NULL;
			m_pFileMap = NULL;
		}
		bool CreateMapping(CZipFile* pFile) 
		{
			if (!pFile)
				return false;
			m_hFileMap = CreateFileMapping((*pFile), NULL, PAGE_READWRITE,
				0, 0, _T("ZipArchive Mapping File"));
			if (!m_hFileMap)
				return false;
			// Get pointer to memory representing file
			m_pFileMap = MapViewOfFile(m_hFileMap, FILE_MAP_WRITE, 0, 0, 0);
			return (m_pFileMap != NULL);
		}
		void RemoveMapping()
		{
			if (m_pFileMap)
			{
				UnmapViewOfFile(m_pFileMap);
				m_pFileMap = NULL;
			}
			if (m_hFileMap)
			{
				CloseHandle(m_hFileMap);
				m_hFileMap = NULL;
			}
			
		}
		~CZipFileMapping()
		{
			RemoveMapping();
		}
		char* GetMappedMemory()
		{
			return reinterpret_cast<char*> (m_pFileMap);
		}
	protected:
		HANDLE m_hFileMap;
		LPVOID m_pFileMap;

	};
}

#endif // !defined(AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_)
