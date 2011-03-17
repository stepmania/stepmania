#include "global.h"
#include "RunningUnderValgrind.h"

#if defined(CPU_X86) && defined(__GNUC__)
bool RunningUnderValgrind()
{
	/* Valgrind crashes and burns on pthread_mutex_timedlock. */
        static int under_valgrind = -1;
	if( under_valgrind == -1 )
	{
		unsigned int magic[8] = { 0x00001001, 0, 0, 0, 0, 0, 0, 0 };
		asm(
			"mov %1, %%eax\n"
			"mov $0, %%edx\n"
			"rol $29, %%eax\n"
			"rol $3, %%eax\n"
			"ror $27, %%eax\n"
			"ror $5, %%eax\n"
			"rol $13, %%eax\n"
			"rol $19, %%eax\n"
			"mov %%edx, %0\t"
			: "=r" (under_valgrind): "r" (magic): "eax", "edx" );
	}
	
	return under_valgrind != 0;
}
#else
bool RunningUnderValgrind()
{
	return false;
}
#endif

/*
 * (c) 2004 Glenn Maynard
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
