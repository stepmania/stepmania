#ifndef CRASH_HANDLER_INTERNAL_H
#define CRASH_HANDLER_INTERNAL_H

#define BACKTRACE_MAX_SIZE 100

struct CrashInfo
{
	char m_CrashReason[1024*8];

	const void *m_BacktracePointers[BACKTRACE_MAX_SIZE];

	enum { MAX_BACKTRACE_THREADS = 32 };
	const void *m_AlternateThreadBacktrace[MAX_BACKTRACE_THREADS][BACKTRACE_MAX_SIZE];
	char m_AlternateThreadName[MAX_BACKTRACE_THREADS][128];

	CrashInfo()
	{
		m_CrashReason[0] = 0;
		memset( m_AlternateThreadBacktrace, 0, sizeof(m_AlternateThreadBacktrace) );
		memset( m_AlternateThreadName, 0, sizeof(m_AlternateThreadName) );
		m_BacktracePointers[0] = nullptr;
	}
};

#define CHILD_MAGIC_PARAMETER "--private-do-crash-handler"

#endif

/*
 * (c) 2003-2006 Glenn Maynard
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
