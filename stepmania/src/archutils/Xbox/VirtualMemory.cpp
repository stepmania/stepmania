/* This handles manual paging.  It's primarily intended for the Xbox, but works
 * in Windows as well; it can be enabled for debugging. */

#include "global.h"
#include "VirtualMemory.h"
#include "RageLog.h"
#include <new>

#if defined(WINDOWS)
#define PAGE_FILE_PATH "StepMania pagefile.dat"
#else
#define PAGE_FILE_PATH "Z:\\xxpagefile.sys"
#endif

VirtualMemoryManager vmem_Manager;

struct vm_page
{
	DWORD startAddress; // start address for this page
	unsigned long headPage; // 0 if not allocated. Otherwise, the index of the first page 
							// of this segment.
	bool committed; // true if this page is committed to RAM (is otherwise in the page file)
	bool locked; // true if this page should not be decommitted
	int pageFaults; // number of times this page has been accessed when it wasn't committed
	unsigned long sizeInPages; // size of the data segment in pages.
	size_t sizeInBytes; // size of the data segment in bytes.
};

VirtualMemoryManager::VirtualMemoryManager():
	vmemMutex("VirtualMemory")
{
	pages = 0;
	pageLRU = -1;
	inited = false;
}

VirtualMemoryManager::~VirtualMemoryManager()
{
	Destroy();	
}

