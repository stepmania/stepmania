#include "global.h"
#include "CrashHandlerInternal.h"

#include <csignal>

const char *itoa( unsigned n )
{
	static char ret[32];
	char *p = ret;
	for( int div = 1000000000; div > 0; div /= 10 )
	{
		*p++ = (n / div) + '0';
		n %= div;
	}
	*p = 0;
	p = ret;
	while( p[0] == '0' && p[1] )
		++p;
	return p;
}

const char *SignalName( int signo )
{
	switch( signo )
	{
	case SIGALRM:   return "Alarm";
	case SIGBUS:    return "Bus error";
	case SIGFPE:    return "Floating point exception";
	case SIGHUP:    return "Hangup";
	case SIGILL:    return "Illegal instruction";
	case SIGINT:    return "Interrupt";
	case SIGPIPE:   return "Broken pipe";
	case SIGABRT:   return "Aborted";
	case SIGQUIT:   return "Quit";
	case SIGSEGV:   return "Segmentation fault";
	case SIGTRAP:   return "Trace trap";
	case SIGTERM:   return "Termination";
	case SIGVTALRM: return "Virtual alarm clock";
	case SIGXCPU:   return "CPU limit exceeded";
	case SIGXFSZ:   return "File size limit exceeded";
#if defined(HAVE_DECL_SIGPWR) && HAVE_DECL_SIGPWR
	case SIGPWR:    return "Power failure restart";
#endif
#if defined(HAVE_DECL_SIGUSR1) && HAVE_DECL_SIGUSR1
	case SIGUSR1:   return "Forced crash";
#endif
	}
	static char buf[128];
	strcpy( buf, "Unknown signal " );
	strcat( buf, itoa(signo) );
	return buf;
}


const char *SignalCodeName( int signo, int code )
{
	switch( code )
	{
	case SI_USER:    return "user signal";
	case SI_QUEUE:   return "sigqueue signal";
	case SI_TIMER:   return "timer expired";
	case SI_MESGQ:   return "mesgq state changed";
	case SI_ASYNCIO: return "async I/O completed";
#if defined(SI_SIGIO)
	case SI_SIGIO:   return "queued SIGIO";
#endif
#if defined(SI_KERNEL)
	case SI_KERNEL:  return "kernel signal";
#endif
	}
	
	switch( signo )
	{
	case SIGILL:
		switch( code )
		{
		case ILL_ILLOPC: return "illegal opcode";
		case ILL_ILLTRP: return "illegal trap";
		case ILL_PRVOPC: return "privileged opcode";
#if defined(ILL_ILLOPN)
		case ILL_ILLOPN: return "illegal operand";
#endif
#if defined(ILL_ILLADR)
		case ILL_ILLADR: return "illegal addressing mode";
#endif
#if defined(ILL_PRVREG)
		case ILL_PRVREG: return "privileged register";
#endif
#if defined(ILL_COPROC)
		case ILL_COPROC: return "coprocessor error";
#endif
#if defined(ILL_BADSTK)
		case ILL_BADSTK: return "internal stack error";
#endif
		}
		break;
		
	case SIGFPE:
		switch( code )
		{
		case FPE_FLTDIV: return "floating point divide by zero";
		case FPE_FLTOVF: return "floating point overflow";
		case FPE_FLTUND: return "floating point underflow";
		case FPE_FLTRES: return "floating point inexact result";
		case FPE_FLTINV: return "floating point invalid operation";
#if defined(FPE_INTDIV)
		case FPE_INTDIV: return "integer divide by zero";
#endif
#if defined(FPE_INTOVF)
		case FPE_INTOVF: return "integer overflow";
#endif
#if defined(FPE_FLTSUB)
		case FPE_FLTSUB: return "subscript out of range";
#endif
		}
		break;
		
	case SIGSEGV:
		switch( code )
		{
		case SEGV_MAPERR:    return "address not mapped";
		case SEGV_ACCERR:    return "invalid permissions";
		}
		break;
		
	case SIGBUS:
		switch( code )
		{
		case BUS_ADRALN: return "invalid address alignment";
#if defined(BUS_ADRERR)
		case BUS_ADRERR: return "nonexistent physical address";
#endif
#if defined(BUS_OBJERR)
		case BUS_OBJERR: return "object specific hardware error";
#endif
		}
		break;
		
	case SIGTRAP:
		switch( code )
		{
#if defined(TRAP_BRKPT)
		case TRAP_BRKPT: return "process breakpoint";
#endif
#if defined(TRAP_TRACE)
		case TRAP_TRACE: return "process trace trap";
#endif
		}
		break;
		
	case SIGCHLD:
		switch( code )
		{
		case CLD_EXITED: return "child has exited";
		case CLD_KILLED: return "child was killed";
		case CLD_DUMPED: return "child terminated abnormally";
		case CLD_TRAPPED: return "traced child has trapped";
		case CLD_STOPPED: return "child has stopped";
		case CLD_CONTINUED: return "stopped child has continued";
		}
		break;
		
#if defined(SIGPOLL)
	case SIGPOLL:
		switch( code )
		{
		case POLL_IN:  return "data input available";
		case POLL_OUT: return "output buffers available";
		case POLL_MSG: return "input message available";
		case POLL_ERR: return "i/o error";
		case POLL_PRI: return "high priority input available";
		case POLL_HUP: return "device disconnected";
		}
		break;
#endif
#if defined(HAVE_DECL_SIGUSR1) && HAVE_DECL_SIGUSR1
	case SIGUSR1: return "";
#endif
	}
	
	static char buf[128];
	strcpy( buf, "Unknown code " );
	strcat( buf, itoa(code) );
	return buf;
}

/*
 * (c) 2003-2006 Glenn Maynard, Steve Checkoway
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

