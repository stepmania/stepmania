#include "global.h"
#include "RageUtil_CachedObject.h"
#include "RageThreads.h"

#if 0
/* Example use: */
struct Object
{
	CachedObject<Object> m_CachedObject;

	Object() { }
};

struct User
{
	CachedObjectPointer<Object> cache;

	Object *foobar()
	{
		Object *p;
		if( !cache.Get(&p) )
		{
			p = nullptr;
			cache.Set(p);
		}
		return p;
	}
};

void test()
{
	Object p;
	User gar;
	gar.foobar();
}
#endif

static RageMutex m_Mutex("CachedObjects");
void CachedObjectHelpers::Lock()
{
	m_Mutex.Lock();
}

void CachedObjectHelpers::Unlock()
{
	m_Mutex.Unlock();
}

/*
 * (c) 2007 Glenn Maynard
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
