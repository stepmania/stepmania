////////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipCollections.h $
// $Archive: /ZipArchive/ZipCollections.h $
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

#ifndef ZIPCOLLECTIONS_DOT_H
#define ZIPCOLLECTIONS_DOT_H

#if _MSC_VER > 1000
#pragma once
#pragma warning( push )
#pragma warning (disable:4786) // 'identifier' : identifier was truncated to 'number' characters in the debug information
#endif // _MSC_VER > 1000

#include <afxtempl.h>
typedef CStringArray CZipStringArray;

template <class TYPE>
class CZipArray : public CArray<TYPE, TYPE>
{

	static int CompareAsc(const void *pArg1, const void *pArg2)
	{
		TYPE w1 = *(TYPE*)pArg1;
		TYPE w2 = *(TYPE*)pArg2;
		return w1 == w2 ? 0 :(w2 > w1 ? - 1 : 1);
	}
	static int CompareDesc(const void *pArg1, const void *pArg2)
	{
		TYPE w1 = *(TYPE*)pArg1;
		TYPE w2 = *(TYPE*)pArg2;
		return w1 == w2 ? 0 :(w1 > w2 ? - 1 : 1);		
	}
public:
	void Sort(bool bAscending)
	{
		int iSize = GetSize();
		if (!iSize) // if ommitted operator [] will fail if empty
			return;
		qsort((void*)&((*this)[0]),iSize , sizeof(TYPE), bAscending ? CompareAsc : CompareDesc);		
	}
};

typedef CZipArray<WORD> CZipWordArray;

template<class TYPE>
class CZipPtrList : public CTypedPtrList<CPtrList, TYPE>
{
public:
	typedef POSITION iterator;
	typedef POSITION const_iterator;

	bool IteratorValid(const iterator &iter) const
	{
		return iter != NULL;
	}

};

template<class KEY, class VALUE>
class CZipMap : public CMap<KEY, KEY, VALUE, VALUE>
{
	
};

#ifdef _MFC_VER
	#pragma warning( pop )
#endif


#endif  /* ZIPCOLLECTIONS_DOT_H */

