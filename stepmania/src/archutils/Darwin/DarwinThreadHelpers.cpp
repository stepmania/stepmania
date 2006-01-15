#include <mach/mach_types.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include "Backtrace.h"
#include "RageLog.h"

bool SuspendThread( uint64_t threadHandle )
{
	return !thread_suspend( thread_act_t(threadHandle) );
}

bool ResumeThread( uint64_t threadHandle )
{
	return !thread_resume( thread_act_t(threadHandle) );
}

uint64_t GetCurrentThreadId()
{
	return mach_thread_self();
}

bool GetThreadBacktraceContext( uint64_t iID, BacktraceContext *ctx )
{
	/* Can't GetThreadBacktraceContext the current thread. */
	ASSERT( iID != GetCurrentThreadId() );

#if defined(__ppc__)
	SuspendThread( iID );

	thread_act_t thread = thread_act_t( iID );
	ppc_thread_state state;
	mach_msg_type_number_t count = PPC_THREAD_STATE_COUNT;
	
	if( thread_get_state(thread, PPC_THREAD_STATE, thread_state_t(&state), &count) )
		return false;
	ctx->FramePtr = (void *)state.r1;
	ctx->PC = (void *)state.srr0;
	return true;
#else
	return false;
#endif
}

void SetThreadPrecedence( int prec )
{
	thread_precedence_policy po = { prec };
	kern_return_t ret = thread_policy_set( mach_thread_self(), THREAD_PRECEDENCE_POLICY,
					       (int *)&po, THREAD_PRECEDENCE_POLICY_COUNT );
	
	if( ret != KERN_SUCCESS )
		LOG->Warn( "Couldn't set the precedence for the input thread: %s",
			   mach_error_string(ret) );
}
	
/*
 * (c) 2004, 2005 Steve Checkoway
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
