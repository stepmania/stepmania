#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#if defined(XBOX)
#include <xtl.h>
#else
#include <windows.h>
#endif

#include "RageThreads.h"

struct vm_page;

class VirtualMemoryManager
{
public:
	VirtualMemoryManager();
	~VirtualMemoryManager();

	bool Init(unsigned long totalPageSize, unsigned long sizePerPage, unsigned long thold);
	void Destroy();
	void* Allocate(size_t size);
	bool Free(void *ptr);
	bool PageFault(void *ptr);
	bool DecommitLRU();
	bool EnsureFreeMemory(size_t size);
	void Lock(void *ptr);
	void Unlock(void *ptr);
	void SetLogging(bool l) { logging = l; }
	
	bool IsValid() { return inited; }
	bool IsLogging() { return logging; }
	unsigned long GetThreshold() { return threshold; }

protected:
	vm_page *pages;
	HANDLE vmemFile;
	DWORD baseAddress;
	unsigned long totalPages;
	unsigned long pageSize;
	unsigned long threshold;
	unsigned long pageLRU;

	bool inited;
	bool logging;

	// mutex to make sure pages aren't allocated/deallocated concurrently
	RageMutex vmemMutex;
};

void *valloc(size_t size);
void vfree(void *ptr);

//#define malloc(size) valloc(size)
//#define free(memblock) vfree(memblock)

extern VirtualMemoryManager vmem_Manager;

LONG _stdcall CheckPageFault(LPEXCEPTION_POINTERS e);
int NoMemory(size_t size);

#endif

/*
 * (c) 2004 Ryan Dortmans
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */