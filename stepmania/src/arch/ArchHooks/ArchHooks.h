#ifndef ARCH_HOOKS_H
#define ARCH_HOOKS_H

/*
 * I'm undecided about when it's a good idea to use arch; I'm experimenting
 * a bit.
 *
 * This arch driver is simply called at various important points, to let
 * architectures do things.  Windows can use this to start up the crash
 * handler and to output Windows-specific debug information, for example.
 *
 * This class should have one-way data transfer only: functions are called,
 * they may do something, and no data is returned.  (We don't want to have
 * to deal with error handling and so on when using these functions.)
 *
 * This class is constructed at the very start of the program.
 */
class ArchHooks
{
protected:
    bool MessageIsIgnored( CString ID );
    void IgnoreMessage( CString ID );
	
public:
	/* This is called as soon as SDL is set up, the loading window is shown
	 * and we can safely log and throw. */
	virtual void DumpDebugInfo() { }

	/* Message box stuff.  Not supported on all archs.  The ID can be used
	 * to identify a class of messages, for "don't display this dialog"-type
	 * prompts.  (Don't overload these, overload *Private below.) */
	enum MessageBoxResult { abort, retry, ignore, cancel };
	void MessageBoxError( CString error, CString ID = "" );
	void MessageBoxOK( CString sMessage, CString ID = "" );
	MessageBoxResult MessageBoxAbortRetryIgnore( CString sMessage, CString ID = "" );
	MessageBoxResult MessageBoxRetryCancel( CString sMessage, CString ID = "" );

	virtual ~ArchHooks() { }
	/* This is called once each time through the game loop */
	virtual void Update(float delta) { }

	/* Re-exec the game.  If this is implemented, it doesn't return. */
	virtual void RestartProgram() { }

	/* Call this to temporarily enter a high-priority or realtime scheduling (depending
	 * on the implementation) mode.  This is used to improve timestamp accuracy.  Do as
	 * little as possible in this mode; hanging in it might hang the system entirely. */
	virtual void EnterTimeCriticalSection() { }
	virtual void ExitTimeCriticalSection() { }

	virtual void SetTime( tm newtime ) { }

protected:
	virtual void MessageBoxErrorPrivate( CString sMessage, CString ID ) { printf("Error: %s\n", sMessage.c_str()); }
	virtual void MessageBoxOKPrivate( CString sMessage, CString ID ) {}
	virtual MessageBoxResult MessageBoxAbortRetryIgnorePrivate( CString sMessage, CString ID ) { return ignore; } 
	virtual MessageBoxResult MessageBoxRetryCancelPrivate( CString sMessage, CString ID ) { return cancel; } 

};

#endif

extern ArchHooks *HOOKS;	// global and accessable from anywhere in our program

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