bool VirtualMemoryManager::Init(unsigned long totalPageSize, unsigned long sizePerPage, unsigned long thold)
{
	threshold = thold;

	totalPages = totalPageSize / sizePerPage;
	pageSize = sizePerPage;

	if(totalPageSize % sizePerPage != 0)
		totalPageSize = sizePerPage * totalPages;

	// initialise the pages array
	// bypass the overridden new by using HeapAlloc
	pages = (vm_page*)HeapAlloc(GetProcessHeap(), 0, totalPages * sizeof(vm_page));

	if(pages == NULL)
		return false;

	// create the page file on Z drive
	vmemFile = CreateFile( PAGE_FILE_PATH, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if(vmemFile == INVALID_HANDLE_VALUE)
		return false;

	// set the file size
	// find the current file size
	unsigned long fileSize = SetFilePointer(vmemFile, 0, 0, FILE_END);
	if(fileSize == INVALID_SET_FILE_POINTER)
		return false;

	if(fileSize < totalPageSize)
	{
		fileSize = SetFilePointer(vmemFile, totalPageSize, 0, FILE_BEGIN);
		if(fileSize == INVALID_SET_FILE_POINTER)
			return false;
	}

	// Reserve the virtual memory and get the base address
	baseAddress = (DWORD)VirtualAlloc(NULL, totalPageSize, MEM_RESERVE, PAGE_NOACCESS);

	if(baseAddress == NULL)
		return false;

	// initialise the page array
	for(unsigned long i = 0; i < totalPages; i++)
	{
		pages[i].startAddress = baseAddress + (i * pageSize);
		pages[i].headPage = -1;
		pages[i].committed = false;
		pages[i].sizeInPages = 0;
		pages[i].sizeInBytes = 0;
		pages[i].pageFaults = 0;
		pages[i].locked = false;
	}

	inited = true;
	return true;
}

void VirtualMemoryManager::Destroy()
{
	if(pages != 0)
	{
		VirtualFree((LPVOID)baseAddress, 0, MEM_RELEASE);
		HeapFree(GetProcessHeap(), 0, pages);
		CloseHandle(vmemFile);
	}
}

void* VirtualMemoryManager::Allocate(size_t size)
{
	if(!inited)
		return NULL;

	LockMut(vmemMutex);

	unsigned long startPage = -1;
	unsigned long freeSegments = 0;
	unsigned long sizeInPages = (size / pageSize) + 1;

	if(size % pageSize == 0)
		sizeInPages--;
	
	// find a contiguous group of pages that will fit the data
	for(unsigned long i = 0; i < totalPages; i++)
	{
		if(pages[i].sizeInPages != 0)
		{
			startPage = -1;
			freeSegments = 0;
			i += pages[i].sizeInPages - 1; // go to next page segment
		}
		else
		{
			if(startPage == -1)
			{
				startPage = i;
				freeSegments = 1;
			}
			else
				freeSegments++;

			if(sizeInPages == freeSegments)
			{
				if(LOG && logging)
					LOG->Trace("Allocating pages %u to %u", startPage, startPage + freeSegments - 1);

				// commit this to memory
				DWORD ret = (DWORD)VirtualAlloc((LPVOID)pages[startPage].startAddress, size, MEM_COMMIT, PAGE_READWRITE);
				while(ret == NULL)
				{
                    bool swappedOut = DecommitLRU();

					if(!swappedOut)
					{
						if(LOG)
							LOG->Trace("VMem error: out of memory with no pages to swap out left");
						return NULL;
					}

					ret = (DWORD)VirtualAlloc((LPVOID)pages[startPage].startAddress, size, MEM_COMMIT, PAGE_READWRITE);
				}

				pageLRU = (startPage + sizeInPages) % totalPages;
				for(unsigned long j = startPage; j < startPage + freeSegments; j++)
				{
					pages[j].headPage = startPage;
					pages[j].pageFaults = 0;
					pages[j].sizeInPages = freeSegments;
					pages[j].sizeInBytes = size;
					pages[j].committed = true;
					pages[i].locked = false;
				}

				return (void*) ret;
			}
		}
	}

	if(LOG)
		LOG->Trace("VMem error: Couldn't find contiguous group of pages to allocate");

	return NULL;
}

bool VirtualMemoryManager::Free(void *ptr)
{
	if(!inited)
		return false;

	LockMut(vmemMutex);

	// check that the address is within the virtual address bounds
	if((DWORD)ptr < baseAddress || (DWORD)ptr >= baseAddress + (totalPages * pageSize))
	{
		return false;
	}

	// find the page(s) to free
	DWORD offset = (DWORD)ptr - baseAddress;
	unsigned long pageIndex = offset / pageSize;
	
	if(pages[pageIndex].headPage == -1)
	{
		return false;
	}

	pageIndex = pages[pageIndex].headPage;
	ptr = (void *)pages[pageIndex].startAddress;

	unsigned long endPage = pageIndex + pages[pageIndex].sizeInPages;
	unsigned long size = pages[pageIndex].sizeInBytes;

	if(size == 0)
	{
		return false; // trying to free unallocated memory
	}

	if(LOG && logging)
		LOG->Trace("Freeing pages %u to %u", pageIndex, endPage - 1);

	if(pages[pageIndex].committed)
		VirtualFree(ptr, size, MEM_DECOMMIT);

	for(unsigned long i = pageIndex; i < endPage; i++)
	{
		pages[i].headPage = -1;
		pages[i].committed = false;
		pages[i].pageFaults = 0;
		pages[i].sizeInBytes = 0;
		pages[i].sizeInPages = 0;
		pages[i].locked = false;
	}

	return true;
}

bool VirtualMemoryManager::PageFault(void *ptr)
{
	if(!inited)
		return false;

	LockMut(vmemMutex);

	// check that the address is within the virtual address bounds
	if((DWORD)ptr < baseAddress || (DWORD)ptr >= baseAddress + (totalPages * pageSize))
	{
		if(LOG)
		{
			LOG->Trace("Vmem error: Page fault outside virtual memory bounds");
			LOG->Trace("Address: %u, bounds: %u to %u", (DWORD)ptr, baseAddress, baseAddress + (totalPages * pageSize));
		}
		return false;
	}

	// find the page segment that the fault occurred
	unsigned long offset = (DWORD)ptr - baseAddress;
	unsigned long pageIndex = offset / pageSize;


	unsigned long startPage = pages[pageIndex].headPage;
	if(startPage == -1)
	{
		if(LOG)
			LOG->Trace("VMem error: Trying to access memory that wasn't allocated");
		// trying to access memory that wasn't allocated
		return false;
	}

	if(pages[startPage].committed)
	{
		if(LOG && logging)
			LOG->Trace("Pages appear to be committed already. Doing nothing...");
		return true;
	}

	pageLRU = (startPage + pages[startPage].sizeInPages) % totalPages;

	if(LOG && logging)
		LOG->Trace("Reallocating pages %u to %u", startPage, startPage + pages[startPage].sizeInPages - 1);	

	DWORD ret = (DWORD)VirtualAlloc((LPVOID)pages[startPage].startAddress, pages[startPage].sizeInBytes, MEM_COMMIT, PAGE_READWRITE);
	
	while(ret == NULL)
	{
        bool swappedOut = DecommitLRU();
		if(!swappedOut)
		{
			if(LOG)
				LOG->Trace("VMem error: no pages left to swap out while reallocating");

			return false;
		}

		ret = (DWORD)VirtualAlloc((LPVOID)pages[startPage].startAddress, pages[startPage].sizeInBytes, MEM_COMMIT, PAGE_READWRITE);
	}

	for(unsigned long i = startPage; i < startPage + pages[startPage].sizeInPages; i++)
	{
		pages[i].committed = true;
		pages[i].pageFaults++;
	}

	DWORD numRead;

	SetFilePointer(vmemFile, pages[startPage].startAddress - baseAddress, 0, FILE_BEGIN);
	ReadFile(vmemFile, (void *)pages[startPage].startAddress, pages[startPage].sizeInBytes, &numRead, NULL);
	
	return true;
}

bool VirtualMemoryManager::DecommitLRU()
{
	if(!inited)
		return false;

	LockMut(vmemMutex);

	// choose random LRU
	pageLRU = rand() % totalPages;

	for(unsigned long i = 0; i < totalPages; i++)
	{
		unsigned long index = (pageLRU + i) % totalPages;

		if(index == 0)
			index++;

		if(pages[index].headPage == index && pages[index].committed && !pages[index].locked) // this is a head page
		{
			DWORD addr = pages[index].startAddress;
			unsigned long size = pages[index].sizeInPages;

			// decommit this page
			// write to the page file
			if(SetFilePointer(vmemFile, addr - baseAddress, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			{
				if(LOG)
					LOG->Trace("Vmem error: could not write to page file");
				return false;
			}

			DWORD written;
			
			WriteFile(vmemFile, (LPCVOID)addr, pages[index].sizeInBytes, &written, NULL);

			// reset the page data
			if(LOG && logging)
				LOG->Trace("Swapping out pages %i to %i", index, index + size - 1);

			for(unsigned long j = index; j < index + size; j++)
			{
				pages[j].committed = false;
			}

			if(VirtualFree((LPVOID)addr, pages[index].sizeInBytes, MEM_DECOMMIT) == 0)
			{
				return false;
			}

			pageLRU = (pageLRU + pages[index].sizeInPages) % totalPages;

			return true;
		}
	}

	return false;
}

bool VirtualMemoryManager::EnsureFreeMemory(size_t size)
{
	if(!inited)
		return false;

	LockMut(vmemMutex);

	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	while(ms.dwAvailPhys < size)
	{
		if(LOG && logging)
			LOG->Trace("Freeing memory: need %i, have %i", size, ms.dwAvailPhys);

		if(!DecommitLRU())
		{
			if(LOG)
				LOG->Trace("VMem error: No pages left to free while reserving memory");
			return false;
		}
	}

	return true;
}

void VirtualMemoryManager::Lock(void *ptr)
{
	if(!inited)
		return;

	LockMut(vmemMutex);

	// check that the address is within the virtual address bounds
	if((DWORD)ptr < baseAddress || (DWORD)ptr >= baseAddress + (totalPages * pageSize))
	{
		return;
	}

	// find the page(s) to free
	DWORD offset = (DWORD)ptr - baseAddress;
	unsigned long pageIndex = offset / pageSize;
	
	if(pages[pageIndex].headPage == -1)
	{
		return;
	}

	pageIndex = pages[pageIndex].headPage;

	unsigned long endPage = pageIndex + pages[pageIndex].sizeInPages;
	
	if(!pages[pageIndex].committed)
	{
		if(!PageFault(ptr))
			return;
	}

	for(unsigned long i = pageIndex; i < endPage; i++)
	{
		pages[i].locked = true;
	}
}
void VirtualMemoryManager::Unlock(void *ptr)
{
	if(!inited)
		return;

	LockMut(vmemMutex);

	// check that the address is within the virtual address bounds
	if((DWORD)ptr < baseAddress || (DWORD)ptr >= baseAddress + (totalPages * pageSize))
	{
		return;
	}

	// find the page(s) to free
	DWORD offset = (DWORD)ptr - baseAddress;
	unsigned long pageIndex = offset / pageSize;
	
	if(pages[pageIndex].headPage == -1)
	{
		return;
	}

	pageIndex = pages[pageIndex].headPage;

	unsigned long endPage = pageIndex + pages[pageIndex].sizeInPages;
	
	for(unsigned long i = pageIndex; i < endPage; i++)
	{
		pages[i].locked = false;
	}
}

void* operator new (size_t size)
{
	if(!vmem_Manager.IsValid())
		return HeapAlloc(GetProcessHeap(), 0, size);
	
	if(size > vmem_Manager.GetThreshold())
		return vmem_Manager.Allocate(size);
	else
	{
		void *ret = HeapAlloc(GetProcessHeap(), 0, size);
		while(ret == NULL)
		{
			if(!vmem_Manager.DecommitLRU())
				return NULL;

			ret = HeapAlloc(GetProcessHeap(), 0, size);
		}
		return ret;
	}
}

void* operator new[] (size_t size)
{
	if(!vmem_Manager.IsValid())
		return HeapAlloc(GetProcessHeap(), 0, size);
	
	if(size > vmem_Manager.GetThreshold())
		return vmem_Manager.Allocate(size);
	else
	{
		void *ret = HeapAlloc(GetProcessHeap(), 0, size);
		while(ret == NULL)
		{
			if(!vmem_Manager.DecommitLRU())
				return NULL;

			ret = HeapAlloc(GetProcessHeap(), 0, size);
		}
		return ret;
	}
}

void operator delete (void *p)
{
	if(vmem_Manager.IsValid())
	{
		if(vmem_Manager.Free(p))
			return;
	}

	HeapFree(GetProcessHeap(), 0, p);
}

void operator delete[] (void *p)
{
	if(vmem_Manager.IsValid())
	{
		if(vmem_Manager.Free(p))
			return;
	}

	HeapFree(GetProcessHeap(), 0, p);
}

void *valloc(size_t size)
{
	if(!vmem_Manager.IsValid())
		return HeapAlloc(GetProcessHeap(), 0, size);

	if(size > vmem_Manager.GetThreshold())
		return vmem_Manager.Allocate(size);
	else
	{
		void *ret = HeapAlloc(GetProcessHeap(), 0, size);
		while(ret == NULL)
		{
			if(!vmem_Manager.DecommitLRU())
				return NULL;

			ret = HeapAlloc(GetProcessHeap(), 0, size);
		}
		return ret;
	}
}

void vfree(void *ptr)
{
	if(vmem_Manager.IsValid())
	{
		if(vmem_Manager.Free(ptr))
			return;
	}

	HeapFree(GetProcessHeap(), 0, ptr);
}

LONG _stdcall CheckPageFault(LPEXCEPTION_POINTERS e)
{
	if(LOG && e->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		LOG->Trace("Exception: %u", e->ExceptionRecord->ExceptionCode);

	if (e->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;

	DWORD addr = (DWORD)e->ExceptionRecord->ExceptionInformation[1];

	if(vmem_Manager.IsValid())
	{
		if(LOG && vmem_Manager.IsLogging())
			LOG->Trace("Page fault");

		if(vmem_Manager.PageFault((void *)addr))
			return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int NoMemory(size_t size)
{
	if(LOG && vmem_Manager.IsLogging())
		LOG->Trace("Out of memory, freeing up some...");

	if(vmem_Manager.DecommitLRU())
	{
		if(LOG && vmem_Manager.IsLogging())
			LOG->Trace("Freed some memory, trying again");
		return 1;
	}
	else
	{
		if(LOG)
			LOG->Trace("No memory left to free. Failed");
		return 0;
	}
}

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