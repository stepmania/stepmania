////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipFileMapping.h $
// $Archive: /ZipArchive_Linux/ZipFileMapping.h $
// $Date: 2003-07-21 21:10:30 -0500 (Mon, 21 Jul 2003) $ $Author: gmaynard $
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


#if !defined(AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_)
#define AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <cstddef>
#include <sys/mman.h>

namespace ziparchv
{


	struct CZipFileMapping
	{
		CZipFileMapping()
		{
			m_iSize = 0;
			m_pFileMap = NULL;
		}
		bool CreateMapping(CZipFile* pFile)
		{
			if (!pFile)
				return false;
			m_iSize = pFile->GetLength();
			m_pFileMap = mmap(0, m_iSize, PROT_READ|PROT_WRITE, MAP_SHARED, pFile->m_hFile, 0);
			return (m_pFileMap != NULL);
		}
		void RemoveMapping()
		{

			if (m_pFileMap)
			{
				munmap(m_pFileMap, m_iSize);
				m_pFileMap = NULL;
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
		void* m_pFileMap;
		std::size_t m_iSize;

	};
}

#endif // !defined(AFX_AUTOHANDLE_H__D68326EA_D7FA_4792_AB1F_68D09533E399__INCLUDED_)
