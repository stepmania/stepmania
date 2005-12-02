#include "global.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "StepMania.h"

#include <cstdarg>

#if defined(_WINDOWS) && defined(DEBUG)
#include <windows.h>
#endif

/* This is no longer actually implemented by throwing an exception, but it acts
 * the same way to code in practice. */
void RageException::Throw(const char *fmt, ...)
{
    va_list	va;
    va_start(va, fmt);
    CString error = vssprintf( fmt, va );
    va_end(va);

    CString msg = ssprintf(
				"\n"
				"//////////////////////////////////////////////////////\n"
				"Exception: %s\n"
				"//////////////////////////////////////////////////////\n"
				"",
				error.c_str());
	if(LOG)
	{
		LOG->Trace("%s", msg.c_str());
		LOG->Flush();
	}
	else
	{
		printf("%s\n", msg.c_str());
		fflush(stdout);
	}

#if defined(_WINDOWS) && defined(DEBUG)
	if( IsDebuggerPresent() )
		DebugBreak();
#endif

	StepMania::HandleException( error );
}

/*
 * Copyright (c) 2001-2004 Chris Danford
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
